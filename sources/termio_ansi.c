#if !defined (_WIN32)
#if defined (USE_ANSI)
//#pragma message "Compiling termio_ansi"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include <termios.h>

#include "dcl.h"
#include "dclfindfile.h"
#include "termio.h"
#include "termio_ansi.h"
#include "dbglog.h"

char    *termio_color_q[] = {"BLACK","RED","GREEN","YELLOW","BLUE","MAGENTA","CYAN","WHITE",NULL};

#define ESC 0x1B
static struct termios   save_termios;

int tty_raw(struct termios *save_termios) {
    struct termios  buf;

    tcgetattr(0, save_termios); /* get the original state */

    buf = *save_termios;

    buf.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
                    /* echo off, canonical mode off, extended input
                       processing off, signal chars off */

    buf.c_iflag &= ~(BRKINT | ICRNL | ISTRIP | IXON);
                    /* no SIGINT on BREAK, CR-toNL off, input parity
                       check off, don't strip the 8th bit on input,
                       ouput flow control off */

    buf.c_cflag &= ~(CSIZE | PARENB);
                    /* clear size bits, parity checking off */

    buf.c_cflag |= CS8;
                    /* set 8 bits/char */

    buf.c_oflag &= ~(OPOST);
                    /* output processing off */

    buf.c_cc[VMIN] = 1;  /* 1 byte at a time */
    buf.c_cc[VTIME] = 0; /* no timer on input */

    tcsetattr(0, TCSANOW, &buf);

    return 0;
}


int tty_reset(struct termios *save_termios)
{
	tcsetattr(0, TCSANOW, save_termios);

    return 0;
}

int tty_getch(void)
{
    int ch = 0;
    tty_raw(&save_termios);
    ch = getchar();
    tty_reset(&save_termios);
    return ch;
}

void tio_clrscr(void)
{
	(void)fputc(ESC, stdout);
	(void)fputc('[', stdout);
	(void)fputc('2', stdout);
	(void)fputc('J', stdout);
}

void tio_clreol(void)
{
	(void)fputc(ESC, stdout);
	(void)fputc('[', stdout);
	(void)fputc('K', stdout);
}

void tio_gotoxy(int x, int y)
{
	(void)printf("%c[%d;%dH", ESC, y, x);
}


BOOL tio_query_cursor_position(int *line, int *column)
{
	BOOL	bOK = 1;
	int		i	= 0;
	char	data[16];
	enum 	states {ST_ESC, ST_BRACKET, ST_LINE, ST_COLUMN, ST_END};
	int		state = ST_ESC;

	*line 	= 0;
	*column	= 0;
	(void)fputc(ESC, stdout);
	(void)fputc('[', stdout);
	(void)fputc('6', stdout);
	(void)fputc('N', stdout);

	while(bOK == 1 && tio_kbhit()) {
		int	ch = tty_getch();
		switch(state) {
		case ST_ESC:
			if (ch == ESC) {
				state = ST_BRACKET;
			}
			else {
				bOK = 0;
			}
			break;
		case ST_BRACKET:
			if (ch == '[') {
				state = ST_LINE;
			}
			else {
				bOK = 0;
			}
			break;
		case ST_LINE:
			if (ch == ';') {
				*line = atoi(data);
				state = ST_COLUMN;
			}
			else if (i < (int) sizeof(data) - 1) {
				data[i] = ch;
				i++;
				data[i] = 0;
			}
			else {
				bOK = 0;
			}
			break;
		case ST_COLUMN:
			if (ch == 'R') {
				*column = atoi(data);
				state = ST_END;
			}
			else if (i < (int) sizeof(data) - 1) {
				data[i] = ch;
				i++;
				data[i] = 0;
			}
			else {
				bOK = 0;
			}
			break;
		case ST_END:
			break;
		default:
			bOK = 0;
		}
	}
	return(bOK);
}

int tio_wherex(void)
{
	int	line	= 0;
	int column	= 0;

	(void)tio_query_cursor_position(&line, &column);

	return(column);
}


int tio_wherey(void)
{
	int	line	= 0;
	int column	= 0;

	(void)tio_query_cursor_position(&line, &column);

	return(line);
}

void tio_init_term(void)
{
	memset(&terminfo, 0, sizeof(terminfo));
    terminfo.Size.Y = 24;
    terminfo.Size.X = 80;
    strcpy(terminfo.szInfo,"TERMIO_ANSI");
	(void)fputc(ESC, stdout);
	(void)fputc('c', stdout);
}

