#ifdef _WIN32
//#pragma message "Compiling termio_WIN32"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <windows.h>
#include <string.h>
#include <time.h>
#include <conio.h>
#include <ctype.h>
#include "dcl.h"
#include "dclfindfile.h"
#include "termio.h"

char    *termio_color_q[] = {"BLACK","BLUE","GREEN","CYAN","RED","MAGENTA","BROWN","LIGHTGRAY",
                             "DARKGRAY","LIGHTBLUE","LIGHTGREEN","LIGHTCYAN","LIGHTRED",
                             "LIGHTMAGENTA","YELLOW","WHITE",NULL};

void tio_clrscr(void)
{
//    DWORD written;
//
//    FillConsoleOutputAttribute(GetStdHandle (STD_OUTPUT_HANDLE),
//      terminfo.fg_color + (terminfo.bg_color << 4),
//      terminfo.Size.X * terminfo.Size.Y,
//      (COORD) {0, 0},
//      &written);
//      FillConsoleOutputCharacter (GetStdHandle
//      (STD_OUTPUT_HANDLE), ' ',
//      2000, (COORD) {0, 0}, &written);
//    tio_gotoxy (0, 0);

    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD written;
    COORD coord;
    CONSOLE_SCREEN_BUFFER_INFO info;
    *(DWORD*)&coord = 0;
    GetConsoleScreenBufferInfo(hOutput, &info);
    FillConsoleOutputCharacter(hOutput, ' ', info.dwSize.X*info.dwSize.Y, coord, &written);
    FillConsoleOutputAttribute(hOutput, terminfo.fg_color + (terminfo.bg_color << 4), info.dwSize.X*info.dwSize.Y, coord, &written);
    SetConsoleCursorPosition(hOutput, coord);
//      tio_init_term();
}


void tio_clreol(void)
{
    COORD coord;
    DWORD written;
    CONSOLE_SCREEN_BUFFER_INFO info;

    GetConsoleScreenBufferInfo (GetStdHandle (STD_OUTPUT_HANDLE),
      &info);
    coord.X = info.dwCursorPosition.X;
    coord.Y = info.dwCursorPosition.Y;

    FillConsoleOutputCharacter (GetStdHandle (STD_OUTPUT_HANDLE),
      ' ', info.dwSize.X - info.dwCursorPosition.X, coord, &written);
    tio_gotoxy (coord.X, coord.Y);
}

void tio_gotoxy(int x, int y)
{
    COORD   coord;

    coord.X = (SHORT)x;
    coord.Y = (SHORT)y;
    (void)SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

int tio_wherex(void)
{
    CONSOLE_SCREEN_BUFFER_INFO info;

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
    return info.dwCursorPosition.X;
}


int tio_wherey(void)
{
    CONSOLE_SCREEN_BUFFER_INFO info;

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
    return info.dwCursorPosition.Y;
}

void tio_init_term(void)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    (void)GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    if (csbi.dwCursorPosition.X | csbi.dwCursorPosition.Y) {
        (void)FreeConsole();
        (void)AllocConsole();
    }
    (void)GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    (void)SetConsoleTitle("PC-DCL");
    terminfo.Size.X = csbi.dwSize.X;
    terminfo.Size.Y = csbi.dwSize.Y;
    terminfo.fg_color = csbi.wAttributes & 0x00FF;
    terminfo.bg_color = (csbi.wAttributes >> 4) & 0x00FF;
    strcpy(terminfo.szInfo,"TERMIO_WIN32(CONSOLE)");
    //(void)SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE),  ENABLE_PROCESSED_INPUT);
    (void)SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE),0);
    (void)SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE),
                         ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT);
}

WORD tio_char_attribute(void)
{
    HANDLE  hConsole;
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    memset(&csbi, 0, sizeof(CONSOLE_SCREEN_BUFFER_INFO));
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    (void) GetConsoleScreenBufferInfo(hConsole, &csbi);
    return (csbi.wAttributes);
}

