#ifndef _TERMIO_H_
#define _TERMIO_H_

//#pragma message "Include termio_WIN32.h"

#define CTRL(c) ((c) & 0x1F) 

#define KEY_BACKSPACE   0xFFF   /* Backspace key (dummy)         */
#define KEY_DOWN        0x102   /* Down arrow key                */
#define KEY_UP          0x103   /* Up arrow key                  */
#define KEY_LEFT        0x104   /* Left arrow key                */
#define KEY_RIGHT       0x105   /* Right arrow key               */
#define KEY_HOME        0x106   /* home key                      */
#define KEY_DC          0x14a   /* delete character              */
#define KEY_IC          0x14b   /* insert char or enter ins mode */
#define KEY_END         0x166   /* end key                       */

#define KEY_F0          0x108   /* function keys. space for      */
#define KEY_F(n)    (KEY_F0+(n))/* 64 keys are reserved.         */


#define COLOR_BLACK         0
#define COLOR_BLUE          1
#define COLOR_GREEN         2
#define COLOR_CYAN          3
#define COLOR_RED           4
#define COLOR_MAGENTA       5
#define COLOR_BROWN         6
#define COLOR_LIGHTGRAY     7
#define COLOR_DARKGRAY      8
#define COLOR_LIGHTBLUE     9
#define COLOR_LIGHTGREEN    10
#define COLOR_LIGHTCYAN     11
#define COLOR_LIGHTRED      12
#define COLOR_LIGHTMAGENTA  13
#define COLOR_YELLOW        14
#define COLOR_WHITE         15
#define COLOR_LAST          15

extern char *termio_color_q[];

enum    termio_color  {BLACK,BLUE,GREEN,CYAN,RED,MAGENTA,BROWN,LIGHTGRAY,
                       DARKGRAY,LIGHTBLUE,LIGHTGREEN,LIGHTCYAN,LIGHTRED,
                       LIGHTMAGENTA,YELLOW,WHITE};

#endif /* _TERMIO_H_ */
