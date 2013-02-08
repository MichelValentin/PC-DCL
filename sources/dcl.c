/****************************************************************************/
/*                                                                          */
/*   PC-DCL -  DCL command line interpreter for Windows.                    */
/*   Copyright (C) 1991-2009 by Michel Valentin                             */
/*                                                                          */
/*   This program is free software; you can redistribute it and/or modify   */
/*   it under the terms of the enclosed   license.                          */
/*                                                                          */
/*   This program is distributed in the hope that it will be useful,        */
/*   but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                   */
/*                                                                          */
/****************************************************************************/
/*lint -e801 use of goto is deprecated*/
/*lint -e818 * could be declared as constant*/
/*lint -e661 off by one */

#include <ctype.h>
#include <time.h>
#ifdef _WIN32
//#include <dos.h>
#include <direct.h>
#endif
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>

#include "platform.h"
#include "dcl.h"
#include "step.h"
#include "dclfindfile.h"
#include "termio.h"
#include "dbglog.h"

static void dcl_init(int argc,char **argv);
static void dcl_read_kbd(void);
static void dcl_read_file(void);
static void dcl_end(void);
static void dcl_init_default_logicals(void);
static void dcl_translate_symbol(void);
static void dcl_translate_lexicals(void);
static int  dcl_command(void);
static int  dcl_exit_proc_level(void);
static int  dcl_exit_cmd_level(void);
static int  dcl_file(void);
static int  dcl_enter_file(char *vmsfile,char *outfile,char **p);
static int  dcl_enter_cmd_level(void);
static int  dcl_load_file(FILE *fp);
static int  next_parenthesis(char *str);
static int  dcl_supress_comment(void);
static int dcl_exit_file(void);
static int dcl_batman(char *command);
static int dcl_robin(char *command, char *file, char *param);
extern char *cmd_get_token(char *str,char *token);
extern char *cmd_find_token(char *str);
extern int dcl_change_dir(char *vms);

char    *COPYRIGHT = "PC-DCL Copyright M.VALENTIN 1991-2007";
char    INI_NAME[MAX_TOKEN];
char    INI_VALUE[MAX_TOKEN];
char    INI_HOME[_MAX_PATH];
char    INI_DCL[_MAX_PATH];
char *err_msg[] =  {"write protect",
                    "unknown unit",
                    "drive not ready",
                    "unknown command",
                    "data error (CRC)",
                    "bad request",
                    "seek error",
                    "unknown media type",
                    "sector not found",
                    "printer out of paper",
                    "write fault",
                    "read fault",
                    "general failure",
                    "reserved",
                    "reserved",
                    "invalid disk change"
                    };
char  EXP_str[1024];
int   EXP_pos;
int   EXP_token;
long  EXP_number_value;
char  EXP_string_value[1024];

DCL     dcl[8];
CMD     cmd[8];
FILE    *dclfile[64];
//Flist   logical_list;
Flist   cmd_stack;
Flist   data_stack;
int     _SEVERITY   = 0;
int     _STATUS     = 0;
int     _MSTATUS    = 0;
int     _MSEVERITY  = 0;
int     must_stop = 1;
int     D;
int     C;
int     dcl_state = DCL_INIT;
int     dcl_subr = 0;
int     verify = 0;
int     mode;
char    dcl_line[1024];
char    dcl_prompt[33] = "$ ";
char    dcl_org_prompt[33] = "$ ";
time_t  start_time;
int     DI_err = 0;
int     HARDERR = 0;
int     recall = 0;
KEYDEF  keydef[24];
SEARCH  search[9];
char    CTRL_Y = 0;
char    CTRL_Z = 0;
int     EXCPT = 0;
TERMINFO terminfo;
int      break_status;
char    CONTROL_Y_ALLOWED = 1;
char    CONTROL_T_ALLOWED = 1;
char    SUBDIR = 0;
char    SWITCHAR = '/';
char    history_filename[_MAX_PATH];

#ifdef _WIN32
BOOL WINAPI dcl_break_handler(DWORD dwCtrlType)
{
    if (dwCtrlType == CTRL_C_EVENT || dwCtrlType == CTRL_BREAK_EVENT) {
        DebugLogV(DEBUGLOG_LEVEL_DEBUG, "<dcl_break_handler> %x.", dwCtrlType);
        if (CONTROL_Y_ALLOWED) {
            CTRL_Y = 1;
            HARDERR = 1;
        }
        return(TRUE);
    }
    return(FALSE);
}
#else

void catch_int(int sig_num)
{
    signal(SIGINT, catch_int);
    if (CONTROL_Y_ALLOWED) {
        CTRL_Y = 1;
        HARDERR = 1;
    }
}
//void catch_brk(int sig_num)
//{
//    signal(SIGINT, catch_brk);
//    if (CONTROL_Y_ALLOWED) {
//        CTRL_Y = 1;
//        HARDERR = 1;
//    }
//}
#endif

int main(int argc, char **argv)
{
    int   i;
    dcl_init_term();
    dcl_init(argc,argv);

    while(dcl_state){
        switch (dcl_state) {
        case DCL_INIT:     strcpy(dcl_line, "@SYS$DCL:login");
                           dcl_state = DCL_LOGIN;
                           break;
        case DCL_INIT_HOME:  strcpy(dcl_line, "@SYS$LOGIN:login");
                             dcl_state = DCL_LOGIN_HOME;
                             break;
        case DCL_ARG   :  *dcl_line = 0;
                           if (argc > 1) {
                                for (i = 1; i < argc; i++) {
                                    strcat(dcl_line,argv[i]);
                                    strcat(dcl_line," ");
                                    }
                              cmd[C].stop = 1;
                              }
                           dcl_state = DCL_RUN;
                           break;
        case DCL_LOGOUT_HOME: strcpy(dcl_line, "@SYS$LOGIN:logout");
                           cmd[C].stop = 2;
                           dcl_state = DCL_RUN;
                           break;
        case DCL_LOGOUT:   strcpy(dcl_line, "@SYS$DCL:logout");
                           cmd[C].stop = 3;
                           dcl_state = DCL_RUN;
                           break;
        case DCL_LOGIN :
        case DCL_LOGIN_HOME :
        case DCL_RUN   :  if (C == 1) {
                              if (cmd[C].stop == 0) {
                                 dcl_read_kbd();
                                }
                              else
                                 {
                                 *dcl_line = 0;
                                 dcl_state = DCL_EXIT;
                                 }
                              }
                           else
                              dcl_read_file();
                           break;
        default        :  dcl_state = DCL_EXIT;
        }
        (void)dcl_process_command_line(0);
    }
    dcl_end();

    return(0);
}

