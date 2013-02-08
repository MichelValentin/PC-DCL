//#ifdef USE_CURSES
//#pragma message "Compiling termio_UNIX (CURSES)"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
//#include <sys/time.h>
#include <ctype.h>
#include <stdarg.h>
#include <curses.h>

#include "dcl.h"
#include "dclfindfile.h"
#include "platform.h"
#include "termio.h"

#include "dbglog.h"

char    *termio_color_q[] = {"BLACK","RED","GREEN","YELLOW","BLUE","MAGENTA","CYAN","WHITE",NULL};

void tio_clrscr(void)
{
    clear ();
    refresh();
    tio_gotoxy (0, 0);
}

void tio_clreol(void)
{
    clrtoeol();
    refresh();
}

void tio_gotoxy(int x, int y)
{
    move(y, x);
    refresh();
}

int tio_wherex(void)
{
    int     x = 0;
    int     y = 0;
    getyx(stdscr, y, x);
    return(x);
}


int tio_wherey(void)
{
    int     y = 0;
    int     x = 0;
    getyx(stdscr, y, x);
    return(y);
}

void tio_init_term(void)
{
	DebugLogV(DEBUGLOG_LEVEL_DEBUG, "<tio_init_term>.");
    initscr();
    cbreak();
    //raw();
    noecho();
    nonl();
    keypad(stdscr, TRUE);
    idlok(stdscr, TRUE);
    scrollok(stdscr, TRUE);
    if (has_colors()) {
        start_color();
        color_set(COLOR_PAIR(0), NULL);
    }
    refresh();
    memset(&terminfo, 0, sizeof(terminfo));
    strcpy(terminfo.szInfo,"TERMIO_CURSES");
    terminfo.fg_color = WHITE;
    terminfo.bg_color = BLACK;
    terminfo.Size.Y = 24;
    terminfo.Size.X = 80;
    getmaxyx(stdscr, terminfo.Size.Y, terminfo.Size.X);
}

WORD tio_char_attribute(void)
{
    return(WORD)termattrs();
}

BOOL tio_screen_attr(void)
{
	assume_default_colors(terminfo.fg_color,terminfo.bg_color);
    refresh();
    return (1);
}

BOOL tio_screen_size(void)
{
//    int     rc;
//    rc = resizeterm((int)terminfo.Size.Y, (int)terminfo.Size.X);
//	DebugLogV(DEBUGLOG_LEVEL_DEBUG, "<tio_screen_size> lines=%d col=%d rc=%d.",(int)terminfo.Size.Y, (int)terminfo.Size.X,rc);
//   refresh();
//    return (rc == OK ? 1 : 0);
    return(1);
}

//WORD tio_attr_reverse(void)
//{
//    return(wattr);
//}

void tio_print_interrupt(void)
{
#ifdef __sun__
    printw("<Interrupt>");
#else
    attron(A_REVERSE);
    printw(" Interrupt ");
    attroff(A_REVERSE);
#endif
    refresh();
}

void tio_print_exit(void)
{
#ifdef __sun__
    printw("<Exit>");
#else
    attron(A_REVERSE);
    printw(" Exit ");
    attroff(A_REVERSE);
#endif
    refresh();
}

BOOL tio_wait_for_char(time_t timeout)
{
    time_t  stoptime    = 0;
    BOOL    go_on       = 0;
    int     ch          = 0;
    (void)nodelay(stdscr, 1);
    stoptime = time(NULL) + timeout;
    while (stoptime > time(NULL))
        if ((ch = getch()) != ERR) {
            ungetch(ch);
            go_on = 1;
            break;
        }
    (void)nodelay(stdscr, 0);
    return(go_on);

}

int _tio_printf(char *buffer)
{
    int ret = 0;

    tio_clreol ();
    //DebugLogDump(DEBUGLOG_LEVEL_DEBUG, buffer, strlen(buffer), "_tio_printf");
    ret = printw("%s",buffer);
    refresh();

    /*
    if (strchr(buffer,9)) {
        char *ch;
        for (ch = buffer; *ch; ch++) {
            if (*ch == 9) {
                int tab = 9 - (tio_wherex() % 8);
                for(; tab--; (void)addch(' ')) ret++;;
            }
            else {
                (void)addch(*ch);
                ret++;
            }
        }
    }
    else {
        ret = printw("%s",buffer);
    }
    refresh();
    */
    return(ret);
}

int tio_putch(int ch)
{
    addch(ch);
    refresh();
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
    //DebugLogV(DEBUGLOG_LEVEL_DEBUG, "<tio_printf>%s", buffer);
    ret = _tio_printf(buffer);
    va_end(ap);
    return(ret);
}

int tio_getch(void)
{
    int ch = getch();
    DebugLogV(DEBUGLOG_LEVEL_DEBUG, "<%2.2X> %d %c", ch, ch, ch);
    return(ch);
}

BOOL tio_kbhit(void)
{
    int     ch          = 0;
    (void)nodelay(stdscr, 1);
    if ((ch = getch()) != ERR) {
        ungetch(ch);
    }
    (void)nodelay(stdscr, 0);
    return(ch ? TRUE : FALSE);
/*
    struct timeval
    tv = { 0, 0 };
    fd_set readfds;

       FD_ZERO(&readfds);
       FD_SET(STDIN_FILENO, &readfds);

       return select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv) == 1;
*/
}

void tio_close(void)
{
    endwin();
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
    DCL_FIND_DATA    ff;

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
        case 127:
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
                             if (isspace((int)buffer[i])) break;
                        }
                        if (isspace((int)buffer[i])) i++;
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
                    if (cc < maxlen && ch >= ' ' && ch <= 255) {
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
//#endif


