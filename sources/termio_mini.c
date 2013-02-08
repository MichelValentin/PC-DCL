#ifndef _WIN32
#ifndef USE_CURSES
#ifndef USE_ANSI
//#pragma message "Compiling termio_mini"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include <term.h>

#include "dcl.h"
#include "platform.h"
#include "termio.h"
#include <readline/readline.h>
#include <readline/history.h>
#include "dbglog.h"

char wrk[MAX_TOKEN+1];

int tio_rl_handler(int count, int key);

char    *termio_color_q[] = {"BLACK","RED","GREEN","YELLOW","BLUE","MAGENTA","CYAN","WHITE",NULL};

void tio_clrscr(void)
{
    //(void)system("clear");
	DEBUGLOG(DEBUGLOG_LEVEL_DEBUG, "<tio_clrscr> clear screen");
	char *buf = tigetstr("clear");
    putp(buf);
}

void tio_clreol(void)
{
	DEBUGLOG(DEBUGLOG_LEVEL_DEBUG, "<tio_clreol> clear to end of line");
	char *buf = tigetstr("el");
    putp(buf);
}

void tio_gotoxy(int x, int y)
{
	DebugLogV(DEBUGLOG_LEVEL_DEBUG, "<tio_gotoxy> gotoxy(%d, %d)", x, y);
	char *cap = tigetstr("cup");
	putp(tparm(cap, y, x));
}

int tio_wherex(void)
{
    int     x = 0;
    return(x);
}


int tio_wherey(void)
{
    int     y = 0;
    return(y);
}

void tio_init_term(void)
{
	memset(&terminfo, 0, sizeof(terminfo));
    terminfo.Size.Y = 24;
    terminfo.Size.X = 80;
    terminfo.bg_color = WHITE;
    terminfo.fg_color = BLACK;
    strcpy(terminfo.szInfo,"TERMIO_MINI");
	using_history();
   	(void)rl_bind_key(CTRL('T'), tio_rl_handler);
}

WORD tio_char_attribute(void)
{
    return(0);
}

BOOL tio_screen_attr(void)
{
    return (1);
}

BOOL tio_screen_size(void)
{
    return (1);
}

//WORD tio_attr_reverse(void)
//{
//    return(wattr);
//}

void tio_print_interrupt(void)
{
	char *buf = tigetstr("rev");
    putp(buf);
    printf("<Interrupt>");
	buf = tigetstr("sgr0");
    putp(buf);
}

void tio_print_exit(void)
{
	char *buf = tigetstr("rev");
    putp(buf);
    printf("<Exit>");
	buf = tigetstr("sgr0");
    putp(buf);
}

BOOL tio_wait_for_char(time_t timeout)
{
    time_t  stoptime    = 0;
    BOOL    go_on       = 0;
    stoptime = time(NULL) + timeout;
    while (stoptime > time(NULL))
        if (tio_kbhit()) {
            go_on = 1;
            break;
        }
    return(go_on);
}

int _tio_printf(char *buffer)
{
    int ret = 0;
	DEBUGLOG(DEBUGLOG_LEVEL_DEBUG, buffer);
    ret = printf("%s", buffer);

    return(ret);
}

int tio_putch(int ch)
{
	sprintf(wrk, "%c", (char)ch);
	DEBUGLOG(DEBUGLOG_LEVEL_DEBUG, wrk);
	putchar(ch);
    return(ch);
}

int tio_printf(const char *format,...)
{
    va_list     ap;
    char        buffer[1024];
    int         ret = 0;

    va_start(ap,format);
    memset(buffer, 0, sizeof(buffer));
    (void)vsprintf(buffer,format,ap);
    ret = _tio_printf(buffer);
    va_end(ap);
    return(ret);
}

int tio_getch(void)
{
    int ch = getchar();
	sprintf(wrk, "%c", (char)ch);
	DEBUGLOG(DEBUGLOG_LEVEL_DEBUG, wrk);
    return(ch);
}

BOOL tio_kbhit(void)
{
    struct timeval tv = { 0, 0 };
    fd_set readfds;

    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    return select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv) == 1;
}

void tio_close(void)
{
}

int	 tio_get_one_line(char *buffer, size_t len, int timeout, Flist stack)
{
	int	ret = 0;
	int	go 	= 1;
	char prompt[MAX_TOKEN];

	if (buffer) {
		*prompt = 0;
		if (*buffer) {
			strcpy(prompt, buffer);
		}
		memset(buffer, 0, len);
    	if (timeout) {
        	go = tio_wait_for_char(timeout);
    	}
    	if (go) {
    		rl_set_prompt((const char *)prompt);
    		char	*line = readline((const char *)prompt);
			if(line) {
				if (*line) {
					DEBUGLOG(DEBUGLOG_LEVEL_DEBUG, line);
					strcpy(buffer, line);
					add_history(line);
				}
				free(line);
			}
    	}
	}
	ret = (int)strlen(buffer);
	return(ret);
}

int tio_rl_handler(int count, int key)
{
	if (key == CTRL('T')) {
		printf("\n");
		tio_print_status();
		printf("\n");
	}
    rl_delete_text(0, rl_end);
    rl_done = 1;
    return(0);
}


#endif
#endif
#endif