int dcl_supress_comment(void)
{
    char    *ch     = dcl_line;
    char    *w      = dcl_line;
    int     quoting = 0;
    int     comment = 0;
    char    token[MAX_TOKEN];
    size_t  i;

    while (*ch && (*ch == ' ' || *ch == '\t')) {
        if (*ch == '\t') *ch = ' ';
        ch++;
        }
    if (*ch == '$') *ch = ' ';
    while (*ch && (*ch == ' ' || *ch == '\t')) {
        if (*ch == '\t') *ch = ' ';
        ch++;
        }

    *token = 0;
    w = dcl_get_token(ch,token);
    i = *token ? strlen(token)-1 : 0;
    if (token[i] == ':' && token[i+1] != '=' && C > 1) {
        i = strlen(token);
        while (i) {
            w--;
            *w = ' ';
            i--;
            }
        }

    while (*ch){
        if (*ch == '"') quoting = quoting ? 0 : 1;
        if (*ch == '!' && !quoting) comment = 1;
        if (comment) *ch = ' ';
        ch++;
        }

    return(0);
}

void dcl_init(int argc,char **argv)
{
    int i;

    default_insert_mode = OVERSTRIKE;
    mode = argc > 1 ? MODE_BATCH : MODE_INTERACTIVE;
    start_time = time(NULL);
    if (mode == MODE_INTERACTIVE) {
        tio_printf("PC-DCL V%s (%3.3s %s)\n",VERSION,__DATE__,&__DATE__[7]);
    }
//    logical_list = mkFlist(sizeof(LOGICAL),0);
//    if (logical_list == 0) {
//        printf("FATAL ERROR - MA0001\n");
//        exit(99);
//      }

    (void)dclini(argc,argv);
    dcl_init_default_logicals();

    for (D = 0; D <= 1; D++) {
        dcl[D].SYS_INPUT  = stdin;
        dcl[D].SYS_OUTPUT = stdout;
        dcl[D].symbol_scope = SYMSCO_GLOBAL + SYMSCO_LOCAL;
        dcl[D].symbol_list = mkFlist(sizeof(SYMBOL),0);
        if (dcl[D].symbol_list == 0) {
            printf("FATAL ERROR - MA0002\n");
            exit(99);
            }
        dcl[D].ccl = 0;
        dcl[D].cc[dcl[D].ccl] = 1;
        }
    D = 1;

    for (C = 0; C <= 1; C++) {
        cmd[C].cmd        = NULL;
        cmd[C].label      = NULL;
        cmd[C].stack      = NULL;
        cmd[C].l_stack    = NULL;
        cmd[C].ln         = 0;
        cmd[C].stop       = 0;
        }
    C = 1;

    cmd_stack = mkFlist(256,0);
    {
        FILE    *fp = NULL;
        char    buffer[1024];
        strcpy(history_filename, INI_HOME);
        strcat(history_filename, ".dcl_history");
        fp = fopen(history_filename, "r");
        if (fp != NULL) {
            while(fgets(buffer, sizeof(buffer), fp)) {
                while(buffer[strlen(buffer)-1] <= ' ') {
                    buffer[strlen(buffer)-1] = 0;
                }
                (void)mkcdptr(cmd_stack,0);
                (void)insdptr(cmd_stack,buffer);
                while (nempty(cmd_stack) > MAX_KBD_STACK + 1){
                    (void)mkcdptr(cmd_stack,nempty(cmd_stack));
                    (void)deln(cmd_stack);
                }
            }
            fclose(fp);
        }
    }

    data_stack = mkFlist(256,0);

    for (i = 0;i < 24; i++){
        keydef[i].value = NULL;
        keydef[i].echo = 0;
        keydef[i].erase = 0;
        keydef[i].terminate = 0;
        }

    for(i = 0; i < 64; i++) {
        dclfile[i] = NULL;
    }
}

void dcl_init_default_logicals(void)
{
    char    name[MAX_TOKEN];
    char    value[MAX_TOKEN];
    char    temp[MAX_TOKEN];

    strcpy(name,"SYS$INPUT");
    strcpy(value,"SYS$INPUT");
    (void)logical_put(name,value,LOG_SYSTEM);

    strcpy(name,"SYS$OUTPUT");
    strcpy(value,"SYS$OUTPUT");
    (void)logical_put(name,value,LOG_SYSTEM);

#ifdef _WIN32
    strcpy(name,"SYS$DISK");
    value[0] = (char)(_getdrive() + 64);
    value[1] = ':';
    value[2] = 0;
    (void)logical_put(name,value,LOG_SYSTEM);

    strcpy(name,"SYS$LOGIN_DEVICE");
    value[0] = (char)(_getdrive() + 64);
    value[1] = ':';
    value[2] = 0;
    (void)logical_put(name,value,LOG_SYSTEM);
#endif
#ifdef __linux
    DISKINFO di;
    if (diskinfo(INI_HOME, &di)==DCL_OK) {
        strcpy(name,"SYS$DISK");
    	strcpy(value,di.szName);
        (void)logical_put(name,value,LOG_SYSTEM);
        strcpy(name,"SYS$LOGIN_DEVICE");
    	strcpy(value,di.szName);
        (void)logical_put(name,value,LOG_SYSTEM);
    }
#endif
    cvfs_dos_to_vms(INI_HOME,value);
    strcpy(name,"SYS$LOGIN");
    (void)logical_put(name,value,LOG_SYSTEM);

    strcpy(name,"SYS$SCRATCH");
    (void)logical_put(name,value,LOG_SYSTEM);

    cvfs_dos_to_vms(INI_DCL,value);
    strcpy(name,"SYS$DCL");
    (void)logical_put(name,value,LOG_SYSTEM);

    strcpy(name,"SYS$HELPDIR");
    strcpy(temp,INI_DCL);
    strcat(temp,"help");
    strcat(temp,SLASH_STR);
    cvfs_dos_to_vms(temp,value);
    (void)logical_put(name,value,LOG_SYSTEM);

}

