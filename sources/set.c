/****************************************************************************/
/*                                                                          */
/*   PC-DCL -  DCL command line interpreter for Windows.                    */
/*   Copyright (C) 1991-2007 by Michel Valentin                             */
/*                                                                          */
/*   This program is free software; you can redistribute it and/or modify   */
/*   it under the terms of the enclosed   license.                          */
/*                                                                          */
/*   This program is distributed in the hope that it will be useful,        */
/*   but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                   */
/*                                                                          */
/****************************************************************************/
/*lint -e818 * could be declared as constant */
/*lint -e749 * local enumeration constant not used */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/time.h>
#endif

#include "platform.h"
#include "dcl.h"
#include "dcltime.h"
#include "step.h"
#include "termio.h"

extern char *cmd_get_token(char *str,char *token);
extern char *cmd_find_token(char *str);

// Set -----------------------------------------------------
RPARAM_T SET_PARAM[] = {
    {1,"What",MANDATORY},
    {2,"To",0/*EXPR*/},
    {3,"",0/*EXPR*/},
    {0,"",0}
    };
RPARAM_T SET_QUAL[] = {
    {1,"/SCOPE",VALUE},
    {2,"/ATTRIBUTE",VALUE},
    {3,"/BEFORE",VALUE},
    {4,"/CONFIRM",VALUE},
    {5,"/NOCONFIRM",VALUE},
    {6,"/DATE",VALUE},
    {7,"/LOG",0},
    {8,"/NOLOG",0},
    {9,"/SINCE",VALUE},
    {10,"/SUBDIR",0},
    {11,"/NOSUBDIR",0},
    {12,"/SIZE",VALUE},
    {13,"/COLOR",VALUE},
    {14,"/OVERSTRIKE",0},
    {15,"/INSERT",0},
    {16,"/CLEAR",0},
    {17,"/NOCLEAR",0},
    {0,"",0}
    };

int dcl_set_default(PARAM_T *p, PARAM_T *q);
int dcl_set_dosvar(PARAM_T *p, PARAM_T *q);
int dcl_set_on(PARAM_T *p, PARAM_T *q);
int dcl_set_noon(PARAM_T *p, PARAM_T *q);
int dcl_set_prompt(PARAM_T *p, PARAM_T *q);
int dcl_set_symbol(PARAM_T *p, PARAM_T *q);
int dcl_set_time(PARAM_T *p, PARAM_T *q);
int dcl_set_verify(PARAM_T *p, PARAM_T *q);
int dcl_set_noverify(PARAM_T *p, PARAM_T *q);
int dcl_set_step(PARAM_T *p, PARAM_T *q);
int dcl_set_nostep(PARAM_T *p, PARAM_T *q);
int dcl_set_term(PARAM_T *p, PARAM_T *q);
int dcl_set_control(PARAM_T *p, PARAM_T *q,int value);

int dcl_set(PARAM_T *p, PARAM_T *q)
{
    char    *option[] = {"DEFAULT","FILE","ON","NOON","PROMPT",
                         "SYMBOL","TIME","VERIFY","NOVERIFY","STEP","NOSTEP",
                         "TERMINAL","CONTROL","NOCONTROL","DOSVAR",NULL};

    enum    option_tag {OPT_DEFAULT,OPT_FILE,OPT_ON,OPT_NOON,
                        OPT_PROMPT,OPT_SYMBOL,OPT_TIME,OPT_VERIFY,
                        OPT_NOVERIFY,OPT_STEP,OPT_NOSTEP,OPT_TERMINAL,
                        OPT_CONTROL,OPT_NOCONTROL,OPT_DOSVAR};


    char    token[MAX_TOKEN];

    NEXT_LINE();
    if (cmd[C].subr) return(0);
    if (!dcl[D].cc[dcl[D].ccl]) return(0);
    if (p == NULL || q == NULL) return(DCL_ERROR);

    _STATUS = 0;
    (void) cmd_parse_line(dcl_line,SET_PARAM,SET_QUAL,p,q);
    dcl_string(p[0].value,token,MAX_TOKEN);
    switch (dcl_search(option,token)){
        case OPT_DEFAULT    :   (void) dcl_set_default(p,q);
                                break;
        case OPT_FILE       :   (void) dcl_set_file(p,q);
                                break;
        case OPT_ON         :   (void) dcl_set_on(p,q);
                                break;
        case OPT_NOON       :   (void) dcl_set_noon(p,q);
                                break;
        case OPT_PROMPT     :   (void) dcl_set_prompt(p,q);
                                break;
        case OPT_SYMBOL     :   (void) dcl_set_symbol(p,q);
                                break;
        case OPT_TIME       :   (void) dcl_set_time(p,q);
                                break;
        case OPT_VERIFY     :   (void) dcl_set_verify(p,q);
                                break;
        case OPT_NOVERIFY   :   (void) dcl_set_noverify(p,q);
                                break;
        case OPT_STEP       :   (void) dcl_set_step(p,q);
                                break;
        case OPT_NOSTEP     :   (void) dcl_set_nostep(p,q);
                                break;
        case OPT_TERMINAL   :   (void) dcl_set_term(p,q);
                                break;
        case OPT_CONTROL    :   (void) dcl_set_control(p,q,TRUE);
                                break;
        case OPT_NOCONTROL  :   (void) dcl_set_control(p,q,FALSE);
                                break;
        case OPT_DOSVAR     :   (void) dcl_set_dosvar(p,q);
                                break;
        case NOT_FOUND:         (void) logical_get("INI$STRICT",INI_VALUE);
                                if (INI_VALUE[0] == 'Y') {
                                    (void) dcl_printf(dcl[D].SYS_OUTPUT,"Invalid SET option.\n");
                                    _SEVERITY = 2;
                                    _STATUS = 19;
                                }
                                break;
        case AMBIGUOUS:         (void) dcl_printf(dcl[D].SYS_OUTPUT,"Ambiguous SET option.\n");
                                _SEVERITY = 2;
                                _STATUS = 19;
                                break;
        default:                ;
        }
    return(0);
}