WORD tio_char_attribute(void)
{
	return(0);
}

BOOL tio_screen_attr(void)
{
	return(1);
}

BOOL tio_screen_size(void)
{
	return(1);
}

WORD tio_attr_reverse(void)
{
//	<ESC>[7;m
    return(0);
}

void tio_print_interrupt(void)
{
    (void)tio_attr_reverse();
    (void)printf("<Interrupt>");
    (void)tio_attr_reverse();
}

void tio_print_exit(void)
{
    (void)tio_attr_reverse();
    (void)printf("<Exit>");
    (void)tio_attr_reverse();
}

BOOL tio_wait_for_char(time_t timeout)
{
    time_t  stoptime    = 0;
    BOOL    go_on       = 0;
	struct termios save_termios;

	tty_raw(&save_termios);
    stoptime = time(NULL) + timeout;
    while (stoptime > time(NULL)) {
        if ((go_on = tio_kbhit() ? 1 : 0) != 0) {
            break;
        }
    }
	tty_reset(&save_termios);
    return(go_on);
}

int _tio_printf(char *buffer)
{
    int ret = 0;

    tio_clreol ();
    ret = printf("%s",buffer);

    return(ret);
}

int tio_putch(int ch)
{
    return (putchar(ch));
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
	DebugLogV(DEBUGLOG_LEVEL_DEBUG, "<%X> %c", ch, ch);
    if (ch == 0 || ch == 224){
        ch = getchar();
        switch (ch) {
        case 82 :   ch = KEY_IC;    break;      /* insert key           */
        case 83 :   ch = KEY_DC;    break;      /*  delete key          */
        case 71 :   ch = KEY_HOME;  break;      /* home key             */
        case 79 :   ch = KEY_END;   break;      /* end key              */
        case 72 :   ch = KEY_UP;    break;      /* up arroww            */
        case 80 :   ch = KEY_DOWN;  break;      /* Down arrow           */
        case 75 :   ch = KEY_LEFT;  break;      /* left arrow           */
        case 77 :   ch = KEY_RIGHT; break;      /* right arrow          */
        default :   /* Test F1..F12 SF1..SF12      */
                    if (ch > 58 && ch < 69) {
                        ch = KEY_F(ch - 58);
                    }
                    else if (ch > 83 && ch < 97) {
                        ch = KEY_F(ch - 71);
                    }
                    else if (ch == 133 || ch == 134) {
                        ch = KEY_F(ch - 122);
                    }
                    else if (ch == 135 || ch == 136) {
                        ch = KEY_F(ch - 112);
                    }
                    else {
                        ch = 0;
                    }
        }
    }
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

int  tio_get_one_line(char *buffer, size_t maxlen, int timeout, Flist stack)
{
    int  insert_mode = default_insert_mode;
    int  cc = 0;
    int  ch;
    int  pos = 0;
    int  i  = 0;
    int  go_on = 1;
    int  sx;
    int  sy;
    char wrk[256];
    time_t  stoptime;
    int     fTab = 0;
    int     fFile = 0;
    char    szSearch[256];
	char    szDrive[_MAX_DRIVE];
    char    szDir[_MAX_DIR];
	int    handle = (int)INVALID_HANDLE_VALUE;
	DCL_FIND_DATA	ff;

    CTRL_Y = 0;
    CTRL_Z = 0;

    if (buffer == NULL) return(0);

    sx = tio_wherex();
    sy = tio_wherey();
    cc = (int)strlen(buffer);
    if (cc < 0) cc = 0;
    pos = cc;
    (void)tio_clreol();
    (void)tio_gotoxy(sx, sy);
    (void)tio_printf("%s",buffer);
    (void)tio_gotoxy(sx + pos, sy);

    if (timeout) {
        go_on = tio_wait_for_char(timeout);
    }

    while (go_on) {
        ch = tio_getch();
        if (ch != 9) {

            fTab = 0;
            if (handle != (int)INVALID_HANDLE_VALUE) {
                Dcl_FindClose(handle);
				handle = (int)INVALID_HANDLE_VALUE;
            }
        }
        switch (ch) {
        /****** CTRL A  (Insert)    ******/
        case CTRL('A') :
        case KEY_IC :
	        	if (insert_mode) {
	            	insert_mode = OVERSTRIKE;
	            }
	            else {
	                insert_mode = INSERT;
	                }
	            break;
        /******  CTRL B  (Up arrow)  ******/
        case CTRL('B') :
        case KEY_UP :
                if (! nextdptr(stack,buffer)) {
                    *buffer = 0;
                }
                cc = (int) strlen(buffer);
                pos=cc;
                (void)tio_gotoxy(sx, sy);
                (void)tio_clreol();
                (void)tio_printf("%s",buffer);
                (void)tio_gotoxy(sx + pos, sy);
                break;
        /******  CTRL C, CTRL Y  (Interrupt)  ******/
        case CTRL('C'):
        case CTRL('Y') :
                tio_print_interrupt();
                *buffer = 0;
                cc = 0;
                go_on = 0;
                CTRL_Y = 1;
                break;
        /******  CTRL D (Left arrow)  ******/
        case CTRL('D'):
        case KEY_LEFT :
                if (pos > 0) {
                    --pos;
                    (void)tio_gotoxy(sx + pos, sy);
                }
                break;
        /******  CTRL E (End of line)  ******/
        case CTRL('E'):
        case KEY_END :
                pos=cc;
                (void)tio_gotoxy(sx + pos, sy);
                break;
        /******  CTRL F (Right arrow)  ******/
        case CTRL('F'):
        case KEY_RIGHT :
                if (pos < cc) {
                    ++pos;
                    (void)tio_gotoxy(sx + pos, sy);
                }
                break;
        /******  backspace key  ******/
        case  8 :
        case KEY_BACKSPACE:
                if (pos > 0) {
                    for (i=pos;i<cc;i++) buffer[i-1]=buffer[i];
                    --cc;
                    buffer[cc]=0x00;
                    --pos;
                    (void)tio_gotoxy(sx + pos, sy);
                    (void)tio_clreol();
                    (void)tio_printf("%s",&buffer[pos]);
                    (void)tio_gotoxy(sx + pos, sy);
                }
                break;
        /******  TAB key  ******/
        case  9 :
				if (pos > 0) {
                	if (fTab == 0) {
                    	for(i = pos; i > 0; i--) {
     						if (isspace(buffer[i])) break;
	    	    		}
						if (isspace(buffer[i])) i++;
						if (buffer[i] == '@') i++;
						if (buffer[i] != 0) {
							strcpy(szSearch, &buffer[i]);
							strcat(szSearch, "*");
							_splitpath(szSearch,szDrive,szDir,NULL, NULL);
                        }
                    }
                    if (handle == (int)INVALID_HANDLE_VALUE) {
    					handle = Dcl_FindFirstFile(szSearch, &ff);
                        if (handle != (int)INVALID_HANDLE_VALUE) {
                        	fFile = 1;
                            fTab  = 1;
                        }
                    }
                    else {
                        fFile = Dcl_FindNextFile(handle, &ff);
                    }
					if (fFile) {
						strcpy(wrk, szDrive);
						strcat(wrk, szDir);
					    strcat(wrk, ff.cFileName);
						strcpy(&buffer[i], wrk);
					}
                    else {
                        handle = (int)INVALID_HANDLE_VALUE;
						strcpy(&buffer[i], szSearch);
                        buffer[strlen(buffer)-1] = 0;
                    }
					cc = strlen(buffer);
					pos = cc;
					(void)tio_gotoxy(sx, sy);
					(void)tio_clreol();
					(void)tio_printf("%s",buffer);
					(void)tio_gotoxy(sx + pos, sy);
				}
                break;
        /******  CTRL K, erase end of line  ******/
        case CTRL('K'):
                buffer[pos] = 0;
                cc = pos;
                (void)tio_clreol();
                break;
        /****** ENTER / LF / CR ******/
        case 10 :
        case 13 :
                go_on = 0;
                break;
        /****** CTRL T, (Display Time) ******/
        case CTRL('T'):
                if (CONTROL_T_ALLOWED)  {
                    stoptime = 0;
                    (void)tio_printf("\n");
                    tio_print_status();
                    (void)tio_printf("\n");
                    (void)tio_printf("%s",dcl_prompt);
                    (void)tio_printf("%s",buffer);
                    sy = tio_wherey();
                    (void)tio_gotoxy(sx, sy);
                    while (!tio_kbhit()) {
                        if (stoptime == time(NULL) || stoptime == 0) {
                            (void)tio_gotoxy(0, sy - 1);
                            tio_print_status();
                            (void)tio_gotoxy(sx + pos, sy);
                            stoptime = time(NULL) + 1;
                        }
                    }
                }
                break;
        /****** CTRL U, (Erase from BoL) ******/
        case CTRL('U'):
                while (pos > 0) {
                    for (i=pos;i<cc;i++) buffer[i-1]=buffer[i];
                    --cc;
                    buffer[cc]=0x00;
                    --pos;
                }
                (void)tio_gotoxy(sx, sy);
                (void)tio_clreol();
                (void)tio_gotoxy(sx, sy);
                (void)tio_printf("%s",buffer);
                (void)tio_gotoxy(sx + pos, sy);
                break;
        /****** CTRL Z, (Exit) ******/
        case CTRL('Z'):
                tio_print_exit();
                *buffer = 0;
                cc = 0;
                go_on = 0;
                CTRL_Z = 1;
                break;
        /****** ESCAPE key ******/
        case 27 :
                (void)tio_gotoxy(sx, sy);
                (void)tio_clreol();
                *buffer = 0;
                cc = 0;
                go_on = 0;
                break;
        /****** DELETE key ******/
		case KEY_DC:
                if (pos < cc) {
                    for (i=pos;i<cc;i++) buffer[i]=buffer[i+1];
                    buffer[cc-1]='\0';
                    --cc;
                    (void)tio_gotoxy(sx + pos, sy);
                    (void)tio_clreol();
                    (void)tio_printf("%s",&buffer[pos]);
                    (void)tio_gotoxy(sx + pos, sy);
                }
                break;
        /****** HOME key ******/
		case KEY_HOME:
                pos=0;
                (void)tio_gotoxy(sx, sy);
                 break;
        /******  DOWN arrow  ******/
        case KEY_DOWN :
                if (! prevdptr(stack,buffer))
                    *buffer = 0;
                cc = (int)strlen(buffer);
                pos=cc;
                (void)tio_gotoxy(sx, sy);
                (void)tio_clreol();
                (void)tio_printf("%s",buffer);
                (void)tio_gotoxy(sx + pos, sy);
                break;
        default:
        		/* TEST FKEY F1..F12 SF1..SF12 */
        		if (ch > KEY_F0 && ch <= (KEY_F0+24)) {
                	i = ch - KEY_F0;
                	i = i - 1;
                    if (i >= 0 && i < 24 && keydef[i].value != NULL){
                        if (keydef[i].erase){
                            *buffer = 0;
                            strcpy(buffer,keydef[i].value);
                            cc = (int)strlen(buffer);
                            pos = cc;
                        }
                        else {
                            char *p;
                            int  j;
                            if (insert_mode){
                                p = keydef[i].value;
                                strcpy(wrk,&buffer[pos]);
                                for(j=0; j < maxlen && *p; buffer[j++] = *p++) ;
                                p = wrk;
                                for(j=0; j < maxlen && *p; buffer[j++] = *p++) ;
                            }
                            else {
                                p = keydef[i].value;
                                strcpy(wrk,&buffer[pos]);
                                for(j=0; j < maxlen && *p; buffer[j++] = *p++) ;
                            }
                            cc = (int)strlen(buffer);
                        }
                        if (keydef[i].echo){
                            (void)tio_gotoxy(sx, sy);
                            (void)tio_clreol();
                            (void)tio_gotoxy(sx, sy);
                            (void)tio_printf("%s",buffer);
                            pos = cc;
                            (void)tio_gotoxy(sx + pos, sy);
                        }
                        if (keydef[i].terminate){
                            go_on = 0;
                        }
                    }
                }
        		/* NORMAL character */
        		else {
                    if (cc < maxlen && ch >= ' ') {
                        if (pos == cc) {
                            buffer[pos] = (char)ch;
                            (void)tio_putch(ch);
                            ++cc;
                            buffer[cc] = 0x00;
                            if (cc == maxlen)
                                go_on = FALSE;
                        }
                        else
                            if (insert_mode) {
                                for(i=cc;i>pos;--i) buffer[i]=buffer[i-1];
                                buffer[pos] = (char)ch;
                                ++cc;
                                buffer[cc] = 0x00;
                                (void)tio_gotoxy(sx + pos, sy);
                                (void)tio_printf("%s",&buffer[pos]);
                            }
                            else {
                                buffer[pos] = (char)ch;
                                (void)tio_putch(ch);
                            }
                        ++pos;
                        (void)tio_gotoxy(sx + pos, sy);
                    }
        		}
        }                                   /*  end switch    */
    }                                       /*  end while     */
    (void)tio_putch('\n');
//    (void)tio_putch('\r');
    if (handle != (int)INVALID_HANDLE_VALUE) {
        Dcl_FindClose(handle);
    }

    return(cc);
}
#endif
#endif