void dcl_read_kbd(void)
{
    int     i;
    char    *w;
    if (!recall)
        (void)memset(dcl_line,0,256);
    recall = 0;
    (void)tio_printf("%s",dcl_prompt);

    if (dcl[D].SYS_INPUT == stdin) {
        (void)kbdread(dcl_line,256,cmd_stack,INFINITE_TIMEOUT);
        i = (int)strlen(dcl_line);
        if (i) i--;
        while (dcl_line[i] == '-' && i < 768) {
            w = &dcl_line[i];
            (void)memset(w,0,256);
            (void)dcl_printf(dcl[D].SYS_OUTPUT,"_%s",dcl_prompt);
            (void)kbdread(w,256,cmd_stack,INFINITE_TIMEOUT);
            i = (int)strlen(dcl_line) - 1;
        }
    }
}

void dcl_read_file(void)
{
    unsigned int     i;
    char             *w;
    PARAM_T          *p,*q;

    (void)memset(dcl_line,0,256);
    if (!rcld(cmd[C].cmd,dcl_line,(unsigned)cmd[C].ln)) {
        (void)cmd_prepro(DUMMY_PARAM,DUMMY_QUAL,&p,&q);
        (void)dcl_exit(p,q);
        free(p); free(q);
        cmd[C].ln--;
    }
    else {
        i = strlen(dcl_line) - 1;
        while (i < 768 && dcl_line[i] == '-') {
            w = &dcl_line[i];
            (void)memset(w,0,256);
            cmd[C].ln++;
            (void)rcld(cmd[C].cmd,dcl_line,(unsigned)cmd[C].ln);
            i = strlen(dcl_line) - 1;
        }
    }
    if (STEP) {
        int saved_line = cmd[C].ln;
        strcpy(STEP_line,dcl_line);
        tio_printf("*STEP>> %s\n",dcl_line);
        STEP_loop = 1;
        while (STEP_loop) {
            tio_printf("*STEP>> ");
            *dcl_line = 0;
            (void)kbdread(dcl_line,256,cmd_stack,INFINITE_TIMEOUT);
            (void)dcl_process_command_line(1);
        }
        strcpy(dcl_line,STEP_line);
        cmd[C].ln = saved_line;
    }
}

int dcl_process_command_line(int exception)
{
    char token1[MAX_TOKEN];
    char token2[MAX_TOKEN];
    char *line    = dcl_line;
    size_t i;

    *token1 = 0;
    *token2 = 0;
    _MSTATUS = _STATUS;
    _MSEVERITY = _SEVERITY;
    _SEVERITY = 0;
    EXCPT = exception;

    dcl_translate_symbol();

    if ((verify & VERIFY_PROC) && *dcl_line && !cmd[C].subr
        && dcl[D].cc[dcl[D].ccl] && C > 1)
        (void)dcl_printf(dcl[D].SYS_OUTPUT,"%s\n",dcl_line);

    (void)dcl_supress_comment();

    dcl_translate_lexicals();

    line = dcl_get_token(line,token1);
    i = *token1 ? strlen(token1)-1 : 0;
    if (token1[i] == ':' && C > 1) {
        line = dcl_get_token(line,token1);
        }
    line = dcl_get_token_raw(line,token2);
    if (strncmp(token2,"=",1)  == 0 || strncmp(token2,"==",2)  == 0)
        (void)dcl_sym_assign(token1,token2);
    else if (strncmp(token2,":=",2) == 0 || strncmp(token2,":==",3) == 0)
            (void)dcl_string_assign(token1,token2);
        else (void)dcl_command();

    if (exception == 2) {
        exception = 0;
        EXCPT = exception;
    }

    if (D && dcl[D].ON && exception == 0 && _SEVERITY){
        if (_SEVERITY >= dcl[D].ON_severity){
            strcpy(dcl_line,dcl[D].ON_error_command);
            (void)dcl_process_command_line(1);
        }
    }

    if (CTRL_Y) {
        CTRL_Y = 0;
        strcpy(dcl_line,dcl[D].ON_break_command);
        (void)dcl_process_command_line(1);
    }

    return(0);
}


char *dcl_get_token(char *str,char *token)
{
    char *ch1;
    char *ch2;
    int  quoting = 0;
    int  len = 0;

    *token = 0;
    ch1 = str;
    ch2 = token;
    while (*ch1 && *ch1 == ' ') ch1++;
    if (!*ch1) return(ch1);
    do
        {
        if (*ch1 == '"') {
            if (quoting) {
                if (*(ch1+1)=='"') {*ch2++ = *ch1; len++;}
                quoting = 0;
                }
            else
                quoting = 1;
            }
        else
            {*ch2++ = *ch1; len++;}
        *ch2 = 0;
        ch1++;
        }
    while ((*ch1 && *ch1 != ' ' && *ch1 != SWITCHAR && *ch1 != '!' && *ch1 != '\t'
                 && *ch1 != '=' && len < MAX_TOKEN)
       ||  (*ch1 && quoting && len < MAX_TOKEN));
    if (*ch1 == '=' && *(ch1-1) == ':') {
        ch1--;
        *(ch2-1) = 0;
    }
    return(ch1);
}

char *dcl_get_token_raw(char *str,char *token)
{
    char *ch1;
    char *ch2;
    int  quoting = 0;
    int  len = 0;

    *token = 0;
    ch1 = str;
    ch2 = token;
    while (*ch1 && *ch1 == ' ') ch1++;
    if (!*ch1) return(ch1);
    do
        {
        if (*ch1 == '"') {
            if (quoting) {
                if (*(ch1+1)=='"') {*ch2++ = *ch1; len++;}
                quoting = 0;
                }
            else
                quoting = 1;
            }
        else
            {*ch2++ = *ch1; len++;}
        *ch2 = 0;
        ch1++;
        }
    while ((*ch1 && *ch1 != ' ' && *ch1 != SWITCHAR && *ch1 != '!' && *ch1 != '\t'
                 && len < MAX_TOKEN)
       ||  (*ch1 && quoting && len < MAX_TOKEN));
    return(ch1);
}