//---------------------------------------------------------------------------
void dcl_trim(char *instr, char *outstr)
{
    int     i   = 0;
    int     j   = 0;
    char    *ch = outstr;

    while(instr[i] && (instr[i] == ' ' || instr[i] == '\t')) i++;
    j = (int)strlen(instr) - 1;
    while (j && (instr[j] == ' ' || instr[j] == '\t')) j--;
    while(i <= j) *ch++ = instr[i++];
    *ch = 0;
}

int dcl_change_dir(char *vms)
{
    char    dos[MAX_TOKEN];
    char    wrk[MAX_TOKEN];
    char    token[MAX_TOKEN];
    int     recurse;

    if (vms == NULL) return(DCL_ERROR);

    *dos = 0; *wrk = 0; *token = 0;

    dcl_trim(vms, wrk);
    cvfs_vms_to_dos(wrk,dos,&recurse);
    /*(void) strupr(dos);***************************/
    if (dos[strlen(dos)-1]==SLASH_CHR
        && strlen(dos) > 1
        && dos[strlen(dos)-2]!=':')
        dos[strlen(dos)-1] = 0;
#ifdef _WIN32
    if (dos[1] == ':'){
        (void) _chdrive(dos[0] - '@');
        strcpy(vms,"SYS$DISK");
        token[0] = dos[0];
        token[1] = ':';
        token[2] = 0;
        (void) logical_put(vms,token,LOG_SYSTEM);
        if (strlen(&dos[2])) {
            if (_chdir(dos)) {
                (void) dcl_printf(dcl[D].SYS_OUTPUT,"%s: %s\n",vms,strerror(errno));
                }
            }
        }
    else
#endif
        if (chdir(dos)) {
            (void) dcl_printf(dcl[D].SYS_OUTPUT,"%s: %s\n",vms,strerror(errno));
        }
    return(0);
}

int dcl_set_default(PARAM_T *p, PARAM_T *q)
{
    char    vms[MAX_TOKEN];
    int     retcod = DCL_OK;

    UNREFERENCED_PARAM(q);

    if (p == NULL) return(DCL_ERROR);

    *vms = 0;
    dcl_string(p[1].value,vms,MAX_TOKEN);
    retcod = dcl_change_dir(vms);
    dcl_interprete_prompt(dcl_org_prompt,dcl_prompt);

    return(retcod);
}

