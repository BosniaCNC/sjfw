#ifndef LIQUID_CRYSTAL_HH
#define LIQUID_CRYSTAL_HH

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "AvrPort.h"
#include "RingBuffer.h"

#include "Time.h"

#define LCD_BUFFER_SIZE 100

class LiquidCrystal {
public:
 
LiquidCrystal(Pin rs, Pin rw, Pin enable,
			     Pin d0, Pin d1, Pin d2, Pin d3,
			     Pin d4, Pin d5, Pin d6, Pin d7,
           uint8_t cols, 
           uint8_t lines, 
           uint8_t linestarts[] 
           ) : commandQueue(LCD_BUFFER_SIZE, command_data),
							       modeQueue(LCD_BUFFER_SIZE, mode_data) 
{                    
  _rs_pin = rs;
  _rw_pin = rw;
  _enable_pin = enable;
  
  _data_pins[0] = d0;
  _data_pins[1] = d1;
  _data_pins[2] = d2;
  _data_pins[3] = d3; 
  _data_pins[4] = d4;
  _data_pins[5] = d5;
  _data_pins[6] = d6;
  _data_pins[7] = d7; 

  _rs_pin.setDirection(true);
  _rs_pin.setValue(false);
  _rw_pin.setDirection(true); 
  _rw_pin.setValue(true);
  _enable_pin.setDirection(true);
  _enable_pin.setValue(false);

  _numlines = lines;
  _numcols  = cols;
  _currline = 0;
  _linestarts = linestarts;

  for (int i = 0; i < 8; i++) {
    _data_pins[i].setDirection(false);
    _data_pins[i].setValue(false);
  }

  setFunction(true, true, false);
  setFunction(true, true, false);
  setFunction(true, true, false);
  setFunction(true, true, false);
  setDisplayControls(true, false, false);
  setEntryMode(false, false);
  writeDDRAM(0);

}

   
  void clear() { command(0x01); }
  void home() { command(0x02); }

  // false, false means shift right don't scroll display
  void setEntryMode(bool left, bool displayscroll) 
  {
    command(0x04 |
           (left ? 0x02 : 0) |
           (displayscroll ? 0x01 : 0));
  }

  void setDisplayControls(bool displayon, bool cursoron, bool blinkon) 
  {
    command(0x08 |
           (displayon ? 0x04 : 0) |
           (cursoron  ? 0x02 : 0) |
           (blinkon   ? 0x01 : 0) );
  }

  // shifts cursor left by default.
  void shift(bool display, bool right)
  {
    command(0x10 |
           (display ? 0x08 : 0) |
           (right   ? 0x04 : 0));
  }         

  void setFunction(bool is8bit, bool is2line, bool fontselect)
  {
    command(0x20 |
           (is8bit      ? 0x10 : 0) |
           (is2line     ? 0x08 : 0) |
           (fontselect? 0x04 : 0));
  }           

  void writeCGRAM(uint8_t location) 
  {
    command(0x40 | location );
  }

  void writeDDRAM(uint8_t location)
  {
    command(0x80 | location );
  }

  void setCursor(uint8_t col, uint8_t row)
  {
    int offset   = _linestarts[row] + col;
    writeDDRAM(offset);
  }

  void write(uint8_t value) { enqueue(value, true); }
  void write(char const *str, uint8_t len) 
  {
    for(int x = 0; x < len; x++) write(str[x]);
  }
  void write(uint16_t d, uint16_t maxradix)
  {
    uint16_t n = d;
    for(uint16_t i = maxradix; i >= 1; i = i/10)
    {
      write((uint8_t)(n/i) + '0');
      n -= (n/i) * i; // This looks like nonsense but it chops off the remainder.
    }
  }

  void handleUpdates()
  {
    if(isBusy())
      return;

    dequeue();
  }

  bool isBusy()
  {
  /*
    static unsigned long prev = millis();
    unsigned long now = millis();
    if(now - prev > 50)
    {
      prev = now;
      return false;
    }
    return true;
  */
   
    _data_pins[7].setDirection(false);
    _rs_pin.setValue(false);
    _rw_pin.setValue(true);

    for (int i = 0; i < 8; i++) {
      _data_pins[i].setDirection(false);
      _data_pins[i].setValue(false);
    }

    // Pulsing enable causes LCD to read this command.
    _enable_pin.setValue(true);
    bool v = _data_pins[7].getValue();
    _enable_pin.setValue(false);

    return v;
  }


private:
  void command(uint8_t value) { enqueue(value, false); }
  void enqueue(uint8_t value, bool mode) {
    if(commandQueue.isFull())
      return;  // TODO: Silent fail.

    commandQueue.push(value);
    modeQueue.push(mode); 
  }

  void dequeue()
  {
    if(commandQueue.isEmpty())
      return;

    // Pull next command off stack
    uint8_t value = commandQueue.pop();
    uint8_t mode  = modeQueue.pop();

    _rw_pin.setValue(false);
    _rs_pin.setValue(mode);

    for (int i = 0; i < 8; i++) {
      _data_pins[i].setDirection(true);
      _data_pins[i].setValue(((value >> i) & 0x01) != 0);
    }

    // Pulsing enable causes LCD to read this command.
    _enable_pin.setValue(true);
    _enable_pin.setValue(false);
  }

  Pin _rs_pin; // LOW: command.  HIGH: character.
  Pin _rw_pin; // LOW: write to LCD.  HIGH: read from LCD.
  Pin _enable_pin; // activated by a HIGH pulse.
  Pin _data_pins[8];

  uint8_t _displayfunction;
  uint8_t _displaycontrol;
  uint8_t _displaymode;

  uint8_t _initialized;

  uint8_t _numlines,_currline;
  uint8_t _numcols;
  uint8_t *_linestarts;

  uint8_t command_data[LCD_BUFFER_SIZE];
  bool    mode_data[LCD_BUFFER_SIZE];
  RingBufferT<uint8_t> commandQueue;
  RingBufferT<bool> modeQueue;
};

#endif // LIQUID_CRYSTAL_HH