void dcl_end(void)
{
    int     i;
    struct  tm  *time_s;
    long    time_l;
    unsigned long  elapsed,hour,min,sec;

    if (mode == MODE_INTERACTIVE) {
        strcpy(INI_NAME,"INI$USERNAME");
        (void)logical_get(INI_NAME,INI_VALUE);
        time_l = time(NULL);
        time_s = localtime(&time_l);
        (void)dcl_printf(dcl[D].SYS_OUTPUT,
                "\n%s logged out at %2.2d-%s-%4.4d %2.2d:%2.2d:%2.2d.00\n",
                INI_VALUE,
                time_s->tm_mday,
                MONTH[time_s->tm_mon+1],
                time_s->tm_year+1900,
                time_s->tm_hour,
                time_s->tm_min,
                time_s->tm_sec);
        elapsed = (unsigned long) difftime(time_l,start_time);
        hour = elapsed / 3600L; elapsed = elapsed % 3600L;
        min  = elapsed / 60L;
        sec  = elapsed % 60L;
        (void)dcl_printf(dcl[D].SYS_OUTPUT,"Elapsed time : ");
        if (hour == 1)
            (void)dcl_printf(dcl[D].SYS_OUTPUT,"%lu hour, ",hour);
        if (hour > 1)
            (void)dcl_printf(dcl[D].SYS_OUTPUT,"%lu hours, ",hour);
        if (min || hour)
            (void)dcl_printf(dcl[D].SYS_OUTPUT,"%lu min, ",min);
        (void)dcl_printf(dcl[D].SYS_OUTPUT,"%lu sec.\n",sec);
    }

    while (D) (void)dcl_exit_proc_level();
    (void)dcl_exit_proc_level();
    while (C) (void)dcl_exit_cmd_level();
    (void)dcl_exit_cmd_level();
//    (void)rmFlist(&logical_list);
    for(i = 0; i < LOGICAL_MAX; i++) {
        (void)memset(&LOGICAL_TABLE[i], 0, sizeof(LOGICAL));
    }
    {
        FILE    *fp = fopen(history_filename, "w");
        char    buffer[1024];
        if (fp != NULL) {
            (void)mkcdptr(cmd_stack,0); //cmd_stack->nodes
            while(prevdptr(cmd_stack,buffer)) {
                fprintf(fp, "%s\n", buffer);
            }
            (void)fclose(fp);
        }
        (void)rmFlist(&cmd_stack);
    }
    (void)rmFlist(&data_stack);
    for (i = 0; i < 24; i++) {
        if (keydef[i].value)
            free(keydef[i].value);
        }
    tio_close();
}

int dcl_search(char **table,char *key)
{
    size_t  l = strlen(key);
    int i = 0;
    int elem = 0;
    int count = 0;

    while (table[i]) {
        if (strncasecmp(table[i],key,l) == 0) {
            elem = i;
            count++;
            }
        i++;
        }
    if (count > 1)  return(-2);
    if (count == 0) return(-1);
    return(elem);
}

int dcl_search_key(char **table,char *key)
{
    size_t l = strlen(key);
    int i = 0;

    while (table[i]) {
        if (strncasecmp(table[i],key,l) == 0) {
            return(i);
            }
        i++;
        }
    return(-1);
}

void dcl_get_param(char *str,char *param)
{
    char    *s = str;
    char    *p = param;

    *p = 0;
    while (*s && *s != '=') s++;
    if (*s == '=') {
        s++;
        while (*s && *s == ' ') s++;
        if (*s == '"') {
            s++;
            while (*s) {
                if (*s == '"') {
                    if (*(s+1) != '"')
                        break;
                    else
                        s++;
                    }
                else {
                    *p++ = *s++;
                    *p = 0;
                    }
                }
            }
        else {
            while (*s && *s != ' ') {
                *p++ = *s++;
                *p = 0;
                }
            }
        }
}

int dcl_command(void)
{
    char    token[MAX_TOKEN];
    char    work[MAX_TOKEN];
    char    temp[MAX_TOKEN];
    char    *ch   = dcl_line;
    int     i;
    PARAM_T *p,*q;

    HARDERR = 0;
    *work = 0;

    ch = dcl_get_token(ch,token);

    if (!*token) {
        NEXT_LINE();
        return(0);
        }

    if (*token != '@') {
        (void)symbol_get(token,work);
        if (*work) {
            strcpy(temp,work);
            strcat(temp,ch);
            strcpy(dcl_line,temp);
            ch = dcl_line;
            (void)dcl_get_token(ch,token);
            }
        }

    DebugLogV(DEBUGLOG_LEVEL_DEBUG, "<dcl_command> %s.", dcl_line);

    if (*token == '@') {
        (void)dcl_file();
        }
    else {
        i = cmd_search_table(token);
        switch (i) {
            case NOT_FOUND:
                 (void)logical_get("INI$DOS",INI_VALUE);
                 if (INI_VALUE[0] == 'Y')
                    (void)dcl_spawn(dcl_line,0);
                 else {
                    (void) logical_get("INI$STRICT",INI_VALUE);
                    if (INI_VALUE[0] == 'Y') {
                        (void)dcl_printf(dcl[D].SYS_OUTPUT,"Unknown command verb.\n");
                        _SEVERITY = 2;
                        _STATUS = 19;
                     }
                 }
                 NEXT_LINE();
                 break;
            case AMBIGUOUS:
                 (void)logical_get("INI$DOS",INI_VALUE);
                 if (INI_VALUE[0] == 'Y')
                     (void)dcl_spawn(dcl_line,0);
                 else {
                     (void)dcl_printf(dcl[D].SYS_OUTPUT,"Ambiguous command verb.\n");
                     _SEVERITY = 2;
                     _STATUS = 19;
                 }
                 NEXT_LINE();
                 break;
            default:
                 if (cmd_prepro(command[i].p,command[i].q,&p,&q) == DCL_OK) {
                    (void)command[i].fn(p,q);
                    free(p); free(q);
                    }
                 else
                    NEXT_LINE();
                 break;
            }
        }
    return(0);
}

int dcl_file(void)
{
    char    *ch;
    char    *args[2]    = {"/OUTPUT",NULL};
    char    token[MAX_TOKEN];
    char    filename[MAX_TOKEN];
    char    outfile[MAX_TOKEN];
    char    *p[8];
    int     i = 0;
    int     j = 0;


    *token = 0; *filename = 0; *outfile = 0;

    for (i=0;i<8;i++) {
        p[i] = (char *) malloc(MAX_TOKEN);
        if (p[i] == NULL) {
            while (i) free(p[i--]);
            (void)dcl_printf(dcl[D].SYS_OUTPUT,"ERROR - MA0012\n");
            _SEVERITY = 4;
            return(-1);
            }
        *p[i] = 0;
        }

    i = 0;
    ch = dcl_line;
    ch = dcl_get_token(ch,token);
    if (strlen(token)==1) {
        ch = dcl_get_token(ch,token);
        strncpy(filename,token,MAX_TOKEN);
        }
    else
        strncpy(filename,&token[1],MAX_TOKEN);

    while (*ch) {
        ch = dcl_get_token(ch,token);
        if (i == 0 && *token == SWITCHAR) {
            if (dcl_search(args,token)==0) {
                dcl_get_param(ch,outfile);
                ch = dcl_get_token(ch,token); /* skip filename token */
                if (*outfile == 0) {
                    (void)dcl_printf(dcl[D].SYS_OUTPUT,"Invalid argument\n");
                    _SEVERITY = 2;
                    for (i = 0; i < 8; free(p[i++])) ;
                    return(-1);
                }
            }
            else {
                (void)dcl_printf(dcl[D].SYS_OUTPUT,"Invalid argument\n");
                _SEVERITY = 2;
                for (i = 0; i < 8; free(p[i++])) ;
                return(-1);
            }
        }
        else {
            i++;
            if (i <= 8) {
                strcpy(p[i-1],token);
            }
            else {
                (void)dcl_printf(dcl[D].SYS_OUTPUT,"Too much arguments\n");
                _SEVERITY = 2;
                for (i = 0; i < 8; free(p[i++])) ;
                return(-1);
            }
        }
    }

    NEXT_LINE();
    i = dcl_enter_file(filename,outfile,p);
    for (j = 0; j < 8; free(p[j++])) ;
    return(i);
}

