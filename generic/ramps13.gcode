G1 M1 $XJ1K1            (endstop config - must be set /before/ min/max pins on axis)
G1 M1 $XSF0DF1ED7IE5A--N0X0 (set X axis config)
G1 M1 $YSF6DF7EF2IJ1A--N0X0 (set Y axis config)
G1 M1 $ZSL3DL1EK0ID3A--N1X1 (set Z axis config)
G1 M1 $ASA4DA6EA2I--A--N0X0 (set E axis config)
G1 M1 ^SK1 (Set LCD RS)
G1 M1 ^WL7 (Set LCD RW)
G1 M1 ^EK3 (Set LCD Enable)
G1 M1 ^4F5 (Set LCD D4)
G1 M1 ^5K2 (Set LCD D5)
G1 M1 ^6L5 (Set LCD D6)
G1 M1 ^7K4 (Set LCD D7 - always set D7 last, is flag for activation)
G1 M1 &R0H1 (Set Keypad Row0)
G1 M1 &R1H0 (Set Keypad Row1)
G1 M1 &R2A1 (Set Keypad Row2)
G1 M1 &R3A3 (Set Keypad Row3)
G1 M1 &C1A7 (Set Keypad Col1)
G1 M1 &C2C6 (Set Keypad Col2)
G1 M1 &C3C4 (Set Keypad Col3)
G1 M1 &C0A5 (Set Keypad Col0 - always set last, is flag for activation)
M207 P13 (Set hotend thermistor analog pin)
M208 P14 (Set platform thermistor analog pin)
G1 M1 %HB4 (Set FET pin for 'H'otend) 
G1 M1 %BH5 (Set FET pin for 'B'ed)
M200 X62.745 Y62.745 Z2267.718 E729.99 (set steps per mm)
M201 X2000 Y2000 Z75 E5000 (set start speeds)
M203 X2000 Y2000 Z75 E5000 (set 'average' speeds)
M202 X12000 Y12000 Z150 E24000 (set max speeds)
M200 X62.745 Y62.745 Z2267.718 E729.99 (Interdependencies mean it's...)
M201 X2000 Y2000 Z75 E5000             (safest if we repeat the last 4...)
M203 X2000 Y2000 Z75 E5000             (lines, so we don't need to worry...)
M202 X12000 Y12000 Z150 E24000         (about the proper ordering of them.)
M206 X300 Y300 Z50 E2000 (set accel mm/s/s)
G91
G1 X10 Y10
G1 Z5
G1 X-10 Y-10
G1 Z-5
M84


