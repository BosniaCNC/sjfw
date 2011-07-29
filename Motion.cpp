#include "Motion.h"

#include "config.h"
#include "MGcode.h"
#include "Axis.h"
#include "Globals.h"
#include "GcodeQueue.h"




Point& Motion::getCurrentPosition()
{
  static Point p;
  for(int ax=0;ax<NUM_AXES;ax++)
    p[ax] = AXES[ax].getCurrentPosition();
  return p;
}

void Motion::setCurrentPosition(MGcode &gcode)
{
  for(int ax=0;ax<NUM_AXES;ax++)
  {
    if(!gcode[ax].isUnused())
    {
      AXES[ax].setCurrentPosition(gcode[ax].getFloat());
    }
  }
}

void Motion::setAbsolute()
{
  for(int ax=0;ax<NUM_AXES;ax++)
    AXES[ax].setAbsolute();
}

void Motion::setRelative()
{
  for(int ax=0;ax<NUM_AXES;ax++)
    AXES[ax].setRelative();
}

void Motion::setMinimumFeedrate(MGcode& gcode)
{
  for(int ax=0;ax < NUM_AXES;ax++)
  {
    if(gcode[ax].isUnused()) continue;
    AXES[ax].setMinimumFeedrate(gcode[ax].getFloat());
  }
}

void Motion::setMaximumFeedrate(MGcode& gcode)
{
  for(int ax=0;ax < NUM_AXES;ax++)
  {
    if(gcode[ax].isUnused()) continue;
    AXES[ax].setMaximumFeedrate(gcode[ax].getFloat());
  }
}

void Motion::setAverageFeedrate(MGcode& gcode)
{
  for(int ax=0;ax < NUM_AXES;ax++)
  {
    if(gcode[ax].isUnused()) continue;
    AXES[ax].setAverageFeedrate(gcode[ax].getFloat());
  }
}

void Motion::setStepsPerUnit(MGcode& gcode)
{
  for(int ax=0;ax < NUM_AXES;ax++)
  {
    if(gcode[ax].isUnused()) continue;
    AXES[ax].setStepsPerUnit(gcode[ax].getFloat());
  }
}

void Motion::disableAllMotors()
{
  for(int ax=0;ax < NUM_AXES;ax++)
    AXES[ax].disable();
}


void Motion::getMovesteps(MGcode& gcode)
{
  for(int ax=0;ax < NUM_AXES;ax++)
  {
    gcode.axismovesteps[ax] = AXES[ax].getMovesteps(gcode.startpos[ax], 
                                                             gcode[ax].isUnused() ? gcode.startpos[ax] : AXES[ax].isRelative() ? gcode.startpos[ax] + gcode[ax].getFloat() : gcode[ax].getFloat(), 
                                                             gcode.axisdirs[ax]);
    if(gcode.movesteps < gcode.axismovesteps[ax])
    {
      gcode.movesteps = gcode.axismovesteps[ax];
      gcode.leading_axis = ax;
    }
  }
}

unsigned long Motion::getLargestStartInterval(MGcode& gcode)
{
  unsigned long mi = 0;
  for(int ax=0;ax < NUM_AXES;ax++)
  {
    if(gcode.axismovesteps[ax] == 0) continue;
    unsigned long t = AXES[ax].getStartInterval(gcode.feed);
    if(t > mi) mi = t;
  }
  return mi;
}

unsigned long Motion::getLargestEndInterval(MGcode& gcode)
{
  unsigned long mi = 0;
  for(int ax=0;ax < NUM_AXES;ax++)
  {
    if(gcode.axismovesteps[ax] == 0) continue;
    unsigned long t = AXES[ax].getEndInterval(gcode.feed);
    if(t > mi) mi = t;
  }
  return mi;
}

unsigned long Motion::getLargestAccelDistance(MGcode& gcode)
{
    unsigned long ad = 0;
    for(int ax=0;ax < NUM_AXES;ax++)
    {
      if(gcode.axismovesteps[ax] == 0) continue;
      unsigned long t = AXES[ax].getAccelDistance();
      if(t > ad) ad = t;
    }
    return ad;
}

void Motion::getActualEndpos(MGcode& gcode)
{
  for(int ax=0;ax<NUM_AXES;ax++)
  {
    gcode.endpos[ax] = AXES[ax].getEndpos(gcode.startpos[ax], gcode.axismovesteps[ax], gcode.axisdirs[ax]);
  }
}