int dcl_enter_file(char *vmsfile,char *outfile,char **p)
{
    FILE  *fp;
    int   i;
    char  dosfile[_MAX_PATH];
    char  drive[_MAX_DRIVE];
    char  dir[_MAX_DIR];
    char  fname[_MAX_FNAME];
    char  ext[_MAX_EXT];
    char  fn[_MAX_PATH];

    *dosfile = 0; *drive = 0; *dir = 0; *fname = 0; *ext = 0;
    cvfs_vms_to_dos(vmsfile,dosfile,&i);
    _splitpath(dosfile,drive,dir,fname,ext);

    if (strlen(ext) == 0) {
        strcpy(INI_NAME,"INI$EXT");
        (void)logical_get(INI_NAME,INI_VALUE);
        strcat(dosfile,".");
        strcat(dosfile,INI_VALUE);
        }
    strcpy(fn, dosfile);
    fp = fopen(fn,"rt");
    if (fp) {
        (void)dcl_enter_proc_level(outfile,p);
        strcpy(dcl[D].filename,vmsfile);
        strcpy(dcl[D].outname,outfile);
        (void)dcl_enter_cmd_level();
        if (dcl_load_file(fp) != -1)
            cmd[C].ln = 1;
        }
    else {
        if (dcl_state == DCL_LOGIN) {
            dcl_state = DCL_INIT_HOME;
            }
        else if (dcl_state == DCL_LOGIN_HOME) {
            dcl_state = DCL_ARG;
            }
        else if (cmd[C].stop == 2) {
            dcl_state = DCL_EXIT;
            }
            else {
                (void)dcl_printf(dcl[D].SYS_OUTPUT,"Cannot open file %s\n",vmsfile);
                _SEVERITY = 2;
                }
        return(-1);
        }
    return(0);
}


int dcl_load_file(FILE *fp)
{
    int     i = 0;
    char    buffer[MAX_TOKEN];
    char    label[MAX_TOKEN];
    CMD_LABEL  LABEL;
    char    *b;

    *buffer = 0; *label = 0;
    while (fgets(buffer,256,fp)) {
        while(*buffer && (buffer[strlen(buffer)-1] == '\n' || buffer[strlen(buffer)-1] == '\r')) {
            buffer[strlen(buffer)-1] = 0;
        }
        if (insdptr(cmd[C].cmd,&buffer[0]) == NULL) {
            (void)dcl_printf(dcl[D].SYS_OUTPUT,"ERROR - MA0010\n");
            _SEVERITY = 4;
            return(-1);
            }
        i++;
        b = buffer;
        while (*b && (*b == ' ' || *b == '\t' || *b == '$' )) b++;
        b = dcl_get_token(b,label);
        if (*label && label[strlen(label)-1] == ':') {
            (void)strupr(label);
            strcpy(LABEL.name,label);
            LABEL.line = i;
            b = dcl_get_token(b,label);
            if (strncasecmp(label,"SUBR",4)==0)
                LABEL.subr = 1;
            else
                LABEL.subr = 0;
            if (insdptr(cmd[C].label,&LABEL) == NULL) {
                (void)dcl_printf(dcl[D].SYS_OUTPUT,"ERROR - MA0011\n");
                _SEVERITY = 4;
                return(-1);
                }
            }
        }
    fclose(fp);
    return(0);
}

int dcl_enter_proc_level(char *outfile,char **p)
{
    int   i;
    char  dosfile[_MAX_PATH];
    char  drive[_MAX_DRIVE];
    char  dir[_MAX_DIR];
    char  fname[_MAX_FNAME];
    char  ext[_MAX_EXT];

    *dosfile = 0; *drive = 0; *dir = 0; *fname = 0; *ext = 0;

    D++;
    if (*outfile) {
        cvfs_vms_to_dos(outfile,dosfile,&i);
        _splitpath(dosfile,drive,dir,fname,ext);
        if (strlen(ext) == 0) strcat(dosfile,".lis");
        dcl[D].SYS_OUTPUT = fopen(dosfile,"at");
        if (dcl[D].SYS_OUTPUT == NULL) {
            dcl[D].SYS_OUTPUT = stdout;
            }
        else {
            strncpy(dcl[D].outname,dosfile,_MAX_PATH);
            }
        }
    else
        dcl[D].SYS_OUTPUT = dcl[D-1].SYS_OUTPUT;

    dcl[D].ccl = 0;
    dcl[D].cc[dcl[D].ccl] = 1;

    dcl[D].symbol_scope = dcl[D-1].symbol_scope;
    dcl[D].symbol_list = mkFlist(sizeof(SYMBOL),0);
    if (dcl[D].symbol_list == 0) {
        _SEVERITY = 4;
        (void)dcl_printf(dcl[D].SYS_OUTPUT,"ERROR - MA0208\n");
        return(-1);
        }
    for (i = 0;i < 8; i++){
        sprintf(dosfile,"P%d",i+1);
        (void)symbol_put(dosfile,p[i],D);
        }

    dcl[D].ON = 1;
    dcl[D].ON_severity = _ERROR;
    strcpy(dcl[D].ON_error_command,"EXIT");

    return(0);
}