BOOL tio_screen_attr(void)
{
    HANDLE  hConsole;
    BOOL    bSuccess;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD   NumberOfCharsWritten;
    DWORD   nLength;
    COORD   coord;
    WORD    wAttr;

    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    bSuccess = GetConsoleScreenBufferInfo(hConsole, &csbi);
    nLength = (DWORD)(csbi.dwMaximumWindowSize.X * csbi.dwMaximumWindowSize.Y);
    coord.X = 0;
    coord.Y = 0;
    wAttr = (WORD)(terminfo.fg_color | (terminfo.bg_color << 4));
    bSuccess = FillConsoleOutputAttribute(hConsole,
                                          wAttr,
                                          nLength,
                                          coord,
                                          &NumberOfCharsWritten);
    (void) SetConsoleTextAttribute(hConsole, wAttr);
    return (bSuccess);
}

BOOL tio_screen_size(void)
{
    HANDLE  hConsole;
    BOOL    bSuccess;
    SMALL_RECT  rect;

    rect.Bottom = min(53, terminfo.Size.Y) - 1;
    rect.Left   = 0;
    rect.Right  = terminfo.Size.X - 1;
    rect.Top    = 0;

    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    bSuccess = SetConsoleWindowInfo(hConsole, RELATIVE, &rect);
    tio_clrscr();

    return (bSuccess);
}

WORD tio_attr_reverse(void)
{
    WORD    attr;
    WORD    wattr;

    attr = tio_char_attribute();
    wattr = ((attr << 4) & 0x70) + (attr >> 4);
    (void) SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), wattr);
    return(wattr);
}

void tio_print_interrupt(void)
{
    (void)tio_attr_reverse();
    (void) _cprintf(" Interrupt ");
    (void)tio_attr_reverse();
}

void tio_print_exit(void)
{
    (void)tio_attr_reverse();
    (void) _cprintf(" Exit ");
    (void)tio_attr_reverse();
}

BOOL tio_wait_for_char(time_t timeout)
{
    time_t          stoptime    = 0;
    BOOL            go_on       = 0;

    stoptime = time(NULL) + timeout;
    while (stoptime > time(NULL)) {
        if ((go_on = kbhit() ? 1 : 0) != 0) {
            break;
        }
    }
    return(go_on);
}

int _tio_printf(char *buffer)
{
    int ret = 0;

    tio_clreol ();
    if (strchr(buffer,9)) {
        char *ch;
        for (ch = buffer; *ch; ch++) {
//            if (*ch == 9) {
//                int tab = 9 - (tio_wherex() % 8);
//                for(; tab--; (void)putch(' ')) ret++;;
//            }
//            else {
                (void)putch(*ch);
                ret++;
//            }
        }
    }
    else {
        ret = _cprintf("%s",buffer);
    }
    return(ret);
}

int tio_putch(int ch)
{
    return (putch(ch));
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
    int ch = getch();
    if (ch == 0 || ch == 224){
        ch = getch();
        switch (ch) {
        case 82 :   ch = KEY_IC;    break;      /* insert key           */
        case 83 :   ch = KEY_DC;    break;      /* delete key           */
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
    else {
        switch(ch) {
        case CTRL('C'):
        case CTRL('Y') :
            tio_print_interrupt();
            CTRL_Y = 1;
            break;
        case CTRL('Z'):
            tio_print_exit();
            CTRL_Z = 1;
            break;
        default:
            ;
        }
    }
    
    return(ch);
}

BOOL tio_kbhit(void)
{
    return(BOOL)_kbhit();
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
//                    while (!tio_kbhit()) {
//                        if (stoptime == time(NULL) || stoptime == 0) {
//                            (void)tio_gotoxy(0, sy - 1);
//                            tio_print_status();
//                            (void)tio_gotoxy(sx + pos, sy);
//                            stoptime = time(NULL) + 1;
//                        }
//                    }
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
//                tio_print_exit();
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