void Motion::gcode_precalc(MGcode& gcode, float& feedin, Point* lastend)
{
  if(gcode.state >= MGcode::PREPARED)
    return;

  
  if(gcode[G].isUnused())
    return;

  if(gcode[G].getInt() == 92)
  {
    // Just carry over new position
    for(int ax=0;ax<NUM_AXES;ax++)
    {
      if(!gcode[ax].isUnused())
      {
        (*lastend)[ax] = gcode[ax].getFloat();
      }
    }
    gcode.state = MGcode::PREPARED;
    return;
  }
  if(gcode[G].getInt() != 1 and gcode[G].getInt() != 2)
  {
    // no precalc for anything but 92, 1, 2
    gcode.state = MGcode::PREPARED;
    return;
  }

  // We want to carry over the previous ending position and feedrate if possible.
  gcode.startpos = *lastend;
  if(gcode[F].isUnused())
    gcode.feed = feedin;
  else
    gcode.feed = gcode[F].getFloat();

  if(gcode.feed == 0)
    gcode.feed = SAFE_DEFAULT_FEED;


  getMovesteps(gcode);
  gcode.startinterval = getLargestStartInterval(gcode);
  gcode.fullinterval = getLargestEndInterval(gcode);
  gcode.currentinterval = gcode.startinterval;
  gcode.steps_to_accel = getLargestAccelDistance(gcode);
  getActualEndpos(gcode);
  gcode.accel_until = gcode.movesteps;
  gcode.decel_from  = 0;
  gcode.accel_inc   = 0;

  long intervaldiff = gcode.startinterval - gcode.fullinterval;
  if(intervaldiff > 0)
  {
    if(gcode.steps_to_accel > gcode.movesteps / 2)
    {
      gcode.accel_until = gcode.movesteps / 2;
      gcode.decel_from  = gcode.accel_until - 1;
    }
    else
    {
      gcode.accel_until = gcode.movesteps - gcode.steps_to_accel;
      gcode.decel_from  = gcode.steps_to_accel;
    }
    gcode.accel_inc = intervaldiff / gcode.steps_to_accel;
  }  

  *lastend = gcode.endpos;
  feedin  = gcode.feed;
  gcode.state = MGcode::PREPARED;
}


void Motion::gcode_execute(MGcode& gcode)
{
  // Only execute codes that are prepared.
  if(gcode.state < MGcode::PREPARED)
    return; // TODO: This should never happen now and is an error.
  // Don't execute codes that are ACTIVE or DONE (ACTIVE get handled by interrupt)
  if(gcode.state > MGcode::PREPARED)
    return;

  // set axis move data, invalidate all precomputes if bad data
  for(int ax=0;ax<NUM_AXES;ax++)
  {
    // AXES[ax].dump_to_host();
    if(!AXES[ax].setupMove(gcode.startpos[ax], gcode.axisdirs[ax], gcode.axismovesteps[ax]))
    {
      GCODES.Invalidate();
      return;
    }
    deltas[ax] = gcode.axismovesteps[ax];
    errors[ax] = gcode.movesteps / 2;
    //AXES[ax].dump_to_host();
  }
  //dumpMovedata(gcode.;
  //gcode.dump_to_host();

  // setup pointer to current move data for interrupt
  gcode.state = MGcode::ACTIVE;
  current_gcode = &gcode;

  setInterruptCycles(gcode.startinterval);
  enableInterrupt();
}

bool Motion::axesAreMoving() 
{ 
  for(int ax=0;ax<NUM_AXES;ax++) 
    if(AXES[ax].isMoving()) return true; 
  
  return false; 
}


void Motion::writePositionToHost()
{
  for(int ax=0;ax<NUM_AXES;ax++)
  {
    HOST.write(ax > Z ? 'A' - Z - 1 + ax : 'X' + ax); HOST.write(':'); HOST.write(AXES[ax].getCurrentPosition(),10,4); HOST.write(' ');
  }
}



void Motion::handleInterrupt()
{
  if(interruptOverflow)
  {
    setInterruptCycles(60000);
    interruptOverflow--;
    return;
  }

  if(current_gcode->movesteps == 0)
  {
    disableInterrupt();
    current_gcode->state = MGcode::DONE;
    return;
  }
  current_gcode->movesteps--;

  if(current_gcode->movesteps > current_gcode->accel_until)
  {
    current_gcode->currentinterval -= current_gcode->accel_inc;
  }
  else if(current_gcode->movesteps < current_gcode->decel_from)
  {
    current_gcode->currentinterval += current_gcode->accel_inc;
  }

  setInterruptCycles(current_gcode->currentinterval);

  for(int ax=0;ax<NUM_AXES;ax++)
  {
    if(ax == current_gcode->leading_axis)
    {
      AXES[ax].doStep();
      continue;
    }

    errors[ax] = errors[ax] - deltas[ax];
    if(errors[ax] < 0)
    {
      AXES[ax].doStep();
      errors[ax] = errors[ax] + deltas[current_gcode->leading_axis];
    }
  }

  if(!axesAreMoving())
  {
    current_gcode->movesteps = 0;
  }
      
  if(current_gcode->movesteps == 0)
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

void Motion::setupInterrupt() 
{
  // 16-bit registers that must be set/read with interrupts disabled:
  // TCNTn, OCRnA/B/C, ICRn
  // "Fast PWM" and no prescaler.
  //TCCR1A = _BV(WGM10) | _BV(WGM11);
  //TCCR1B = _BV(WGM12) | _BV(WGM13) | _BV(CS10);
  TCCR1A = 0;
  TCCR1B = _BV(WGM12) | _BV(CS10);
}


void Motion::enableInterrupt() 
{
  // reset counter in case.
  // TCNT1 = 0;
   // Enable timer
  PRR0 &= ~(_BV(PRTIM1));
  // Outcompare Compare match A interrupt
  TIMSK1 |= _BV(OCIE1A);

}
void Motion::disableInterrupt() 
{
  // Outcompare Compare match A interrupt
  TIMSK1 &= ~(_BV(OCIE1A));
  // Disable timer...
  PRR0 |= _BV(PRTIM1);

}
void Motion::setInterruptCycles(unsigned long cycles) 
{
  if(cycles > 60000)
  {
    OCR1A = 60000;
    interruptOverflow = cycles / 60000;
  }
  else
    OCR1A = cycles;
}





ISR(TIMER1_COMPA_vect)
{
  MOTION.handleInterrupt();
}