int dcl_enter_cmd_level(void)
{
    C++;
    cmd[C].stop = 0;

    cmd[C].cmd = mkFlist(256,0);
    if (cmd[C].cmd == 0) {
        (void)dcl_printf(dcl[D].SYS_OUTPUT,"ERROR - MA0205\n");
        _SEVERITY = 4;
        return(-1);
        }

    cmd[C].label = mkFlist(sizeof(CMD_LABEL),0);
    if (cmd[C].label == 0) {
        (void)dcl_printf(dcl[D].SYS_OUTPUT,"ERROR - MA0206\n");
        _SEVERITY = 4;
        return(-1);
        }

    cmd[C].stack = mkFlist(sizeof(int),0);
    if (cmd[C].stack == 0) {
        (void)dcl_printf(dcl[D].SYS_OUTPUT,"ERROR - MA0207\n");
        _SEVERITY = 4;
        return(-1);
        }

    cmd[C].l_stack = mkFlist(sizeof(int),0);
    if (cmd[C].l_stack == 0) {
        (void)dcl_printf(dcl[D].SYS_OUTPUT,"ERROR - MA0207\n");
        _SEVERITY = 4;
        return(-1);
        }

    return(0);
}

int dcl_exit_proc_level(void)
{
    if (dcl[D].SYS_OUTPUT != stdout) {
        fclose(dcl[D].SYS_OUTPUT);
        }
    (void)rmFlist(&dcl[D].symbol_list);
    memset(&dcl[D],0,sizeof(dcl[D]));
    D--;
    return(0);
}

int dcl_exit_cmd_level(void)
{
    (void)rmFlist(&cmd[C].cmd);
    (void)rmFlist(&cmd[C].label);
    (void)rmFlist(&cmd[C].stack);
    (void)rmFlist(&cmd[C].l_stack);
    memset(&cmd[C],0,sizeof(cmd[C]));
    C--;
    return(0);
}

int dcl_exit_file(void)
{
    if (C > 1) {
        if (dcl_state == DCL_LOGIN) dcl_state = DCL_INIT_HOME;
        if (dcl_state == DCL_LOGIN_HOME) dcl_state = DCL_ARG;
        (void)dcl_exit_proc_level();
        (void)dcl_exit_cmd_level();
        if (cmd[C].stop == 1) {
            dcl_state = DCL_LOGOUT_HOME;
            }
        if (cmd[C].stop == 2) {
            dcl_state = DCL_LOGOUT;
            }
        if (cmd[C].stop == 3) {
            dcl_state = DCL_EXIT;
            }
        }
    else
        NEXT_LINE();
    return(0);
}


int dcl_exit_subroutine(void)
{
    int i;

    if (D > 1)
        (void)dcl_exit_proc_level();

    if (popd(cmd[C].stack,&i))
        cmd[C].ln = i;
    else
        NEXT_LINE();
    dcl_subr--;
    return(0);
}

int dcl_exit(PARAM_T *p,PARAM_T *q)
{
    char    work[MAX_TOKEN];
    int     retcod = DCL_OK;

    NEXT_LINE();
    if (cmd[C].subr) return(DCL_OK);

    retcod = cmd_parse_line(dcl_line,RETURN_PARAM,DUMMY_QUAL,p,q);
    if ((retcod == DCL_OK) && (p[0].flag & PRESENT)) {
        (void)EXP_compute(p[0].value,work);
        _STATUS = atoi(work);
        }

    if (dcl_subr){
        (void)dcl_exit_subroutine();
        }
    else {
        (void)dcl_exit_file();
        }
    return(0);
}

void dcl_string(char *input, char *output, int len)
{
    char *si = input;
    char *so = output;
    int  quoting = 0;
    int  l = 0;

    if (strcmp(si,"\"\"") == 0) {
        *so = 0;
        return;
        }

    while(*si && l < len) {
        if (*si == '"') {
            if (*(si+1) == '"') {*so++ = *si; l++;}
            quoting = quoting ? 0 : 1;
            }
        else{
            if (quoting)
                {*so++ = *si; l++;}
            else
#ifdef _WIN32
                {*so++ = (char)toupper(*si); l++;}
#else
                {*so++ = *si; l++;}
#endif
            }
        si++;
        }
    *so = 0;
}


int dcl_logout(PARAM_T *p, PARAM_T *q)
{
    UNREFERENCED_PARAM(p);
    UNREFERENCED_PARAM(q);
    cmd[C].stop = 1;
    NEXT_LINE();
    dcl_state = DCL_LOGOUT;
    return(0);
}

void dcl_translate_symbol(void)
{
    char    work[1024];
    char    name[SYMBOL_MAX_NAME];
    char    value[SYMBOL_MAX_VALUE];
    char    *ch = dcl_line;
    char    *n, *v;
    char    *w = work;
    int     quoting = 0;
    int     commenting = 0;


    *work = 0; *name = 0; *value = 0;
    while (*ch){
        switch (*ch) {
        case '"' :  quoting = quoting ? 0 : 1;
                    *w++ = *ch++;
                    *w = 0;
                    break;
        case '!' :  if (!quoting) {
                        commenting = 1;
                    }
                    *w++ = *ch++;
                    *w = 0;
                    break;
        case '\'':  if (!commenting) {
                        if (quoting) {
                            if (*(ch+1) == '\'') {
                                ch +=2;
                                n = name;
                                v = value;
                                while (*ch && *ch != '\'') {
                                    *n++ = *ch++;
                                    *n = 0;
                                    }
                                ch++;
                                (void)symbol_get(name,value);
                                while (*v){
                                    *w++ = *v++;
                                    *w = 0;
                                    }
                                }
                            else{
                                *w++ = *ch++;
                                *w = 0;
                                }
                            }
                        else{
                            ch++;
                            n = name;
                            v = value;
                            while (*ch && *ch != '\'') {
                                *n++ = *ch++;
                                *n = 0;
                                }
                            ch++;
                            (void)symbol_get(name,value);
                            while (*v){
                                *w++ = *v++;
                                *w = 0;
                                }
                            }
                        }
                    else
                        {
                        *w++ = *ch++;
                        *w = 0;
                        }
                    break;
        default :   *w++ = *ch++;
                    *w = 0;
        }
    }
    strncpy(dcl_line,work,1024);
    return;
}

void dcl_translate_lexicals(void)
{
    char    temp[MAX_TOKEN];
    char    name[MAX_TOKEN];
    char    value[MAX_TOKEN];
    char    *work;
    int     i      = 0;
    int     si     = 0;
    int     sj     = 0;


    *temp = 0; *name = 0; *value = 0;
    work = dcl_line;
    i = (int)strlen(work) - 3;
    while (i >= 0) {
        if (strncasecmp(&work[i],"F$",2)==0){
            si = i;
            sj = si + next_parenthesis(&work[i]);
            strncpy(name,&work[si],(unsigned)((sj - si) + 1));
            name[(sj-si)+1] = 0;
            (void)f_lexical(name,value);
            strcpy(temp,&work[sj+1]);
            strcpy(&work[si],value);
            strcat(work,temp);
            }
        i--;
        }
    return;
}

