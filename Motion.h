#ifndef _MOTION_H_
#define _MOTION_H_

#include "config.h"
#include "Movedata.h"
#include "MGcode.h"
#include "Axis.h"
#include "Globals.h"
#include "Gcodes.h"

class Motion
{
  // Singleton

  
public:
  static Motion& Instance() { static Motion instance; return instance; };
private:
  explicit Motion() :AXES((Axis[NUM_AXES])
  {
    Axis(X_STEP_PIN,X_DIR_PIN,X_ENABLE_PIN,X_MIN_PIN,X_MAX_PIN,X_STEPS_PER_UNIT,X_INVERT_DIR,X_LENGTH,X_MAX_FEED,X_AVG_FEED,X_START_FEED,X_ACCEL_DIST,X_HOME_DIR),
    Axis(Y_STEP_PIN,Y_DIR_PIN,Y_ENABLE_PIN,Y_MIN_PIN,Y_MAX_PIN,Y_STEPS_PER_UNIT,Y_INVERT_DIR,Y_LENGTH,Y_MAX_FEED,Y_AVG_FEED,Y_START_FEED,Y_ACCEL_DIST,Y_HOME_DIR),
    Axis(Z_STEP_PIN,Z_DIR_PIN,Z_ENABLE_PIN,Z_MIN_PIN,Z_MAX_PIN,Z_STEPS_PER_UNIT,Z_INVERT_DIR,Z_LENGTH,Z_MAX_FEED,Z_AVG_FEED,Z_START_FEED,Z_ACCEL_DIST,Z_HOME_DIR),
    Axis(A_STEP_PIN,A_DIR_PIN,A_ENABLE_PIN,Pin(),Pin(),A_STEPS_PER_UNIT,A_INVERT_DIR,A_LENGTH,A_MAX_FEED,A_AVG_FEED,A_START_FEED,A_ACCEL_DIST,A_HOME_DIR)
  })
  {
    setupInterrupt();
  };
  Motion(Motion&);
  Motion& operator=(Motion&);

  Axis AXES[NUM_AXES];
  volatile MGcode* volatile current_gcode;
  volatile unsigned long deltas[NUM_AXES];
  volatile long errors[NUM_AXES];

public:

  Point getCurrentPosition()
  {
    Point p;
    for(int ax=0;ax<NUM_AXES;ax++)
      p[ax] = AXES[ax].getCurrentPosition();
    return p;
  }

  void setCurrentPosition(MGcode &gcode)
  {
    for(int ax=0;ax<NUM_AXES;ax++)
    {
      if(!gcode[ax].isUnused())
      {
        AXES[ax].setCurrentPosition(gcode[ax].getFloat());
      }
    }
  }

  void setAbsolute()
  {
    for(int ax=0;ax<NUM_AXES;ax++)
      AXES[ax].setAbsolute();
  }

  void setRelative()
  {
    for(int ax=0;ax<NUM_AXES;ax++)
      AXES[ax].setRelative();
  }




  void getMovesteps(MGcode& gcode)
  {
    for(int ax=0;ax < NUM_AXES;ax++)
    {
      gcode.movedata.axismovesteps[ax] = AXES[ax].getMovesteps(gcode.movedata.startpos[ax], 
                                                               gcode[ax].isUnused() ? gcode.movedata.startpos[ax] : AXES[ax].isRelative() ? gcode.movedata.startpos[ax] + gcode[ax].getFloat() : gcode[ax].getFloat(), 
                                                               gcode.movedata.axisdirs[ax]);
      if(gcode.movedata.movesteps < gcode.movedata.axismovesteps[ax])
      {
        gcode.movedata.movesteps = gcode.movedata.axismovesteps[ax];
        gcode.movedata.leading_axis = ax;
      }
    }
  }

