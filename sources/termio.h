#ifndef TERMIO_H_
#define TERMIO_H_

#ifdef _WIN32
#include "termio_win32.h"
#elif defined(USE_CURSES)
#include <stdarg.h>
#include <curses.h>
#include "termio_unix.h"
#elif defined(USE_ANSI)
#include "termio_ansi.h"
#else
#include "termio_ansi.h"
#endif

void tio_init_term(void);
void tio_clrscr(void);
void tio_clreol(void);
int tio_wherex(void);
int tio_wherey(void);
void tio_gotoxy(int x, int y);
BOOL tio_kbhit(void);
int  tio_getch(void);
int  tio_putch(int ch);
int  tio_printf(const char *format,...);
int _tio_printf(char *buffer);
BOOL tio_wait_for_char(time_t timeout);
void tio_print_interrupt(void);
void tio_print_exit(void);
void tio_print_status(void);
WORD tio_char_attribute(void);
WORD tio_attr_reverse(void);
BOOL tio_screen_size(void);
BOOL tio_screen_attr(void);
void tio_close(void);
int  tio_get_one_line(char *buffer, size_t len, int timeout, Flist stack);
#endif /*TERMIO_H_*/