int next_parenthesis(char *str)
{
    int i = 0;
    int l = 1;

    while (str[i] && str[i] != '(') i++;
    if (str[i]) i++;
    while (str[i]){
        if (str[i] == '(') l++;
        if (str[i] == ')') l--;
        if (l == 0) return(i);
        i++;
        }
    return(0);
}


int dcl_continue(PARAM_T *p,PARAM_T *q)
{
    UNREFERENCED_PARAM(p);
    UNREFERENCED_PARAM(q);
    NEXT_LINE();
    return(0);
}

int dcl_spawn(char *szCmd, int nowait)
{
    int     retcod = DCL_OK;
    char    file[1024];
    char    param[1024];

    memset(file, 0, sizeof(file));
    memset(param, 0, sizeof(param));

    _STATUS = 0;

    if (*szCmd == '-') {
        *szCmd = ' ';
        nowait = 1;
    }

    (void)dcl_batman(szCmd);
#ifdef _WIN32
    if (nowait) {
        HANDLE  handle;
        (void)dcl_robin(szCmd, file, param);
        handle = ShellExecute(NULL,"open",file,param,NULL,SW_SHOWNORMAL);
        if ((long)handle <= 32) {
            _STATUS = (int)handle;
        }
    }
    else {
#endif
        (void)dcl_robin(szCmd, file, param);
        if (strcasecmp(file, "cd") == 0)  /* Cheat */
            (void)dcl_change_dir(param);
        else {
            FILE *fp = NULL;
            char line[MAX_TOKEN];
            int  i   = 0;

            if ((fp = popen(szCmd, "r")) != NULL) {
                while(fgets(line, sizeof(line), fp))  {
                    (void)dcl_printf(dcl[D].SYS_OUTPUT,"%s", line);
                    i++;
                }
                pclose(fp);
                if (i == 0) {
                    (void)dcl_printf(dcl[D].SYS_OUTPUT,"\r\n");
                }
            }
            else {
                _STATUS = errno;
            }
//            _STATUS = system(szCmd);
//            (void)dcl_printf(dcl[D].SYS_OUTPUT,"\r\n");
        }
#ifdef _WIN32
    }
#endif

    if (_STATUS != 0) {
        _SEVERITY = 2;
        retcod = DCL_ERROR;
    }

    dcl_interprete_prompt(dcl_org_prompt,dcl_prompt);

    return(retcod);

}

int dcl_confirm(char *msg)
{
    char answer[9];

    while (1){ /*lint !e716 */
        (void)dcl_printf(dcl[D].SYS_OUTPUT,"%s ? [N]: ",msg);
        *answer = 0;
        (void)kbdread(answer,8,data_stack,INFINITE_TIMEOUT);
        if (!*answer) strcpy(answer,"N");
        if (toupper(*answer)=='Y' || toupper(*answer)=='T' || toupper(*answer)=='1') {
            return(CONFIRM_YES);
        }
        else if (strncasecmp(answer,"ALL",3)==0) {
            return(CONFIRM_ALL);
        }
        else if (toupper(*answer)=='Q' || CTRL_Y || CTRL_Z) {
            return(CONFIRM_QUIT);
        }
        else if (toupper(*answer)=='N' || toupper(*answer)=='F' || toupper(*answer)=='0') {
            return(CONFIRM_NO);
        }
    }
}

int dcl_searchdirlst(char *szCmd,char *path_list,int subdir,
                     int fn(char *,DCL_FIND_DATA *,void *, char),
                     void *fn_param)
{
    char    path_name[_MAX_PATH];
    int     i   = 0;
    int     j   = 0;

    *path_name = 0;

    for (i = 0; path_list[i]; i++) {
        if (path_list[i] == ',') {
            (void) dcl_searchdir(szCmd, path_name, subdir, fn, fn_param);
            *path_name = 0;
            j = 0;
        }
        else {
            path_name[j++] = path_list[i];
            path_name[j] = 0;
        }
    }
    (void) dcl_searchdir(szCmd, path_name, subdir, fn, fn_param);

    return DCL_OK;
}

int dcl_searchdir(char *szCmd,char *path_name,int subdir,
                  int fn(char *,DCL_FIND_DATA *,void *, char),
                  void *fn_param)
{
    DCL_FIND_DATA ff;
    int     ok;
    char    temp[_MAX_PATH];
    char    path[_MAX_PATH];
    char    drive[_MAX_DRIVE];
    char    dir[_MAX_DIR];
    char    file[_MAX_FNAME];
    char    ext[_MAX_EXT];
    int     handle;

    if (path_name == NULL) {
        return(DCL_ERROR);
    }

    memset(temp, 0, _MAX_PATH);
    memset(path, 0, _MAX_PATH);
    memset(drive, 0, _MAX_DRIVE);
    memset(dir, 0, _MAX_DIR);
    memset(file, 0, _MAX_FNAME);
    memset(ext, 0, _MAX_EXT);
    strcpy(path,path_name);
    _splitpath(path,drive,dir,file,ext);
//    _splitpath(strupr(path),drive,dir,file,ext);
    if (path[strlen(path)-1] == SLASH_CHR)
        path[strlen(path)-1] = '\0';
    handle = Dcl_FindFirstFile(path,&ff);
    ok = handle == (int)INVALID_HANDLE_VALUE ? 0 : 1;
    while (ok && !CTRL_Y && !HARDERR){
        if (ff.dwFileAttributes & _A_SUBDIR) {
            if (ff.cFileName[0] != '.') {
                if (!subdir) {
                    if (!wildcard(path)) {
                        sprintf(temp,"%s%s",drive,dir);
                        (void)fn(temp,&ff,fn_param, 1);
                    }
                }
            }
        }
        else {
            sprintf(temp,"%s%s%s",drive,dir,ff.cFileName);
            (void)fn(temp,&ff,fn_param, 0);
        }
        ok = Dcl_FindNextFile(handle, &ff);
    }
    (void)Dcl_FindClose(handle);

    strcpy(temp,path);
    _splitpath(path,drive,dir,file,ext);
    _makepath(path,drive,dir,"*",".*");

    if (subdir) {
        SUBDIR = 1;
        handle = Dcl_FindFirstFile(path,&ff);
        ok = handle == (int)INVALID_HANDLE_VALUE ? 0 : 1;
        while (ok && !CTRL_Y){
            if (ff.dwFileAttributes & _A_SUBDIR && ff.cFileName[0] != '.') {
                _splitpath(temp,drive,dir,file,ext);
                strcat(dir,ff.cFileName);
                strcat(dir,SLASH_STR);
                _makepath(path,drive,dir,file,ext);
                (void)dcl_searchdir(szCmd,path,1,fn,fn_param);
                }
            ok = Dcl_FindNextFile(handle, &ff);
            }
        strcpy(path,temp);
        (void)Dcl_FindClose(handle);
        }

    return(1);
}