  unsigned long getLargestStartInterval(MGcode& gcode)
  {
    unsigned long mi = 0;
    for(int ax=0;ax < NUM_AXES;ax++)
    {
      if(gcode.movedata.axismovesteps[ax] == 0) continue;
      unsigned long t = AXES[ax].getStartInterval(gcode.movedata.feed);
      if(t > mi) mi = t;
    }
    return mi;
  }

  unsigned long getLargestEndInterval(MGcode& gcode)
  {
    unsigned long mi = 0;
    for(int ax=0;ax < NUM_AXES;ax++)
    {
      if(gcode.movedata.axismovesteps[ax] == 0) continue;
      unsigned long t = AXES[ax].getEndInterval(gcode.movedata.feed);
      if(t > mi) mi = t;
    }
    return mi;
  }

  unsigned long getLargestAccelDistance(MGcode& gcode)
  {
      unsigned long ad = 0;
      for(int ax=0;ax < NUM_AXES;ax++)
      {
        if(gcode.movedata.axismovesteps[ax] == 0) continue;
        unsigned long t = AXES[ax].getAccelDistance();
        if(t > ad) ad = t;
      }
      return ad;
  }

  void getActualEndpos(MGcode& gcode)
  {
    for(int ax=0;ax<NUM_AXES;ax++)
    {
      gcode.movedata.endpos[ax] = AXES[ax].getEndpos(gcode.movedata.startpos[ax], gcode.movedata.axismovesteps[ax], gcode.movedata.axisdirs[ax]);
    }
  }


  void gcode_precalc(MGcode& gcode, float& feedin, Point& lastend)
  {
    Movedata& md = gcode.movedata;

    if(gcode.state >= MGcode::PREPARED)
      return;

    // We want to carry over the previous ending position and feedrate if possible.
    md.startpos = lastend;
    if(gcode[F].isUnused())
      md.feed = feedin;
    else
      md.feed = gcode[F].getFloat();

    if(md.feed == 0)
      md.feed = SAFE_DEFAULT_FEED;


    getMovesteps(gcode);
    md.startinterval = getLargestStartInterval(gcode);
    md.fullinterval = getLargestEndInterval(gcode);
    md.currentinterval = md.startinterval;
    md.steps_to_accel = getLargestAccelDistance(gcode);
    getActualEndpos(gcode);
    md.accel_until = md.movesteps;
    md.decel_from  = 0;
    md.accel_inc   = 0;

    long intervaldiff = md.startinterval - md.fullinterval;
    if(intervaldiff > 0)
    {
      if(md.steps_to_accel > md.movesteps / 2)
      {
        md.accel_until = md.movesteps / 2;
        md.decel_from  = md.accel_until - 1;
      }
      else
      {
        md.accel_until = md.movesteps - md.steps_to_accel;
        md.decel_from  = md.steps_to_accel;
      }
      md.accel_inc = intervaldiff / md.steps_to_accel;
    }  

    lastend = md.endpos;
    feedin  = md.feed;
    gcode.state = MGcode::PREPARED;
  }

  
  void gcode_execute(MGcode& gcode)
  {
    // Only execute codes that are prepared.
    if(gcode.state < MGcode::PREPARED)
      return; // TODO: This should never happen now and is an error.
    // Don't execute codes that are ACTIVE or DONE (ACTIVE get handled by interrupt)
    if(gcode.state > MGcode::PREPARED)
      return;

    // temporary - dump data to host
    dumpMovedata(gcode.movedata);
    // NOT temporary - set axis move data, invalidate all precomputes if bad data
    for(int ax=0;ax<NUM_AXES;ax++)
    {
      // AXES[ax].dump_to_host();
      if(!AXES[ax].setupMove(gcode.movedata.startpos[ax], gcode.movedata.axisdirs[ax], gcode.movedata.axismovesteps[ax]))
      {
        GCODES.Invalidate();
        return;
      }
      deltas[ax] = gcode.movedata.axismovesteps[ax];
      errors[ax] = gcode.movedata.movesteps / 2;
      //AXES[ax].dump_to_host();
    }
    gcode.dump_to_host();

    // setup pointer to current move data for interrupt
    gcode.state = MGcode::ACTIVE;
    current_gcode = &gcode;

    setInterruptCycles(gcode.movedata.startinterval);
    enableInterrupt();
  }

