// LCD pin connections and command definitions.
// Used by the LCD driver to control the 16x2/20x4 character LCD.
#ifndef MAJOR_LCD_DEFINES_H
#define MAJOR_LCD_DEFINES_H

// LCD PIN CONFIGURATION

#define LCD_DATA 0
// LCD data lines start from P0.0.
// P0.0 to P0.7 are used as the 8-bit LCD data bus (D0 = P0.0 ... D7 = P0.7).

#define RW 8
// LCD Read/Write control pin is connected to P0.8.
// RW = 0 -> Write operation
// RW = 1 -> Read operation

#define RS 9
// LCD Register Select pin is connected to P0.9.
// RS = 0 -> Command register selected
// RS = 1 -> Data register selected

#define EN 10
// LCD Enable pin is connected to P0.10.
// A pulse on EN tells the LCD to accept the command/data.


// LCD COMMANDS

#define CLEAR_LCD 0x01
#define RET_CUR_HOME 0X02
#define DISP_OFF 0X08
#define DISP_ON 0X0C
#define DISP_ON_CUR_ON 0X0E
#define DISP_ON_CUR_ON_BLINK 0X0F


// LCD DISPLAY MODE CONFIGURATION

#define MODE_8BIT_1LINE 0X30
#define MODE_8BIT_2LINE 0X38
#define MODE_4BIT_1LINE 0X20
#define MODE_4BIT_2LINE 0X28


// LCD CURSOR POSITION COMMANDS (20x4 LCD)

#define GOTO_LINE1_POS0 0X80
#define GOTO_LINE2_POS0 0XC0
#define GOTO_LINE3_POS0 0X94
#define GOTO_LINE4_POS0 0XD4


// LCD CURSOR / DISPLAY SHIFT COMMANDS

#define SHIFT_CUR_RIGHT 0X06
#define SHIFT_DISP_LEFT 0X10
#define SHIFT_DISP_RIGHT 0X14


// LCD CUSTOM CHARACTER MEMORY

#define GOTO_CGRAM 0X40
// CGRAM stores custom characters (left / right indicator symbols).

#endif