int wildcard(char *path)
{
    if (path != NULL) {
        if ((strchr(path,'*') != NULL) || (strchr(path,'?') != NULL))
            return 1;
    }
    return 0;
}

int dcl_batman(char *szCmd)
{
    char    work[1024];
    char    vms[MAX_TOKEN];
    char    dos[MAX_TOKEN];
    char    *ch;
    int     dummy;


    ch = cmd_find_token(szCmd);
    if (*ch == ':') {
        *ch = ' ';
        }
    else {
        ch = szCmd;
        *work = 0;
        while (*ch) {
            *vms = *dos = 0;
            ch = cmd_get_token(ch,vms);
            if (*vms) {
                cvfs_vms_to_dos(vms,dos,&dummy);
                strcat(work,strcat(dos," "));
                }
            }
        strcpy(szCmd,work);
        }
    return (DCL_OK);
}

int dcl_robin(char *szCmd, char *file, char *param)
{
    char    *ch;

    *file = 0;
    *param = 0;

    ch = cmd_get_token(szCmd, file);
    if (*ch)
        strcpy(param, ch);

    return (DCL_OK);
}

int dcl_cls(PARAM_T *p,PARAM_T *q)
{
    UNREFERENCED_PARAM(p);
    UNREFERENCED_PARAM(q);
    NEXT_LINE();
    if (cmd[C].subr) return(DCL_OK);
    (void)tio_clrscr();
    return (DCL_OK);
}

void dcl_init_term(void)
{
#ifdef _WIN32
    (void)SetConsoleCtrlHandler(dcl_break_handler, TRUE);
#else
    signal(SIGINT, catch_int);
    //signal(SIGBREAK, catch_brk);
#endif
    tio_init_term();
}

void dcl_reset_term(void)
{
    tio_clrscr();
}

int dcl_printf(FILE *file,const char *format,...)
{
    va_list     ap;
    char        buffer[1024];
    int         ret = 0;
    int            i   = 0;

    va_start(ap,format);
    if (file == stdout || file == stderr) {
        memset(buffer, 0, sizeof(buffer));
        (void)vsnprintf(buffer,1023,format,ap);
        i = (int)strlen(buffer) - 1;
        if (i > 1 && buffer[i-1] == '\r' && buffer[i] == '\n') {
            buffer[i] = 0;
            buffer[i-1] = '\n';
        }
        (void)_tio_printf(buffer);
    }
    else {
        ret = vfprintf(file,format,ap);
        (void)fflush(file);
    }
    va_end(ap);
    return(ret);
}

void dcl_interprete_prompt(char *prompt,char *new_prompt)
{
    char buffer[256];
    char dpath[_MAX_PATH];
    char path[_MAX_PATH];
    int  i,j,k;

    j = 0;
    for(i = 0; i < 32 && prompt[i]; i++) {
        if (prompt[i] == '$') {
            switch (toupper(prompt[i+1])) {
                case 'P':
                    if (getcwd(dpath,_MAX_PATH) != NULL) {
                        if (dpath[strlen(dpath)-1] != SLASH_CHR)
                            strcat(dpath,SLASH_STR);
                        cvfs_dos_to_vms(dpath,path);
                        if ((int) strlen(path) > 32 - j) {
                            k = (int)(strlen(path) - 32) - j;
                            }
                        for(k = 0;k < (int) strlen(path); buffer[j++] = path[k++]) ;
                    }
                    i++;
                    break;
                case 'G':
                    i++;
                    buffer[j++] = '>';
                    break;
                default:
                    buffer[j++] = prompt[i];
                }
            }
        else {
            buffer[j++] = prompt[i];
            }
        }
    buffer[j] = 0;
//    for (i = 0; i < 32 && buffer[i]; i++)
//        new_prompt[i] = buffer[i];
//    new_prompt[i] = 0;
    if (strlen(buffer) > 32) {
        char *ptr = buffer;
        ptr = ptr + strlen(buffer);
        ptr = ptr - 32;
        (void)sprintf(new_prompt, "%32.32s", ptr);
    }
    else {
        (void)sprintf(new_prompt, "%s", buffer);
    }
#ifdef _WIN32
    buffer[0] = (char)(_getdrive() + 64);
    buffer[1] = ':';
    buffer[2] = 0;
    (void)logical_put("SYS$DISK",buffer,LOG_SYSTEM);
#endif
}


int xstricmp(const char *pat, const char *str)
{
    switch (*pat) {
    case '\0':
        return !*str;
    case '*' :
        return xstricmp(pat+1, str) || (*str && xstricmp(pat, str+1));
    case '?' :
        return *str && xstricmp(pat+1, str+1);
    default  :
        return (toupper(*pat) == toupper(*str)) && xstricmp(pat+1, str+1);
    }
}

void dcl_tostring(unsigned long long ull, char *str, size_t size)
{
    int     p = (int)size - 1;
    if (str != NULL && size > 1) {
        memset(str, ' ', size-1);
        str[p--] = 0;
        while (ull / 10 && p >= 0) {
            str[p--] = (ull % 10) + 0x30;
            ull = ull / 10;
        }
        if (p >= 0) {
            str[p] = (ull % 10) + 0x30;
        }
    }
}

void dcl_tostring_left(unsigned long long ull, char *str, size_t size)
{
    int     p1 = (int)size - 1;
    int     p2 = 0;

    if (str != NULL && size > 1) {
        memset(str, ' ', size-1);
        str[p1--] = 0;
        while (ull / 10 && p1 >= 0) {
            str[p1--] = (ull % 10) + 0x30;
            ull = ull / 10;
        }
        if (p1 >= 0) {
            str[p1] = (ull % 10) + 0x30;
        }
    }
    for(p1 = 0; str[p1] && str[p1] == ' '; p1++);
    for(; str[p1]; p1++) {
        str[p2] = str[p1];
        p2++;
    }
    for(; p2 < size-1; p2++) {
        str[p2] = 0;
    }
}

