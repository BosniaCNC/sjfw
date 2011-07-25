#ifndef _MOVEDATA_H_
#define _MOVEDATA_H_

#include "config.h"
#include "Point.h"
#include <stdint.h>


// There is ABSOLUTELY no reason to have this here and not just in the gcode
// directly.  I had some idea of doing things differently and it would have made
// more sense here if I had.  Bygones!
class Movedata
{
public:
  float feed;

  uint32_t movesteps;
  uint32_t axismovesteps[NUM_AXES];
  bool          axisdirs[NUM_AXES];
  int           leading_axis;

  uint32_t startinterval;
  uint32_t currentinterval;
  uint32_t fullinterval;
  uint32_t steps_to_accel;
  uint32_t accel_until;
  uint32_t decel_from;
  uint32_t accel_inc;

  Point         endpos;
  Point         startpos;
};

#endif // _MOVEDATA_H_
