#ifndef _CONFIG_H_
#define _CONFIG_H_



#include "config-common.h"

THIS CONFIG FILE IS NOT READY FOR USE.

//#define NUM_AXES 4
#define ENDSTOPS_INVERTING 1
#define ENDSTOPPULLUPS 1
#define SAFE_DEFAULT_FEED 1500

#define HOTEND_TEMP_PIN 13
#define PLATFORM_TEMP_PIN 14

#define HOTEND_HEAT_PIN Pin(PortD,6)
#define PLATFORM_HEAT_PIN Pin()

//#define SD_AUTORUN 
#define SD_DETECT_PIN   Pin()
#define SD_WRITE_PIN    Pin()
#define SD_SELECT_PIN   Pin(PortB, 0)


#define X_STEP_PIN      Pin(PortD, 7)
#define X_DIR_PIN       Pin(PortC, 2)
#define X_ENABLE_PIN    Pin(PortC, 3)
#define X_MIN_PIN       Pin(PortC, 4)
#define X_MAX_PIN       Pin()
#define X_INVERT_DIR    false
#define X_HOME_DIR      -1
#define X_STEPS_PER_UNIT 62.745
#define X_MAX_FEED      12000
#define X_AVG_FEED      6000
#define X_START_FEED    2000
#define X_ACCEL_RATE    200
#define X_LENGTH        110
#define X_DISABLE       false

#define Y_STEP_PIN      Pin(PortC, 7)
#define Y_DIR_PIN       Pin(PortC, 6)
#define Y_ENABLE_PIN    Pin(PortA, 7)
#define Y_MIN_PIN       Pin(PortA, 6)
#define Y_MAX_PIN       Pin()
#define Y_INVERT_DIR    false
#define Y_HOME_DIR      -1
#define Y_STEPS_PER_UNIT 62.745
#define Y_MAX_FEED      12000
#define Y_AVG_FEED      6000
#define Y_START_FEED    2000
#define Y_ACCEL_RATE    200
#define Y_LENGTH        110
#define Y_DISABLE       false

#define Z_STEP_PIN      Pin(PortA, 4)
#define Z_DIR_PIN       Pin(PortA, 3)
#define Z_ENABLE_PIN    Pin(PortA, 2)
#define Z_MIN_PIN       Pin(PortA, 1)
#define Z_MAX_PIN       Pin()
#define Z_INVERT_DIR    true
#define Z_HOME_DIR      1
#define Z_STEPS_PER_UNIT 2267.718
#define Z_MAX_FEED      150
#define Z_AVG_FEED      100
#define Z_START_FEED    75
#define Z_ACCEL_RATE    50
#define Z_LENGTH        110
#define Z_DISABLE       true

#define A_STEP_PIN      Pin(PortB, 4)
#define A_DIR_PIN       Pin(PortB, )
#define A_ENABLE_PIN    Pin(PortB, 3)
#define A_INVERT_DIR    false
#define A_HOME_DIR      0
#define A_STEPS_PER_UNIT 729.99
#define A_MAX_FEED      24000
#define A_AVG_FEED      12000
#define A_START_FEED    5000
#define A_ACCEL_RATE    2000
#define A_LENGTH        110
#define A_DISABLE       false


#define LCD_RS_PIN      Pin(PortC,5)
#define LCD_RW_PIN      Pin(PortL,2)
#define LCD_E_PIN       Pin(PortL,4)
#define LCD_0_PIN       Pin(PortL,6)
#define LCD_1_PIN       Pin(PortG,0)
#define LCD_2_PIN       Pin(PortA,5)
#define LCD_3_PIN       Pin(PortC,0)
#define LCD_4_PIN       Pin(PortC,2)
#define LCD_5_PIN       Pin(PortC,4)
#define LCD_6_PIN       Pin(PortC,6)
#define LCD_7_PIN       Pin(PortA,7)

#define LCD_X 16
#define LCD_Y 2

#define LCD_LINESTARTS {0x0, 0x40}
#define LCD_FULLADDRESS 1






#endif // CONFIG_H