//---------------------------------------------------------------------------
int dcl_set_on(PARAM_T *p, PARAM_T *q)
{
    UNREFERENCED_PARAM(p);
    UNREFERENCED_PARAM(q);

    dcl[D].ON = 1;
    return(0);
}
//---------------------------------------------------------------------------
int dcl_set_noon(PARAM_T *p, PARAM_T *q)
{
    UNREFERENCED_PARAM(p);
    UNREFERENCED_PARAM(q);

    dcl[D].ON = 0;
    return(0);
}
//---------------------------------------------------------------------------
int dcl_set_prompt(PARAM_T *p, PARAM_T *q)
{
    char    param[MAX_TOKEN];
    char    work[MAX_TOKEN];
    char    *c;

    UNREFERENCED_PARAM(q);
    if (p == NULL) return(DCL_ERROR);

    *param = 0;
    c = p[1].expr;
    if (c != NULL) {
        c = cmd_find_token(c);
        if (*c == '=') {
            c = &c[1];
            }
        if (*c) {
            c = cmd_get_token(c,work);
            dcl_string(work,param,MAX_TOKEN);
            }
        }
    if (strlen(param)){
        strncpy(dcl_prompt,param,32);
        dcl_prompt[32] = 0;
        }
    else
        strcpy(dcl_prompt,"$ ");
//    strcat(dcl_prompt," ");
    strcpy(dcl_org_prompt,dcl_prompt);
    dcl_interprete_prompt(dcl_org_prompt,dcl_prompt);

    return(0);
}
//---------------------------------------------------------------------------
int dcl_set_symbol(PARAM_T *p, PARAM_T *q)
{
    char    param[MAX_TOKEN];
    int     retcod = DCL_OK;
    int     i;

    if (p == NULL) return(DCL_ERROR);
    if (q == NULL) return(DCL_ERROR);

    *param = 0;
    if (p[1].flag & PRESENT) {
        (void) dcl_printf(dcl[D].SYS_OUTPUT,"Invalid parameters number.\n");
        _SEVERITY = 2;
        _STATUS = 19;
        return(DCL_ERROR);
        }

    for (i = 0; q[i].tag && retcod == DCL_OK; i++) {
        if (q[i].flag & PRESENT)
            switch (q[i].tag) {
                case 1:                     /* /SCOPE  */
                    dcl_string(q[i].value,param,MAX_TOKEN);
                    if (strncasecmp(param,"LOCAL",5) == 0)
                        dcl[D].symbol_scope |= SYMSCO_LOCAL;
                    else if (strncasecmp(param,"NOLOCAL",7) == 0)
                            dcl[D].symbol_scope ^= SYMSCO_LOCAL;
                    else if (strncasecmp(param,"GLOBAL",6) == 0)
                            dcl[D].symbol_scope |= SYMSCO_GLOBAL;
                    else if (strncasecmp(param,"NOGLOBAL",5) == 0)
                            dcl[D].symbol_scope ^= SYMSCO_GLOBAL;
                    else {
                        (void) dcl_printf(dcl[D].SYS_OUTPUT,"Invalid keyword.\n");
                        _SEVERITY = 2;
                        _STATUS = 19;
                        retcod = DCL_ERROR;
                        }
                    break;
                default:
                    (void) dcl_printf(dcl[D].SYS_OUTPUT,"ERROR - Invalid command qualifier.\n");
                    _SEVERITY = 2;
                    _STATUS = 19;
                    retcod = DCL_ERROR;
                } /* end switch */
        }   /* end for */
    return(retcod);
}
//---------------------------------------------------------------------------
int dcl_set_time(PARAM_T *p, PARAM_T *q)
{
#ifdef _WIN32
    char            param[MAX_TOKEN];
    SYSTEMTIME      st;
    time_t          tl;
    struct tm       ts;

    UNREFERENCED_PARAM(q);
    if (p == NULL) return(DCL_ERROR);

    *param = 0;
    dcl_string(p[1].value,param,MAX_TOKEN);
    tm_str_to_long(param,&tl);
    tm_long_to_tm(&tl, &ts);
    st.wDay = ts.tm_mday;
    st.wDayOfWeek = 0;
    st.wHour = ts.tm_hour;
    st.wMilliseconds = 0;
    st.wMinute = ts.tm_min;
    st.wMonth = ts.tm_mon + 1;
    st.wSecond = ts.tm_sec;
    st.wYear = ts.tm_year + 1900;
    (void) SetLocalTime(&st);
#else
    char            param[MAX_TOKEN];
    time_t          tl;
    struct timeval     tp;
    /*struct timezone    tzp;  not used anymore ??? */

    UNREFERENCED_PARAM(q);
    if (p == NULL) return(DCL_ERROR);

    *param = 0;
    dcl_string(p[1].value,param,MAX_TOKEN);
    tm_str_to_long(param,&tl);
    tp.tv_sec = tl;
    tp.tv_usec = 0;

    settimeofday(&tp, NULL);
#endif
    return(0);
}
//---------------------------------------------------------------------------
int dcl_set_verify(PARAM_T *p, PARAM_T *q)
{
    char    param[MAX_TOKEN];

    UNREFERENCED_PARAM(q);
    if (p == NULL) return(DCL_ERROR);

    *param = 0;
    dcl_string(p[1].value,param,MAX_TOKEN);
    if (*param){
        if (strncasecmp(param,"PROCEDURE",9)==0)
            verify |= VERIFY_PROC;
        else if (strncasecmp(param,"NOPROCEDURE",11)==0)
                verify &= ~VERIFY_PROC;
        else if (strncasecmp(param,"IMAGE",5)==0)
                verify |= VERIFY_IMAGE;
        else if (strncasecmp(param,"NOIMAGE",7)==0)
                verify &= ~VERIFY_IMAGE;
        else {
            (void) dcl_printf(dcl[D].SYS_OUTPUT,"Invalid keyword.\n");
            _SEVERITY = 2;
            _STATUS = 19;
            return(-1);
             }
        }
    else
        verify = VERIFY_PROC + VERIFY_IMAGE;

    return(0);
}
//---------------------------------------------------------------------------
int dcl_set_noverify(PARAM_T *p, PARAM_T *q)
{
    char    param[MAX_TOKEN];

    UNREFERENCED_PARAM(q);
    if (p == NULL) return(DCL_ERROR);

    *param = 0;

    dcl_string(p[1].value,param,MAX_TOKEN);
    if (*param){
        if (strncasecmp(param,"PROCEDURE",9)==0)
            verify &= ~VERIFY_PROC;
        else if (strncasecmp(param,"NOPROCEDURE",11)==0)
                verify |= VERIFY_PROC;
        else if (strncasecmp(param,"IMAGE",5)==0)
                verify &= ~VERIFY_IMAGE;
        else if (strncasecmp(param,"NOIMAGE",7)==0)
                verify |= VERIFY_IMAGE;
        else {
            (void) dcl_printf(dcl[D].SYS_OUTPUT,"Invalid keyword.\n");
            _SEVERITY = 2;
            _STATUS = 19;
            return(-1);
            }
        }
    else
        verify = 0;

    return(0);
}
//---------------------------------------------------------------------------
int dcl_set_step(PARAM_T *p, PARAM_T *q)
{
    UNREFERENCED_PARAM(p);
    UNREFERENCED_PARAM(q);

    STEP = 1;
    return(DCL_OK);
}
//---------------------------------------------------------------------------
int dcl_set_nostep(PARAM_T *p, PARAM_T *q)
{
    UNREFERENCED_PARAM(p);
    UNREFERENCED_PARAM(q);

    STEP = 0;
    return(DCL_OK);
}
//---------------------------------------------------------------------------
int dcl_set_term(PARAM_T *p, PARAM_T *q)
{
    char    work[MAX_TOKEN];
    char    *w;
    char    *ch;
    int     i,j;
    int     retcod = DCL_OK;
    int     clear = 0;
    short   sizeX = terminfo.Size.X;
    short   sizeY = terminfo.Size.Y;
    int     f_resize = 0;

    UNREFERENCED_PARAM(p);
    if (q == NULL) return(DCL_ERROR);

    for (i = 0; q[i].tag && retcod == DCL_OK; i++) {
        if (q[i].flag & PRESENT)
            switch (q[i].tag) {
                case 12:                    /* /SIZE  */
                    *work = 0;
                    w = work;
                    ch = q[i].value;
                    while (*ch && (*ch == '(' || isspace((int)*ch))) ch++;
                    while (*ch && *ch != ',' && !isspace((int)*ch))
                        *w++ = *ch++;
                    *w = 0;
                    j = atoi(work);
                    if (j < 1 || j > 128) {
                        (void) dcl_printf(dcl[D].SYS_OUTPUT,"Invalid number of columns in SIZE qualifier\n");
                        _SEVERITY = 2;
                        _STATUS = 19;
                        retcod = DCL_ERROR;
                    }
                    else {
                        sizeX = (short) j;
                        f_resize = 1;
                    }
                    *work = 0;
                    w = work;
                    while (*ch && (*ch == ',' || isspace((int)*ch))) ch++;
                    while (*ch && *ch != ')' && !isspace((int)*ch))
                        *w++ = *ch++;
                    *w = 0;
                    j = atoi(work);
                    if (j < 25 || j > 1024) {
                        (void) dcl_printf(dcl[D].SYS_OUTPUT,"Invalid number of lines in SIZE qualifier\n");
                        _SEVERITY = 2;
                        _STATUS = 19;
                        retcod = DCL_ERROR;
                    }
                    else {
                        sizeY = (short) j;
                        f_resize = 1;
                    }
                    break;
                case 13:                    /* /COLOR */
                    *work = 0;
                    w = work;
                    ch = q[i].value;
                    while (*ch && (*ch == '(' || isspace((int)*ch))) ch++;
                    while (*ch && *ch != ',' && !isspace((int)*ch))
                        *w++ = *ch++;
                    *w = 0;
                    j = dcl_search(termio_color_q,work);
                    if (j < BLACK || j > WHITE) {
                        (void) dcl_printf(dcl[D].SYS_OUTPUT,"Invalid or ambiguous FOREGROUND COLOR qualifier\n");
                        _SEVERITY = 2;
                        _STATUS = 19;
                        retcod = DCL_ERROR;
                        }
                    else {
                        terminfo.fg_color = (unsigned short) j;
                        }
                    *work = 0;
                    w = work;
                    while (*ch && (*ch == ',' || isspace((int)*ch))) ch++;
                    while (*ch && *ch != ')' && !isspace((int)*ch))
                        *w++ = *ch++;
                    *w = 0;
                    j = dcl_search(termio_color_q,work);
                    if (j > 7) j -= 8;
                    if (j < BLACK || j > 7) {
                        (void) dcl_printf(dcl[D].SYS_OUTPUT,"Invalid or ambiguous BACKGROUND COLOR qualifier\n");
                        _SEVERITY = 2;
                        _STATUS = 19;
                        retcod = DCL_ERROR;
                    }
                    else {
                        terminfo.bg_color = (unsigned short)j;
                    }
//                    clear = 1;
                    break;
                case 14:                    /* /OVERSTRIKE */
                    default_insert_mode = OVERSTRIKE;
                    break;
                case 15:                    /* /INSERT */
                    default_insert_mode = INSERT;
                    break;
                case 16:                    /* /CLEAR */
                    clear = 1;
                    break;
                case 17:                    /* /NOCLEAR */
                    clear = 0;
                    break;
                default:
                    (void)dcl_printf(dcl[D].SYS_OUTPUT,"ERROR - Invalid command qualifier.\n");
                    _SEVERITY = 2;
                    _STATUS = 19;
                    retcod = DCL_ERROR;
                } /* end switch */
        }   /* end for */

    if (retcod == DCL_OK) {
        (void)tio_screen_attr();
        if (f_resize) {
            terminfo.Size.X = sizeX;
            terminfo.Size.Y = sizeY;
            (void)tio_screen_size();
            clear = 1;
        }
        if (clear) {
            (void)tio_clrscr();

        }
    }

    return(retcod);
}
//---------------------------------------------------------------------------
int dcl_set_control(PARAM_T *p, PARAM_T *q,int value)
{
    char    work[MAX_TOKEN];
    char    *c;
    int     retcod = DCL_OK;
    int     present_flag = 0;

    UNREFERENCED_PARAM(q);
    if (p == NULL) return(DCL_ERROR);

    c = p[1].expr;
    c = cmd_find_token(c);
    if (*c == '=') {
        c = &c[1];
        }
    c = cmd_get_token(c,work);
    c = work;
    while (*c && retcod == DCL_OK) {
        switch (*c) {
            case 'Y':
            case 'y':
            case 'C':
            case 'c':
                CONTROL_Y_ALLOWED = (char)value;
                present_flag = 1;
                break;
            case 'T':
            case 't':
                CONTROL_T_ALLOWED = (char)value;
                present_flag = 1;
                break;
            case '(':
            case ',':
            case ')':
                break;
            default:
                (void)dcl_printf(dcl[D].SYS_OUTPUT,"Invalid CONTROL parameter.\n");
                _SEVERITY = 2;
                _STATUS = 19;
                retcod = DCL_ERROR;
            }
        c++;
        }

    if (retcod == DCL_OK && present_flag == 0)
        CONTROL_Y_ALLOWED = (char)value;

    return(retcod);
}
//---------------------------------------------------------------------------
int dcl_set_dosvar(PARAM_T *p, PARAM_T *q)
{
    char    name[MAX_TOKEN];
    char    value[MAX_TOKEN];
    char    work[MAX_TOKEN];
    char    *ch;

    UNREFERENCED_PARAM(q);
    if (p == NULL) return(DCL_ERROR);

    ch = dcl_get_token(p[0].expr,work);
    ch = dcl_get_token(ch,work);
    dcl_string(work,name,MAX_TOKEN);
    dcl_get_param(ch,work);
    (void) dcl_get_token(work,value);
#ifdef _WIN32
    (void)sprintf(work,"%s=%s",name,value);
    (void)putenv(work);
#else
    (void)setenv(name, value, 1);
    (void)sprintf(work,"export %s=%s",name,value);
    system(work);
#endif
    return(DCL_OK);
}