  bool axesAreMoving() 
  { 
    for(int ax=0;ax<NUM_AXES;ax++) 
      if(AXES[ax].isMoving()) return true; 
    
    return false; 
  }

  void dumpMovedata(Movedata& md)
  {
    HOST.labelnum("FEED:", md.feed, false);
    HOST.labelnum(" MS:", md.movesteps, false);
    HOST.labelnum(" LEAD:", md.leading_axis, false);
    HOST.labelnum(" SI:", md.startinterval, false);
    HOST.labelnum(" FI:", md.fullinterval, false);
    HOST.labelnum(" CI:", md.currentinterval, false);
    HOST.labelnum(" STA:", md.steps_to_accel, false);
    HOST.labelnum(" AU:", md.accel_until, false);
    HOST.labelnum(" DF:", md.decel_from, false);
    HOST.labelnum(" AI:", md.accel_inc);
  }

  void writePositionToHost()
  {
    for(int ax=0;ax<NUM_AXES;ax++)
    {
      HOST.write(ax > Z ? 'A' - Z - 1 + ax : 'X' + ax); HOST.write(':'); HOST.write(AXES[ax].getCurrentPosition(),10,4); HOST.write(' ');
    }
  }



  void handleInterrupt()
  {
    volatile Movedata& md = current_gcode->movedata;
    if(md.movesteps == 0)
    {
      disableInterrupt();
      current_gcode->state = MGcode::DONE;
      return;
    }
    md.movesteps--;

    if(md.movesteps > md.accel_until)
    {
      md.currentinterval -= md.accel_inc;
    }
    else if(md.movesteps < md.decel_from)
    {
      md.currentinterval += md.accel_inc;
    }

    OCR1A = md.currentinterval;

    for(int ax=0;ax<NUM_AXES;ax++)
    {
      if(ax == md.leading_axis)
      {
        AXES[ax].doStep();
        continue;
      }

      errors[ax] = errors[ax] - deltas[ax];
      if(errors[ax] < 0)
      {
        AXES[ax].doStep();
        errors[ax] = errors[ax] + deltas[md.leading_axis];
      }
    }
        
    if(md.movesteps == 0)
    {
      // Finish up any remaining axis - I don't know this will ever actually happen
      for(int ax=0;ax<NUM_AXES;ax++)
      {
        while(AXES[ax].isMoving())
        {
          HOST.labelnum("SR:",AXES[ax].getRemainingSteps(), true);
          AXES[ax].doStep();
        }
      }

      disableInterrupt();
      current_gcode->state = MGcode::DONE;
    }
  }

  void setupInterrupt() 
  {
    // 16-bit registers that must be set/read with interrupts disabled:
    // TCNTn, OCRnA/B/C, ICRn
    // "Fast PWM" and no prescaler.
    //TCCR1A = _BV(WGM10) | _BV(WGM11);
    //TCCR1B = _BV(WGM12) | _BV(WGM13) | _BV(CS10);
    TCCR1A = 0;
    TCCR1B = _BV(WGM12) | _BV(CS10);
  }


  void enableInterrupt() 
  {
    // reset counter in case.
    // TCNT1 = 0;
     // Enable timer
    PRR0 &= ~(_BV(PRTIM1));
    // Outcompare Compare match A interrupt
    TIMSK1 |= _BV(OCIE1A);

  };
  void disableInterrupt() 
  {
    // Outcompare Compare match A interrupt
    TIMSK1 &= ~(_BV(OCIE1A));
    // Disable timer...
    PRR0 |= _BV(PRTIM1);

  };
  void setInterruptCycles(unsigned long cycles) 
  {
    // Reset timer to 0, target to cycles.
    OCR1A = cycles;
  };

};


extern Motion& MOTION;

#endif // _MOTION_H_
