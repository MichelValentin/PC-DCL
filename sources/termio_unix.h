
#ifndef _TERMIO_H_
#define _TERMIO_H_
//#pragma message "Include termio_unix.h"

#define CTRL(c) ((c) & 0x1F) 
/**************************
#define COLOR_BLACK     0
#define COLOR_RED       1
#define COLOR_GREEN     2
#define COLOR_YELLOW    3
#define COLOR_BLUE      4
#define COLOR_MAGENTA   5
#define COLOR_CYAN      6
#define COLOR_WHITE     7
***************************/
#define COLOR_LAST      7

extern  char *termio_color_q[];
enum    termio_color  {BLACK,RED,GREEN,YELLOW,BLUE,MAGENTA,CYAN,WHITE};
 

#endif /* _TERMIO_H_ */
