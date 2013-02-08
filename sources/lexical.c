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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#ifndef _WIN32
#include <sys/utsname.h>
#endif
#include <math.h>

#include "platform.h"
#include "dcl.h"
#include "dcltime.h"
#include "dclfindfile.h"
#include "md5.h"
#include "pcre.h"

#define CPU_REG_KEY     HKEY_LOCAL_MACHINE
#define CPU_REG_SUBKEY  "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"

#define CPU_SPEED           "~MHz"
#define CPU_IDENTIFIER      "Identifier"
#define CPU_NAME            "ProcessorNameString"
#define CPU_VENDOR          "VendorIdentifier"

extern  SEARCH  search[9];

int f_chr(char *name, char *value);
int f_cunits(char *name, char *value);
int f_cvsi(char *name, char *value);
int f_cvtime(char *name, char *value);
int f_cvui(char *name, char *value);
int f_directory(char *value);
int f_dosvar(char *name, char *value);
int f_edit(char *name, char *value);
int f_element(char *name, char *value);
int f_environment(char *name, char *value);
int f_extract(char *name, char *value);
int f_fao(char *name, char *value);
int f_file_attributes(char *name, char *value);
int f_getdvi(char *name, char *value);
int f_getenv(char *name, char *value);
int f_getjpi(char *name, char *value);
int f_getsyi(char *name, char *value);
int f_integer(char *name, char *value);
int f_length(char *name,char *value);
int f_locate(char *name, char *value);
int f_match_wild(char *name, char *value);
int f_message(char *name, char *value);
int f_mode(char *value);
int f_node(char *value);
int f_parse(char *name, char *value);
int f_privilege(char *name, char *value);
int f_process(char *value);
int f_search(char *name, char *value);
int f_setprv(char *name, char *value);
int f_string(char *name, char *value);
int f_time(char *value);
int f_trnlnm(char *name, char *value);
int f_type(char *name, char *value);
int f_unique(char *value);
int f_user(char *value);
int f_verify(char *name,char *value);

int fao(char *result,char *fmt,char **arg);
unsigned long cvui(int start,int length,const char *str);
long cvsi(int start,int length,const char *str);
#ifdef __linux
int GetMemoryStatus(MEMORYSTATUS *mi);
void GetLogicalDriveStrings(size_t maxlen, char *buffer);
float get_cpu_clock_speed();
#endif

char *dcl_get_quoted_string(char *si,char *so,size_t maxlen)
{
    int loop = 1;
    int quoting = 0;

    if (si == NULL || so == NULL) return(NULL);

    while (loop && maxlen) {
        switch (*si) {
        case '\0':
            loop = 0;
            break;
        case '\"':
            if (!quoting) {
                quoting = 1;
            }
            else
                if (*(si+1) == '\"') {
                    si++;
                }
                else {
                    loop = 0;
                }
        default: /*lint -fallthrough */
            *so++ = *si++;
            maxlen--;
        }
    }
    *so = 0;
    if (*si) si++;
    return(si);
}

int f_lexical(char *name,char *value)
{
    if (name == NULL || value == NULL) return(DCL_ERROR);

    *value = 0;
    if (strncasecmp(name,"F$CHR",5) == 0) {
        return(f_chr(name,value));
    }
    if (strncasecmp(name,"F$CUNITS",8) == 0) {
        return(f_cunits(name,value));
    }
    if (strncasecmp(name,"F$CVTIME",8) == 0) {
        return(f_cvtime(name,value));
    }
    if (strncasecmp(name,"F$CVUI",6) == 0) {
        return(f_cvui(name,value));
    }
    if (strncasecmp(name,"F$CVSI",6) == 0) {
        return(f_cvsi(name,value));
    }
    if (strncasecmp(name,"F$DIRECTORY",11) == 0) {
        return(f_directory(value));
    }
    if (strncasecmp(name,"F$GETENV",8) == 0) {
        return(f_dosvar(name,value));
    }
    if (strncasecmp(name,"F$DOSVAR",8) == 0) {
        return(f_dosvar(name,value));
    }
    if (strncasecmp(name,"F$EDIT",6) == 0) {
        return(f_edit(name,value));
    }
    if (strncasecmp(name,"F$ELEMENT",9) == 0) {
        return(f_element(name,value));
    }
    if (strncasecmp(name,"F$ENV",5) == 0) {
        return(f_environment(name,value));
    }
    if (strncasecmp(name,"F$ENVIRONMENT",13) == 0) {
        return(f_environment(name,value));
    }
    if (strncasecmp(name,"F$EXTRACT",9) == 0) {
        return(f_extract(name,value));
    }
    if (strncasecmp(name,"F$FAO",5) == 0) {
        return(f_fao(name,value));
    }
    if (strncasecmp(name,"F$FILE_ATTRIBUTES",17) == 0) {
        return(f_file_attributes(name,value));
    }
    if (strncasecmp(name,"F$GETENV",8) == 0) {
        return(f_dosvar(name,value));
    }
    if (strncasecmp(name,"F$GETDVI",8) == 0) {
        return(f_getdvi(name,value));
    }
    if (strncasecmp(name,"F$GETJPI",8) == 0) {
        return(f_getjpi(name,value));
    }
    if (strncasecmp(name,"F$GETSYI",8) == 0) {
        return(f_getsyi(name,value));
    }
    if (strncasecmp(name,"F$INTEGER",9) == 0) {
        return(f_integer(name,value));
    }
    if (strncasecmp(name,"F$LENGTH",8) == 0) {
        return(f_length(name,value));
    }
    if (strncasecmp(name,"F$LOCATE",8) == 0) {
        return(f_locate(name,value));
    }
    if (strncasecmp(name,"F$MATCH_WILD",12) == 0) {
        return(f_match_wild(name,value));
    }
    if (strncasecmp(name,"F$MESSAGE",9) == 0) {
        return(f_message(name,value));
    }
    if (strncasecmp(name,"F$MODE",6) == 0) {
        return(f_mode(value));
    }
    if (strncasecmp(name,"F$NODE",6) == 0) {
        return(f_node(value));
    }
    if (strncasecmp(name,"F$PARSE",7) == 0) {
        return(f_parse(name, value));
    }
    if (strncasecmp(name,"F$PRIVILEGE",11) == 0) {
        return(f_privilege(name, value));
    }
    if (strncasecmp(name,"F$PID",5) == 0) {
        return(f_process(value));
    }
    if (strncasecmp(name,"F$PROCESS",9) == 0) {
        return(f_process(value));
    }
    if (strncasecmp(name,"F$SEARCH",8) == 0) {
        return(f_search(name,value));
    }
    if (strncasecmp(name,"F$SETPRV",8) == 0) {
        return(f_setprv(name,value));
    }
    if (strncasecmp(name,"F$STRING",8) == 0) {
        return(f_string(name,value));
    }
    if (strncasecmp(name,"F$TIME",6) == 0) {
        return(f_time(value));
    }
    if (strncasecmp(name,"F$TRNLNM",8) == 0) {
        return(f_trnlnm(name,value));
    }
    if (strncasecmp(name,"F$TYPE",6) == 0) {
        return(f_type(name,value));
    }
    if (strncasecmp(name,"F$UNIQUE",8) == 0) {
        return(f_unique(value));
    }
    if (strncasecmp(name,"F$USER",6) == 0) {
        return(f_user(value));
    }
    if (strncasecmp(name,"F$VERIFY",8) == 0) {
        return(f_verify(name,value));
    }
    (void) logical_get("INI$STRICT",INI_VALUE);
    if (INI_VALUE[0] != 'Y') {
        sprintf(value, "\"\"");
        return(0);
    }
    return(-1);
}

//------------------------------------------------------------------------

int f_chr(char *name, char *value)
{
    char    work[MAX_TOKEN];
    char    result[MAX_TOKEN];
    char    *ch;
    char    *w;
    int     i = 0;

    if (name == NULL || value == NULL) return(DCL_ERROR);

    *work = 0; *result = 0;
    ch = name;
    w  = work;
    while (*ch && *ch != '(') ch++;
    if (*ch) ch++;
    while (*ch && *ch != ')' && *ch != ',') *w++ = *ch++;
    *w = 0;

    (void) EXP_compute(work,result);
    i = atoi(result);
    sprintf(value,"\"%c\"",i);

    return(0);
}
// ---------------------------------------------------------------------------

int f_cunits(char *name, char *value)
{
    char            token1[MAX_TOKEN];
    char            token2[MAX_TOKEN];
    char            token3[MAX_TOKEN];
    char            work[MAX_TOKEN];
    char            *w;
    char            *ch;
    unsigned long   num = 0;
    double          dnum = 0;
    char            unit[5];

    if (name == NULL || value == NULL) return(DCL_ERROR);

    *token1 = 0; *token2 = 0; *token3 = 0; *work = 0;
    ch = name;
    w = work;
    while (*ch && *ch != '(') ch++;
    if (*ch) ch++;

    while (*ch && *ch != ')' && *ch != ',') *w++ = *ch++;
    *w = 0;
    (void) EXP_compute(work,token1);
    num = atoi(token1);

    w = work;
    if (*ch) ch++;
    while (*ch && *ch != ')' && *ch != ',') *w++ = *ch++;
    *w = 0;
    (void) EXP_compute(work,token2);
    if (*token2 == 0) strcpy(token2,"BLOCKS");
    if (strcasecmp(token2, "BLOCKS") != 0) {
        return(DCL_ERROR);
    }

    w = work;
    if (*ch) ch++;
    while (*ch && *ch != ')' && *ch != ',') *w++ = *ch++;
    *w = 0;
    (void) EXP_compute(work,token3);
    if (*token3 == 0) strcpy(token3,"BYTES");
    if (strcasecmp(token3, "BYTES") != 0) {
        return(DCL_ERROR);
    }

    dnum = num * 512;
    strcpy(unit, "");

    if (dnum > 1024) {
        dnum = dnum / 1024;
        strcpy(unit, "KB.");
        if (dnum > 1000) {
            dnum = dnum / 1000;
            strcpy(unit, "MB.");
            if (dnum > 1000) {
                dnum = dnum / 1000;
                strcpy(unit, "GB.");
            }
        }
    }
    if (fmod(dnum * 100 ,100)) {
        sprintf(value, "\"%.2f %s\"", dnum, unit);
    }
    else {
        sprintf(value, "\"%.0f %s\"", dnum, unit);
    }


    return(0);
}


// ---------------------------------------------------------------------------

int f_cvtime(char *name, char *value)
{
    char            str[MAX_TOKEN];
    char            fmt[MAX_TOKEN];
    char            fld[MAX_TOKEN];
    char            work[MAX_TOKEN];
    char            *w;
    char            *ch;
    struct tm       ts;
    time_t          tl;

    if (name == NULL || value == NULL) return(DCL_ERROR);

    *str = 0; *fmt = 0; *fld = 0; *work = 0;
    ch = name;
    w = work;
    while (*ch && *ch != '(') ch++;
    if (*ch) ch++;

    while (*ch && *ch != ')' && *ch != ',') *w++ = *ch++;
    *w = 0;
    (void) EXP_compute(work,str);

    w = work;
    if (*ch) ch++;
    while (*ch && *ch != ')' && *ch != ',') *w++ = *ch++;
    *w = 0;
    (void) EXP_compute(work,fmt);
    if (*fmt == 0) strcpy(fmt,"COMPARISON");

    w = work;
    if (*ch) ch++;
    while (*ch && *ch != ')' && *ch != ',') *w++ = *ch++;
    *w = 0;
    (void) EXP_compute(work,fld);
    if (*fld == 0) strcpy(fld,"DATETIME");

    if (*str)
        tm_str_to_long(str,&tl);
    else
        tm_str_to_long("NOW",&tl);
    tm_long_to_tm(&tl, &ts);

    if (strcasecmp(fld,"DATE") == 0) {
        if (strcasecmp(fmt,"ABSOLUTE") == 0) {
                sprintf(value,
                                "\"%2.2d-%s-%4.4d\"",
                                ts.tm_mday,
                                MONTH[ts.tm_mon + 1],
                                ts.tm_year + 1900);
        }
        if (strcasecmp(fmt,"COMPARISON") == 0) {
                sprintf(value,
                                "\"%4.4d-%2.2d-%2.2d\"",
                                ts.tm_year + 1900,
                                ts.tm_mon + 1,
                                ts.tm_mday);
        }
    }
    if (strcasecmp(fld,"DATETIME") == 0) {
        if (strcasecmp(fmt,"ABSOLUTE") == 0) {
                sprintf(value,
                                "\"%2.2d-%s-%4.4d %2.2d:%2.2d:%2.2d.%2.2d\"",
                                ts.tm_mday,
                                MONTH[ts.tm_mon + 1],
                                ts.tm_year + 1900,
                                ts.tm_hour,
                                ts.tm_min,
                                ts.tm_sec,
                                0);
        }
        if (strcasecmp(fmt,"COMPARISON") == 0) {
                sprintf(value,
                                "\"%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d.%2.2d\"",
                                ts.tm_year + 1900,
                                ts.tm_mon + 1,
                                ts.tm_mday,
                                ts.tm_hour,
                                ts.tm_min,
                                ts.tm_sec,
                                0);
        }
    }

    if (strcasecmp(fld,"DAY") == 0) {
        sprintf(value,"\"%2.2d\"",ts.tm_mday);
    }

    if (strcasecmp(fld,"HOUR") == 0) {
        sprintf(value,"\"%2.2d\"",ts.tm_hour);
    }

    if (strcasecmp(fld,"HUNDRETH") == 0) {
        sprintf(value,"\"%2.2d\"",0);
    }

    if (strcasecmp(fld,"MINUTE") == 0) {
        sprintf(value,"\"%2.2d\"",ts.tm_min);
    }

    if (strcasecmp(fld,"MONTH") == 0) {
        if (strcasecmp(fmt,"ABSOLUTE") == 0) {
                sprintf(value,"\"%s\"",MONTH[ts.tm_mon + 1]);
        }
        if (strcasecmp(fmt,"COMPARISON") == 0) {
            sprintf(value,"\"%2.2d\"",ts.tm_mon + 1);
        }
    }

    if (strcasecmp(fld,"SECOND") == 0) {
        sprintf(value,"\"%2.2d\"",ts.tm_sec);
    }

    if (strcasecmp(fld,"TIME") == 0) {
            sprintf(value,
                            "\"%2.2d:%2.2d:%2.2d.%2.2d\"",
                            ts.tm_hour,
                            ts.tm_min,
                            ts.tm_sec,
                            0);
    }

    if (strcasecmp(fld,"WEEKDAY") == 0) {
        switch (ts.tm_wday){
            case 0 :    strcpy(value,"\"Sunday\""); break;
            case 1 :    strcpy(value,"\"Monday\""); break;
            case 2 :    strcpy(value,"\"Tuesday\""); break;
            case 3 :    strcpy(value,"\"Wednesday\""); break;
            case 4 :    strcpy(value,"\"Thursday\""); break;
            case 5 :    strcpy(value,"\"Friday\""); break;
            case 6 :    strcpy(value,"\"Saturday\""); break;
            default:    strcpy(value,"\"Oops!\""); break;
            }
        }

    if (strcasecmp(fld,"YEAR") == 0) {
        sprintf(value,"\"%4.4d\"",ts.tm_year + 1900);
        }
    return(0);
}

//------------------------------------------------------------------------

int f_directory(char *value)
{
        char dos[_MAX_PATH+1];
        char dir[_MAX_PATH+1];
        char vms[MAX_TOKEN];

    if (value == NULL) return(DCL_ERROR);

    *dos = 0; *vms = 0; *dir = 0;
    if (_getcwd(dir,_MAX_PATH) != NULL) {
        strcat(dir,SLASH_STR);
    }
    _splitpath(dir, NULL, dos, NULL, NULL);
    cvfs_dos_to_vms(dos,vms);
    sprintf(value,"\"%s\"",vms);
    return(0);
}
//------------------------------------------------------------------------

int f_dosvar(char *name, char *value)
{
        char    work[MAX_TOKEN];
        char    result[MAX_TOKEN];
    char    *ch;
    char    *w;

    if (name == NULL || value == NULL) return(DCL_ERROR);

    *work = 0; *result = 0;
    ch = name;
    w  = work;
        while (*ch && *ch != '(') ch++;
        if (*ch) ch++;
        while (*ch && *ch != ')' && *ch != ',') *w++ = *ch++;
    *w = 0;

        dcl_string(work,result,MAX_TOKEN);
    if ((w = getenv(strupr(result))) != NULL)
        sprintf(value,"\"%s\"",w);
    else
        sprintf(value,"\"\"");
    return(0);
}
//------------------------------------------------------------------------

int f_edit(char *name, char *value)
{
        char    string[MAX_TOKEN];
        char    edit[MAX_TOKEN];
        char    work[MAX_TOKEN];
        char    *w;
        char    *ch;
        int     i = 0;
        int     j = 0;
        int     quoting = 0;
        int     comment = 0;

    if (name == NULL || value == NULL) return(DCL_ERROR);

    *string = 0; *edit = 0; *work = 0;
    ch = name;
    w = work;
    while (*ch && *ch != '(') ch++;
    if (*ch) ch++;
    while (*ch && *ch != ')' && *ch != ',') *w++ = *ch++;
    *w = 0;

    (void) EXP_compute(work,string);

    w = work;
        if (*ch) ch++;
        while (*ch && *ch != ')') *w++ = *ch++;
    *w = 0;
    (void) EXP_compute(work,edit);

    ch = value;
    w  = string;
    if (strcasecmp(edit,"COLLAPSE") == 0) {
        *ch++ = '"';
        while(*w) {
            if (*w == '"') quoting = quoting ? 0 : 1;
            if ((*w != ' ' && *w != '\t') || quoting)
                *ch++ = *w;
            w++;
        }
        *ch++ = '"';
        *ch = 0;
    }

    if (strcasecmp(edit,"COMPRESS") == 0) {
        *ch++ = '"';
        while(*w) {
            if (*w == '"') quoting = quoting ? 0 : 1;
            if (quoting)
                *ch++ = *w++;
            else {
                if (*w == ' ' || *w == '\t')
                    i++;
                else
                    i = 0;
                if (i <= 1) *ch++ = *w;
                    w++;
                }
        }
        *ch++ = '"';
        *ch = 0;
    }

    if (strcasecmp(edit,"LOWERCASE") == 0) {
        *ch++ = '"';
        while (*w){
            if (*w == '"') quoting = quoting ? 0 : 1;
            if (!quoting) *w = (char) tolower(*w);
            *ch++ = *w++;
        }
        *ch++ = '"';
        *ch = 0;
    }

    if (strcasecmp(edit,"TRIM") == 0) {
        *ch++ = '"';
        while(w[i] && (w[i] == ' ' || w[i] == '\t')) i++;
        j = (int)strlen(string) - 1;
        while (j && (w[j] == ' ' || w[j] == '\t')) j--;
        while(i <= j) *ch++ = w[i++];
        *ch++ = '"';
        *ch = 0;
    }

    if (strcasecmp(edit,"UNCOMMENT") == 0) {
        *ch++ = '"';
        while (*w) {
            if (*w == '"') quoting = quoting ? 0 : 1;
            if (*w == '!' && !quoting) comment = 1;
            if (!comment) *ch++ = *w;
            w++;
        }
        *ch++ = '"';
        *ch = 0;
    }

    if (strcasecmp(edit,"UPCASE") == 0) {
        *ch++ = '"';
        while (*w) {
            if (*w == '"') quoting = quoting ? 0 : 1;
            if (!quoting) *w = (char)toupper(*w);
            *ch++ = *w++;
        }
        *ch++ = '"';
        *ch = 0;
    }

    return(0);
}

//------------------------------------------------------------------------

int f_element(char *name, char *value)
{
    char    string[MAX_TOKEN];
    char    delim[MAX_TOKEN];
    char    work[MAX_TOKEN];
    char    *w;
    char    *ch;
    char    *tk;
    int     num     = 0;

    if (name == NULL || value == NULL) return(DCL_ERROR);

    *string = 0; *delim = 0; *work = 0;
    ch = name;
    w  = work;
    while (*ch && *ch != '(') ch++;
    if (*ch) ch++;
    while (*ch && *ch != ',' && *ch != ')') *w++ = *ch++;
    *w = 0;
    (void) EXP_compute(work,string);
    num = atoi(string);

    w = work;
    if (*ch) ch++;
    while (*ch && *ch != ',' && *ch != ')') *w++ = *ch++;
    *w = 0;
    (void) EXP_compute(work,delim);
    delim[1] = 0;

    w = work;
    if (*ch) ch++;
    while (*ch && *ch != ')') *w++ = *ch++;
    *w = 0;
    (void) EXP_compute(work,string);

    tk = strtok(string,delim);
    while (tk && num--){
        tk = strtok(NULL,delim);
    }
    if (tk)
        sprintf(value,"\"%s\"",tk);
    else
        sprintf(value,"\"%s\"",delim);

    return(0);
}

//------------------------------------------------------------------------

int f_extract(char *name, char *value)
{
    char    work[MAX_TOKEN];
    char    string[MAX_TOKEN];
    char    *w;
    char    *ch;
    int     offset  = 0;
    int     length  = 0;

    if (name == NULL || value == NULL) return(DCL_ERROR);

    *work = 0; *string = 0;
    ch = name;
    w  = work;
    while (*ch && *ch != '(') ch++;
    if (*ch) ch++;
    while (*ch && *ch != ',' && *ch != ')') *w++ = *ch++;
    *w = 0;
    (void) EXP_compute(work,string);
    offset = atoi(string);

    w = work;
    if (*ch) ch++;
    while (*ch && *ch != ',' && *ch != ')') *w++ = *ch++;
    *w = 0;
    (void) EXP_compute(work,string);
    length = atoi(string);

    w = work;
    if (*ch) ch++;
    while (*ch && *ch != ',' && *ch != ')') *w++ = *ch++;
    *w = 0;
    (void) EXP_compute(work,string);

    ch = string;
    w  = value;
    *w++ = '"';
    while (*ch && offset--) ch++;
    while (*ch && length--) *w++ = *ch++;
    *w++ = '"';
    *w = 0;

    return(0);
}

//------------------------------------------------------------------------

int f_integer(char *name, char *value)
{
        char    work[MAX_TOKEN];
        char    result[MAX_TOKEN];
    char    *ch;
    char    *w;
    int     i = 0;

    if (name == NULL || value == NULL) return(DCL_ERROR);

    *work = 0; *result = 0;
    ch = name;
    w  = work;
        while (*ch && *ch != '(') ch++;
        if (*ch) ch++;
        while (*ch && *ch != ')' && *ch != ',') *w++ = *ch++;
    *w = 0;

        (void) EXP_compute(work,result);
    i = atoi(result);
    sprintf(value,"%d",i);
    return(0);
}

//------------------------------------------------------------------------

int f_length(char *name,char *value)
{
        char    work[MAX_TOKEN];
        char    string[MAX_TOKEN];
    char    *w;
    char    *ch;

    if (name == NULL || value == NULL) return(DCL_ERROR);

    *work = 0; *string = 0;
    w  = work;
        ch = name;
        while (*ch && *ch != '(') ch++;
        if (*ch) ch++;
        while (*ch && *ch != ',' && *ch != ')') *w++ = *ch++;
        *w = 0;
    (void) EXP_compute(work,string);
    sprintf(value, "%d", (int)strlen(string));
    return(0);
}

//------------------------------------------------------------------------

int f_locate(char *name, char *value)
{
        char    work[MAX_TOKEN];
        char    string[MAX_TOKEN];
        char    substr[MAX_TOKEN];
        char    *w;
        char    *ch;

    if (name == NULL || value == NULL) return(DCL_ERROR);

    *work = 0; *string = 0; *substr = 0;
    ch = name;
    w  = work;
        while (*ch && *ch != '(') ch++;
        if (*ch) ch++;
        while (*ch && *ch != ')' && *ch != ',') *w++ = *ch++;
    *w = 0;
    (void) EXP_compute(work,substr);

    w  = work;
        if (*ch) ch++;
        while (*ch && *ch != ')' && *ch != ',') *w++ = *ch++;
    *w = 0;
    (void) EXP_compute(work,string);

    w = strstr(string,substr);
    if (w)
        sprintf(value,"%d",(int)(w - string));
    else
        sprintf(value,"%d",(int)strlen(string));
    return(0);
}

// ---------------------------------------------------------------------------

int f_match_wild(char *name, char *value)
{
    char            token1[MAX_TOKEN];
    char            token2[MAX_TOKEN];
    char            work[MAX_TOKEN];
    char            *w;
    char            *ch;
    pcre            *re;
    const char         *error;
    int             erroffset;

    if (name == NULL || value == NULL) return(DCL_ERROR);

    *token1 = 0; *token2 = 0; *work = 0;
    ch = name;
    w = work;
    while (*ch && *ch != '(') ch++;
    if (*ch) ch++;

    while (*ch && *ch != ')' && *ch != ',') *w++ = *ch++;
    *w = 0;
    (void) EXP_compute(work,token1);

    w = work;
    if (*ch) ch++;
    while (*ch && *ch != ')' && *ch != ',') *w++ = *ch++;
    *w = 0;
    (void) EXP_compute(work,token2);

    w = work;
    for(ch = token2; *ch; ch++) {
        if (*ch == '*') {*w++ = '.'; *w++ = '*';}
        else if (*ch == '%') {*w++ = '.';}
        else {*w++ = *ch;}
    }
    *w = 0;

    sprintf(value, "\"FALSE\"");

    re = pcre_compile(work, PCRE_CASELESS, &error, &erroffset, NULL);            /* use default character tables */
    if (re != NULL) {
        int rc;
        int ovector[30];
        rc = pcre_exec(re,             /* result of pcre_compile() */
                       NULL,           /* we didn't study the pattern */
                       token1,  /* the subject string */
                       strlen(token1),             /* the length of the subject string */
                       0,              /* start at offset 0 in the subject */
                       0,              /* default options */
                       ovector,        /* vector of integers for substring information */
                       30);            /* number of elements (NOT size in bytes) */
        if (rc >= 0) {
            sprintf(value, "\"TRUE\"");
        }
        pcre_free(re);
    }

    return(0);
}

//------------------------------------------------------------------------

int f_mode(char *value)
{
    if (value == NULL) return(DCL_ERROR);

    switch(mode){
        case MODE_BATCH         :   strcpy(value,"\"BATCH\"");
                                    break;
        case MODE_INTERACTIVE   :   strcpy(value,"\"INTERACTIVE\"");
                                    break;
        default                 :   strcpy(value,"\"UNDEFINED\"");
        }
    return(0);
}

//------------------------------------------------------------------------

int f_node(char *value)
{
    char    work[MAX_TOKEN];
    size_t  size = MAX_TOKEN;

    if (value == NULL) return(DCL_ERROR);
#ifdef _WIN32
    if (!GetComputerName(work, (DWORD *)&size))
#else
    if (gethostname(work, size))
#endif
        *work = 0;
    sprintf(value,"\"%s\"",work);
    return(0);
}

//------------------------------------------------------------------------

int f_parse(char *name, char *value)
{
    char    item[MAX_TOKEN];
    char    work[MAX_TOKEN];
    char    vms[MAX_TOKEN];
    char    dos[MAX_TOKEN];
    char    path[_MAX_PATH];
    char    drive[_MAX_DRIVE];
    char    dir[_MAX_DIR];
    char    fname[_MAX_FNAME];
    char    ext[_MAX_EXT];
    char    def_path[_MAX_PATH];
    char    def_drive[_MAX_DRIVE];
    char    def_dir[_MAX_DIR];
    char    def_fname[_MAX_FNAME];
    char    def_ext[_MAX_EXT];
    char    rel_path[_MAX_PATH];
    char    rel_drive[_MAX_DRIVE];
    char    rel_dir[_MAX_DIR];
    char    rel_fname[_MAX_FNAME];
    char    rel_ext[_MAX_EXT];
    char    field[MAX_TOKEN];
    char    type[MAX_TOKEN];
    char    *w;
    char    *ch;
    int     retcod  = 0;
    int     i;

    if (name == NULL || value == NULL) return(DCL_ERROR);

    _STATUS = 0;
    memset(item,    0, MAX_TOKEN);
    memset(work,    0, MAX_TOKEN);
    memset(vms,     0, MAX_TOKEN);
    memset(dos,     0, MAX_TOKEN);
    memset(path,    0, _MAX_PATH);
    memset(drive,   0, _MAX_DRIVE);
    memset(dir,     0, _MAX_DIR);
    memset(fname,   0, _MAX_FNAME);
    memset(ext,     0, _MAX_EXT);
    memset(def_path,    0, _MAX_PATH);
    memset(def_drive,   0, _MAX_DRIVE);
    memset(def_dir,     0, _MAX_DIR);
    memset(def_fname,   0, _MAX_FNAME);
    memset(def_ext,     0, _MAX_EXT);
    memset(rel_path,    0, _MAX_PATH);
    memset(rel_drive,   0, _MAX_DRIVE);
    memset(rel_dir,     0, _MAX_DIR);
    memset(rel_fname,   0, _MAX_FNAME);
    memset(rel_ext,     0, _MAX_EXT);
    memset(field,   0, MAX_TOKEN);
    memset(type,    0, MAX_TOKEN);

    *value = 0;
    ch = name;
    w = work;
    while (*ch && *ch != '(') ch++;
    if (*ch) ch++;
    while (*ch && *ch != ')' && *ch != ',') *w++ = *ch++;
    *w = 0;
    if (*work == '"')
        dcl_string(work,path,_MAX_PATH);
    else
        (void) symbol_get(work,path);

    w = work;
    if (*ch) ch++;
    while (*ch && *ch != ')' && *ch != ',') *w++ = *ch++;
    *w = 0;
    if (*work == '"')
        dcl_string(work,def_path,_MAX_PATH);
    else
        (void) symbol_get(work,def_path);

    w = work;
    if (*ch) ch++;
    while (*ch && *ch != ')' && *ch != ',') *w++ = *ch++;
    *w = 0;
    if (*work == '"')
        dcl_string(work,rel_path,_MAX_PATH);
    else
        (void) symbol_get(work,rel_path);

    w = work;
    if (*ch) ch++;
    while (*ch && *ch != ')' && *ch != ',') *w++ = *ch++;
    *w = 0;
    if (*work == '"')
        dcl_string(work,field, MAX_TOKEN);
    else
        (void) symbol_get(work, field);

    w = work;
    if (*ch) ch++;
    while (*ch && *ch != ')' && *ch != ',') *w++ = *ch++;
    *w = 0;
    if (*work == '"')
        dcl_string(work,type, MAX_TOKEN);
    else
        (void) symbol_get(work, type);

    cvfs_vms_to_dos(path,dos,&i);
    (void) _splitpath(dos, drive, dir, fname, ext);

    cvfs_vms_to_dos(def_path,dos,&i);
    (void) _splitpath(dos, def_drive, def_dir, def_fname, def_ext);

    cvfs_vms_to_dos(rel_path,dos,&i);
    (void) _splitpath(dos, rel_drive, rel_dir, rel_fname, rel_ext);

    if (*drive == 0) strcpy(drive, def_drive);
    if (*drive == 0) strcpy(drive, rel_drive);

    if (*dir == 0) strcpy(dir, def_dir);
    if (*dir == 0) strcpy(dir, rel_dir);

    if (*fname == 0) strcpy(fname, def_fname);
    if (*fname == 0) strcpy(fname, rel_fname);

    if (*ext == 0) strcpy(ext, def_ext);
    if (*ext == 0) strcpy(ext, rel_ext);

    _makepath(path, drive, dir, fname, ext);

    //if (strcasecmp(type,"SYNTAX_ONLY") == 0) {

    if (*field == 0) {
        (void) cvfs_dos_to_vms(path, vms);
        sprintf(value, "\"%s\"", vms);
    }
    else if (strcasecmp(field, "NODE") == 0) {
        strcpy(value, "\"\"");
    }
    else if (strcasecmp(field, "DEVICE") == 0) {
        (void) cvfs_dos_to_vms(drive, vms);
        sprintf(value, "\"%s\"", vms);
    }
    else if (strcasecmp(field, "DIRECTORY") == 0) {
        (void) cvfs_dos_to_vms(dir, vms);
        sprintf(value, "\"%s\"", vms);
    }
    else if (strcasecmp(field, "NAME") == 0) {
        (void) cvfs_dos_to_vms(fname, vms);
        sprintf(value, "\"%s\"", vms);
    }
    else if (strcasecmp(field, "TYPE") == 0) {
        (void) cvfs_dos_to_vms(ext, vms);
        sprintf(value, "\"%s\"", vms);
    }
    else if (strcasecmp(field, "VERSION") == 0) {
        strcpy(value, "\"1\"");
    }
    //}

    return(retcod);
}

//------------------------------------------------------------------------

int f_process(char *value)
{
    DWORD   dwPid;

    if (value == NULL) return(DCL_ERROR);

//    dwPid = GetCurrentProcessId();
    dwPid= (pid_t)getpid();
    sprintf(value,"\"%ld\"",dwPid);
    return(0);
}

//------------------------------------------------------------------------

int f_privilege(char *name, char *value)
{
    if (value == NULL) return(DCL_ERROR);

    sprintf(value,"\"TRUE\"");

    return(0);
}

//------------------------------------------------------------------------

int f_string(char *name, char *value)
{
        char    work[MAX_TOKEN];
        char    result[MAX_TOKEN];
    char    *ch;
    char    *w;

    if (name == NULL || value == NULL) return(DCL_ERROR);

    *work = 0; *result = 0;
    ch = name;
    w  = work;
        while (*ch && *ch != '(') ch++;
        if (*ch) ch++;
        while (*ch && *ch != ')' && *ch != ',') *w++ = *ch++;
    *w = 0;

    (void) EXP_compute(work,result);
    sprintf(value,"\"%s\"",result);
    return(0);
}

//------------------------------------------------------------------------

int f_time(char *value)
{
        struct  tm  *time_s;
        time_t  time_l;

    if (value == NULL) return(DCL_ERROR);

        (void) time(&time_l);
        time_s = localtime(&time_l);
        sprintf(value,
                        "\"%2.2d-%s-%4.4d %2.2d:%2.2d:%2.2d.00\"",
                        time_s->tm_mday,
                        MONTH[time_s->tm_mon+1],
                        time_s->tm_year+1900,
                        time_s->tm_hour,
                        time_s->tm_min,
                        time_s->tm_sec);
    return(0);
}

//------------------------------------------------------------------------

int f_trnlnm(char *name, char *value)
{
    char    work[MAX_TOKEN];
    char    lognam[MAX_TOKEN];
    char    logval[MAX_TOKEN];
    char    *w;
    char    *ch;

    if (name == NULL || value == NULL) return(DCL_ERROR);

    *work = 0; *lognam = 0; *logval = 0;
    ch = name;
    w  = work;
    while (*ch && *ch != '(') ch++;
    if (*ch) ch++;
    while (*ch && *ch != ')') *w++ = *ch++;
    *w = 0;
    (void) EXP_compute(work,lognam);

    (void) logical_get(lognam,logval);
    sprintf(value,"\"%s\"",logval);
    return(0);
}

//------------------------------------------------------------------------

int f_type(char *name, char *value)
{
    char    work[MAX_TOKEN];
    char    result[MAX_TOKEN];
    char    *ch;
    char    *w;

    if (name == NULL || value == NULL) return(DCL_ERROR);

    *work = 0; *result = 0;
    ch = name;
    w  = work;
    while (*ch && *ch != '(') ch++;
    if (*ch) ch++;
    while (*ch && *ch != ')' && *ch != ',') *w++ = *ch++;
    *w = 0;

    (void) symbol_get(work,result);
    if (*result) {
        if (EXP_isnumber(result))
            strcpy(value,"\"INTEGER\"");
        else
            strcpy(value,"\"STRING\"");
        }
    else
        strcpy(value,"\"UNDEFINED\"");
    return(0);
}

//------------------------------------------------------------------------

int f_verify(char *name,char *value)
{
        char work[MAX_TOKEN];
        char expr[MAX_TOKEN];
        char *ch;
        char *w;

    if (name == NULL || value == NULL) return(DCL_ERROR);

    *work = 0; *expr = 0;
    if (verify & VERIFY_PROC)
                value[0] = '1';
        else
                value[0] = '0';
        value[1] = 0;

        ch = name;
        w  = work;
        while (*ch && *ch != '(') ch++;
        if (*ch) ch++;
        while (*ch && *ch != ')' && *ch != ',') *w++ = *ch++;
        *w = 0;
    if (*work){
            (void) EXP_compute(work,expr);
            *expr = (char)toupper(*expr);
            if (*expr == '1' || *expr == 'T' || *expr == 'Y')
                    verify |= VERIFY_PROC;
            else
                    verify &= ~VERIFY_PROC;
        }

        w = work;
        if (*ch) ch++;
        while (*ch && *ch != ')') *w++ = *ch++;
        *w = 0;
    if (*work) {
            (void) EXP_compute(work,expr);
            *expr = (char)toupper(*expr);
            if (*expr == '1' || *expr == 'T' || *expr == 'Y')
                    verify |= VERIFY_IMAGE;
            else
                    verify &= ~VERIFY_IMAGE;
        }
    return(0);
}

//------------------------------------------------------------------------

int f_unique(char *value)
{
    char            buffer[MAX_TOKEN];
    char            work[MAX_TOKEN];
    struct  md5_ctx ctx;
    int             i = 0;

    if (value == NULL) return(DCL_ERROR);

    memset(&ctx, 0, sizeof(ctx));
    memset(buffer, 0, sizeof(buffer));
#ifdef _WIN32
    (void) tmpnam(buffer);
#else
    i =  mkstemp(buffer);
#endif

    md5_init_ctx(&ctx);
    md5_process_bytes(buffer, strlen(buffer), &ctx);
    md5_finish_ctx (&ctx, buffer);

    strcpy(value, "\"");
    for(i = 0; i < 16; i++) {
        sprintf(work, "%2.2X", buffer[i] & 0xFF);
        strcat(value, work);
    }
    strcat(value, "\"");

    return(0);
}

//------------------------------------------------------------------------

int f_user(char *value)
{
    if (value == NULL) return(DCL_ERROR);

    strcpy(INI_NAME,"INI$USERNAME");
    (void) logical_get(INI_NAME,INI_VALUE);
    sprintf(value,"\"%s\"",INI_VALUE);

    return(0);
}

//------------------------------------------------------------------------

int f_environment(char *name, char *value)
{
    char    item[MAX_TOKEN];
    char    work[MAX_TOKEN];
    char    *w;
    char    *ch;

    if (name == NULL || value == NULL) return(DCL_ERROR);

    *item = 0; *work = 0;
    ch = name;
    w = work;
    while (*ch && *ch != '(') ch++;
    if (*ch) ch++;
    while (*ch && *ch != ')') *w++ = *ch++;
    *w = 0;
    (void) EXP_compute(work,item);

    ch = value;
    if (strcasecmp(item,"CONTROL") == 0) {
        strcpy(value,"\"");
        if (CONTROL_Y_ALLOWED) strcat(value,"C");
        if (CONTROL_Y_ALLOWED && CONTROL_T_ALLOWED) strcat(value,",");
        if (CONTROL_Y_ALLOWED) strcat(value,"T");
        strcat(value,"\"");
    }

    if (strcasecmp(item,"DEFAULT") == 0) {
        if (getcwd(work,MAX_TOKEN-1) != NULL) {
            strcat(work,SLASH_STR);
        }
        cvfs_dos_to_vms(work,item);
        sprintf(value,"\"%s\"",item);
    }

    if (strcasecmp(item,"DEPTH") == 0) {
        sprintf(value,"\"%d\"",D-1);
    }

    if (strcasecmp(item,"INTERACTIVE") == 0) {
        if (mode == MODE_INTERACTIVE)
            strcpy(value,"\"TRUE\"");
        else
            strcpy(value,"\"FALSE\"");
    }

    if (strcasecmp(item,"MAX_DEPTH") == 0) {
        strcpy(value,"\"7\"");
    }

    if (strcasecmp(item,"PROCEDURE") == 0) {
        sprintf(value,"\"%s\"",dcl[D].filename);
    }

    if (strcasecmp(item,"PROMPT") == 0) {
        sprintf(value,"\"%s\"",dcl_prompt);
    }

    if (strcasecmp(item,"SYMBOL_SCOPE") == 0) {
        if (dcl[D].symbol_scope & SYMSCO_LOCAL)
            strcpy(value,"\"LOCAL");
        else
            strcpy(value,"\"NOLOCAL");
        if (dcl[D].symbol_scope & SYMSCO_GLOBAL)
            strcat(value,",GLOBAL\"");
        else
            strcpy(value,",NOGLOBAL\"");
    }

    if (strcasecmp(item,"VERB_SCOPE") == 0) {
        strcpy(value,"\"GLOBAL\"");
    }

    if (strcasecmp(item,"VERIFY_IMAGE") == 0) {
        if (verify & VERIFY_IMAGE)
            strcpy(value,"\"TRUE\"");
        else
            strcpy(value,"\"FALSE\"");
    }

    if (strcasecmp(item,"VERIFY_PROCEDURE") == 0) {
        if (verify & VERIFY_PROC)
            strcpy(value,"\"TRUE\"");
        else
            strcpy(value,"\"FALSE\"");
    }

    if (strcasecmp(item,"VERSION") == 0) {
        sprintf(value,"\"%s\"",VERSION);
    }
    return(0);
}

//------------------------------------------------------------------------

int f_file_attributes(char *name, char *value)
{
    char    item[MAX_TOKEN];
    char    work[MAX_TOKEN];
    char    vms[MAX_TOKEN];
    char    dos[MAX_TOKEN];
    char    drive[_MAX_DRIVE];
    char    dir[_MAX_DIR];
    char    ext[_MAX_EXT];
    char    *w;
    char    *ch;
    int     retcod  = 0;
    int     i;
    DCL_FIND_DATA ff;
    int     handle;

    if (name == NULL || value == NULL) return(DCL_ERROR);

    _STATUS = 0;
    *item = 0; *work = 0; *vms = 0; *dos = 0;
    *drive = 0; *dir = 0; *ext = 0;
    ch = name;
    w = work;
    while (*ch && *ch != '(') ch++;
    if (*ch) ch++;
    while (*ch && *ch != ')' && *ch != ',') *w++ = *ch++;
    *w = 0;

    (void) EXP_compute(work,vms);

    w = work;
    if (*ch) ch++;
    while (*ch && *ch != ')') *w++ = *ch++;
    *w = 0;
    if (*work == '"')
        dcl_string(work,item,MAX_TOKEN);
    else
        (void) symbol_get(work,item);

    cvfs_vms_to_dos(vms,dos,&i);
#ifdef _WIN32
    _searchenv(dos, "PATH", ch);
    if (!*ch) {
        _SEVERITY = 1;
        _STATUS = 2;
        retcod = -1;
        goto exit_label;
    }
    strcpy(dos,ch);
#else
    ch = realpath(dos, ch);
    if (ch != NULL) {
        strcpy(dos,ch);
    }
#endif
    _splitpath(dos,drive,dir,name,ext);
    if (*dir == 0) {
        strcpy(dir, ".");
    }

    handle = Dcl_FindFirstFile(dos,&ff);
    if (handle == (int)INVALID_HANDLE_VALUE) {
        _SEVERITY = 1;
        _STATUS = 2;
        retcod = -1;
        goto exit_label;
    }
    (void) Dcl_FindClose(handle);

    if (strcasecmp(item,"ATT") == 0) {
        strcpy(value,"\"W...\"");
        if (ff.dwFileAttributes & _A_RDONLY)  value[1] = 'R';
        if (ff.dwFileAttributes & _A_SUBDIR) value[1] = 'D';
        if (ff.dwFileAttributes & _A_HIDDEN)    value[2] = 'H';
        if (ff.dwFileAttributes & _A_SYSTEM)    value[3] = 'S';
        if (ff.dwFileAttributes & _A_ARCH)   value[4] = 'A';
    }

    if (strcasecmp(item,"DAT") == 0) {
        tm_long_to_str(&ff.WriteTime,work);
        work[20] = 0;
        sprintf(value,"\"%s\"",work);
    }

    if (strcasecmp(item,"DEV") == 0) {
        sprintf(value,"\"%s\"",drive);
    }

    if (strcasecmp(item,"DIR") == 0) {
        cvfs_dos_to_vms(dir,vms);
        sprintf(value,"\"%s\"",vms);
    }

    if (strcasecmp(item,"EXT") == 0) {
        if (*ext)
            sprintf(value,"\"%s\"",&ext[1]);
    }

    if (strcasecmp(item,"NAM") == 0) {
        sprintf(value,"\"%s\"",name);
    }

    if (strcasecmp(item,"SIZ") == 0) {
        sprintf(value,"\"%ld\"",ff.nFileSize);
    }

exit_label:
    return(retcod);
}

//------------------------------------------------------------------------

int f_getdvi(char *name, char *value)
{
    char            item[MAX_TOKEN];
    char            devnam[MAX_TOKEN];
    char            work[MAX_TOKEN];
    char            *w;
    char            *ch;
    int             retcod  = 0;
    DISKINFO        di;
#ifdef _WIN32
    int             drive;
#else
    char			buffer[1024];
#endif
    unsigned long   total_size;
    unsigned long   free_space;
    unsigned long   total_size_mod;
    unsigned long   free_space_mod;
    int				i = 0;

    if (name == NULL || value == NULL) return(DCL_ERROR);

    _STATUS = 0;
    *item = 0; *devnam = 0; *work = 0;
    ch = name;
    w = work;
    while (*ch && *ch != '(') ch++;
    if (*ch) ch++;
    while (*ch && *ch != ')' && *ch != ',') *w++ = *ch++;
    *w = 0;

    (void) EXP_compute(work,devnam);

    w = work;
    if (*ch) ch++;
    while (*ch && *ch != ')') *w++ = *ch++;
    *w = 0;
    (void) EXP_compute(work,item);

        (void) cvfs_translate_logical(devnam);
#ifdef _WIN32
        drive = *devnam - '@';
        if (diskinfo(drive,&di)) {
            _SEVERITY = 1;
            _STATUS = 15;
            retcod = -1;
            goto exit_label;
        }
        total_size = di.TotalNumberOfBytes / 1000000;
        total_size_mod = di.TotalNumberOfBytes % 1000000;
        free_space = di.FreeBytesAvailable / 1000000;
        free_space_mod = di.FreeBytesAvailable % 1000000;
#else
            memset(buffer, 0, sizeof(buffer));
            (void) GetLogicalDriveStrings(sizeof(buffer), buffer);
            for(i = 0; buffer[i]; i = i + 1 + (int)strlen(&buffer[i])) {
           		DISKINFO di;
           		if (diskinfo(&buffer[i],&di)==DCL_OK) {
           			char str[256];
           			strcpy(str, &buffer[i]);
                   	if (strcasecmp(str, di.szMountPoint)==0) {
                   		strcpy(devnam, di.szMountPoint);
                   		break;
            		}
            	}
            }

        if (diskinfo(devnam,&di)) {
            _SEVERITY = 1;
            _STATUS = 15;
            retcod = -1;
            goto exit_label;
        }
        total_size = di.Total1kBlocks / 1000;
        total_size_mod = di.Total1kBlocks % 1000;
        free_space = di.Available1kBlocks / 1000;
        free_space_mod = di.Available1kBlocks % 1000;
#endif
    *value = 0;
    if (strcasecmp(item,"SIZ") == 0) {
        sprintf(value,"\"%lu%6.6lu\"",total_size, total_size_mod);
    }

    if (strcasecmp(item,"FRE") == 0) {
        sprintf(value,"\"%lu%6.6lu\"",free_space, free_space_mod);
    }

    if (strcasecmp(item,"VOL") == 0) {
        sprintf(value,"\"%s\"",di.szName);
    }

    if (strcasecmp(item,"SYS") == 0) {
#ifdef _WIN32
        sprintf(value,"\"%s\"",di.szFileSystemName);
#else
        sprintf(value,"\"%s\"",di.szMountPoint);
#endif
    }

    if (strcasecmp(item,"TYP") == 0) {
        sprintf(value,"\"%s\"",di.szType);
    }

#ifdef _WIN32
    if (strcasecmp(item,"SER") == 0) {
        sprintf(value,"\"%4.4X-%4.4X\"", (unsigned int)di.dwVolumeSerialNumber >> 16,
                                         (unsigned int)di.dwVolumeSerialNumber & 0x0000FFFF);
    }
#endif

exit_label:
    return(retcod);
}

//------------------------------------------------------------------------

int f_getjpi(char *name, char *value)
{
    char            item[MAX_TOKEN];
    char            work[MAX_TOKEN];
    char            *w;
    char            *ch;
    int             retcod  = 0;
    struct tm       *time_s;


    if (name == NULL || value == NULL) return(DCL_ERROR);

    *item = 0; *work = 0;
    ch = name;
    w = work;
    while (*ch && *ch != '(') ch++;
    if (*ch) ch++;
    while (*ch && *ch != ')') *w++ = *ch++;
    *w = 0;

    (void) EXP_compute(work,item);

    if (strcasecmp(item,"JOBTYPE") == 0 ||
        strcasecmp(item,"MODE") == 0) {
        retcod = f_mode(value);
    }
    else if (strcasecmp(item,"LOGINTIM") == 0) {
        time_s = localtime(&start_time);
        sprintf(value,
                "\"%2.2d-%s-%4.4d %2.2d:%2.2d:%2.2d.00\"",
                time_s->tm_mday,
                MONTH[time_s->tm_mon+1],
                time_s->tm_year+1900,
                time_s->tm_hour,
                time_s->tm_min,
                time_s->tm_sec);
    }
    else if(strcasecmp(item,"MASTER_PID") == 0 ||
            strcasecmp(item,"PID") == 0 ||
            strcasecmp(item,"OWNER") == 0) {
        retcod = f_process(value);
    }
    else if(strcasecmp(item,"NODENAME") == 0) {
        retcod = f_node(value);
    }
    else if(strcasecmp(item,"USERNAME") == 0) {
        retcod = f_user(value);
    }
    else {
        sprintf(value,"\"???\"");
    }

    return(retcod);
}

//------------------------------------------------------------------------

int f_getsyi(char *name, char *value)
{
    int             retcod  = 0;
#ifdef _WIN32
    char            item[MAX_TOKEN];
    char            work[MAX_TOKEN];
    char            *w;
    char            *ch;

    MEMORYSTATUS    mi;
    SYSTEM_INFO     si;

    if (name == NULL || value == NULL) return(DCL_ERROR);

    mi.dwLength = sizeof(MEMORYSTATUS);
    GlobalMemoryStatus(&mi);
    GetSystemInfo(&si);

    *item = 0; *work = 0;
    ch = name;
    w = work;
    while (*ch && *ch != '(') ch++;
    if (*ch) ch++;
    while (*ch && *ch != ')') *w++ = *ch++;
    *w = 0;

    (void) EXP_compute(work,item);

    if (strcasecmp(item,"CPU") == 0) {
        HKEY hKey;
        DWORD dwType;
        DWORD dwSize = MAX_TOKEN - 1;

        LONG ret = RegOpenKeyEx(CPU_REG_KEY, CPU_REG_SUBKEY, 0, KEY_READ, &hKey);

        if(ret == ERROR_SUCCESS) {
            ret = RegQueryValueEx(hKey, CPU_NAME, NULL, &dwType, (LPBYTE)work, &dwSize);
            RegCloseKey(hKey);
        }
        dcl_trim(work, item);
        sprintf(value,"\"%s\"", item);
    }
    else if(strcasecmp(item,"SPEED") == 0) {
        HKEY hKey;
        DWORD dwType;
        DWORD dwSize;
        DWORD dwSpeed = -1;

        LONG ret = RegOpenKeyEx(CPU_REG_KEY, CPU_REG_SUBKEY, 0, KEY_READ, &hKey);

        if(ret == ERROR_SUCCESS) {
            dwSize = sizeof(DWORD);
            ret = RegQueryValueEx(hKey,   // key handle
                  CPU_SPEED,              // "~MHz"
                  NULL,                   // reserved, must be NULL
                  &dwType,                // returns the data type
                  (LPBYTE)&dwSpeed,       // data buffer
                  &dwSize);               // size of the buffer

            RegCloseKey(hKey);
        }
        sprintf(value,"\"%d\"",(int)dwSpeed);
    }
    else if(strcasecmp(item,"TOTMEM") == 0 || strcasecmp(item,"MEM") == 0) {
        sprintf(value,"\"%ld\"",mi.dwTotalPhys);
    }
    else if (strcasecmp(item,"FREMEM") == 0) {
        sprintf(value,"\"%lu\"",mi.dwAvailPhys);
    }
    else if (strcasecmp(item,"CODEPAGE") == 0) {
        sprintf(value,"\"%d\"", GetConsoleCP());
    }
    else if (strcasecmp(item,"VERSION") == 0) {
        sprintf(value,"\"%s\"", VERSION);
    }
    else if (strcasecmp(item,"ARCH_TYPE") == 0) {
        sprintf(value,"\"%d\"", si.wProcessorArchitecture);
    }
    else if (strcasecmp(item,"AVAILCPU_CNT") == 0) {
        sprintf(value,"\"%d\"", (int)si.dwNumberOfProcessors);
    }
    else {
        sprintf(value,"\"???\"");
    }
#else
    char            item[MAX_TOKEN];
    char            work[MAX_TOKEN];
    char            *w;
    char            *ch;
    MEMORYSTATUS    mi;

    if (name == NULL || value == NULL) return(DCL_ERROR);


    *item = 0; *work = 0;
    ch = name;
    w = work;
    while (*ch && *ch != '(') ch++;
    if (*ch) ch++;
    while (*ch && *ch != ')') *w++ = *ch++;
    *w = 0;

    (void) EXP_compute(work,item);

    if(strcasecmp(item,"TOTMEM") == 0 || strcasecmp(item,"MEM") == 0) {
    	if (GetMemoryStatus(&mi)==0) {
    		sprintf(value,"\"%ld\"",mi.dwMemTotal);
    	}
    }
    else if (strcasecmp(item,"FREMEM") == 0) {
    	if (GetMemoryStatus(&mi)==0) {
    		sprintf(value,"\"%ld\"",mi.dwMemFree);
    	}
    }
    else if (strcasecmp(item,"CPU") == 0) {
        struct utsname buf;
        if (uname(&buf)== 0) {
            strcpy(work, buf.machine);
            dcl_trim(work, item);
            sprintf(value,"\"%s\"", item);
        }
    }
    else if(strcasecmp(item,"SPEED") == 0) {
    	int		speed = (int)get_cpu_clock_speed();
        sprintf(value,"\"%d\"",speed);
    }
    else if (strcasecmp(item,"VERSION") == 0) {
        sprintf(value,"\"%s\"", VERSION);
    }
#endif
    return(retcod);
}

//------------------------------------------------------------------------

int f_message(char *name, char *value)
{
    char    err[MAX_TOKEN];
    char    work[MAX_TOKEN];
    char    *w;
    char    *ch;
    int     error;
    int     retcod  = 0;

    if (name == NULL || value == NULL) return(DCL_ERROR);

    *err = 0; *work = 0;
    ch = name;
    w = work;
    while (*ch && *ch != '(') ch++;
    if (*ch) ch++;
    while (*ch && *ch != ')') *w++ = *ch++;
    *w = 0;

    (void) EXP_compute(work,err);
    error = atoi(err);

    sprintf(value,"\"NOMESSAGE\"");
    switch (error){
    case 100 : sprintf(value,"\"End of file\"");
               break;
    case 101 : sprintf(value,"\"File not open\"");
               break;
    case 102 : sprintf(value,"\"File not open in READ mode\"");
               break;
    case 103 : sprintf(value,"\"File not open in WRITE mode\"");
               break;
    default:   sprintf(value,"\"%s\"",strerror(error));
               break;
    }

    return(retcod);
}

//------------------------------------------------------------------------

int f_search(char *name, char *value)
{
    char    fspec[MAX_TOKEN];
    char    work[MAX_TOKEN];
    char    *w;
    char    *ch;
    int     retcod  = 0;
    int     stream_id = 0;

    if (name == NULL || value == NULL) return(DCL_ERROR);

    *fspec = 0; *work = 0;
    ch = name;
    w = work;
    while (*ch && *ch != '(') ch++;
    if (*ch) ch++;
    while (*ch && *ch != ')' && *ch != ',') *w++ = *ch++;
    *w = 0;
    (void) EXP_compute(work,fspec);

    w = work;
    if (*ch) ch++;
    while (*ch && *ch != ')') *w++ = *ch++;
    *w = 0;
    if (strlen(work)) {
        stream_id = atoi(work);
        if (stream_id < 1 || stream_id > 8) {
            _SEVERITY = 2;
            retcod = -1;
            goto exit_label;
        }
    }
    else
        stream_id = 0;

    if (strncasecmp(fspec,search[stream_id].filespec,MAX_TOKEN)) {
        strncpy(search[stream_id].filespec,fspec,MAX_TOKEN);
        search[stream_id].handle = _findfirst(fspec,&search[stream_id].ffblk);
        if (search[stream_id].handle == -1) {
            retcod = errno;
        }
    }
    else {
        retcod = _findnext(search[stream_id].handle, &search[stream_id].ffblk);
    }
    if (retcod) {
        _SEVERITY = 1;
        (void) _findclose(search[stream_id].handle);
        search[stream_id].filespec[0] = 0;
    }
    else {
        sprintf(value,"\"%s\"",search[stream_id].ffblk.name);
    }

exit_label:
    return(retcod);
}

//------------------------------------------------------------------------

int f_setprv(char *name, char *value)
{
    char    result[MAX_TOKEN];
    char    work[MAX_TOKEN];
    char    *w;
    char    *ch;

    if (name == NULL || value == NULL) return(DCL_ERROR);

    ch = name;
    w = work;
    while (*ch && *ch != '(') ch++;
    if (*ch) ch++;
    while (*ch && *ch != ')' ) *w++ = *ch++;
    *w = 0;
    dcl_string(work,result,MAX_TOKEN);
    sprintf(value,"\"%s\"",result);

    return(0);
}

// --------------------------------------------------------------------------

int f_fao(char *name,char *value)
{
    char    work[MAX_TOKEN];
    char    format[MAX_TOKEN];
    char    *param[15];
    int     p = 0;
    char    *ch;
    char    *w;

    if (name == NULL || value == NULL) return(DCL_ERROR);

    // call fao
    *format = *work = 0;
    for (p = 15; p; param[--p] = (char *) calloc(1,MAX_TOKEN)) ;

    // get format
    ch = name;
    while (*ch && *ch != '(') ch++;       // F$FAO
    if (*ch) ch++;                        // F$FAO(
    if (*ch == '\"')
        ch = dcl_get_quoted_string(ch,work,MAX_TOKEN);         // F$FAO("fmt",
    else {
        w = work;
        while (*ch && *ch != ')' && *ch != ',') *w++ = *ch++;  // F$FAO("fmt"
        if (*ch) ch++;                                         // F$FAO("fmt",
        *w = 0;
        }
    (void) EXP_compute(work,format);

    // get parameters comma separated (max 15)
    while (*ch && p < 15) {
        if (*ch == '\"')
            ch = dcl_get_quoted_string(ch,work,MAX_TOKEN);     // F$FAO("fmt","Pn",
        else {
            w = work;
            while (*ch && *ch != ')' && *ch != ',') *w++ = *ch++;  // F$FAO("fmt","Pn"
            *w = 0;
            if (*ch) ch++;                        // F$FAO("fmt","Pn",
        }
        (void) EXP_compute(work,param[p]);
        p++;
    }
    (void) fao(work,format,param);
    sprintf(value,"\"%s\"",work);

    for (p = 15; p; free(param[--p])) ;

    return(DCL_OK);
}

int fao(char *result,char *fmt,char *arg[])
{
    unsigned int    i = 0;          /* index to format */
    unsigned int    r = 0;          /* index to result */
    unsigned int    s = 0;          /* saved format index */
    unsigned int    j = 0;          /* work index */
    int             a = 0;          /* index to arg */
    long            ltemp = 0;
    int             repeat = 0;
    unsigned int    length = 0;
    int             skip_parenthesis = 0;
    char            help[32];

    if (result == NULL) return(DCL_ERROR);
    if (fmt    == NULL) return(DCL_ERROR);
    if (arg    == NULL) return(DCL_ERROR);

    memset(help, 0, sizeof(help));

    while (fmt[i]) {
        if (fmt[i] == '!') {
            s = i;
            i++;
            length = 0;
            repeat = 0;
            skip_parenthesis = 0;

            /*  Get first numeric parameter if any   */
            if (fmt[i] == '#') {
                length = (unsigned int) atoi(arg[a++]);
                i++;
                }
            else {
                j = 0;
                while(isdigit((int)fmt[i])) {
                    help[j++] = fmt[i++];
                    help[j] = 0;
                    }
                length = (unsigned int) atoi(help);
                }

            /*  Skip left parenthesis if any    */
            if (fmt[i] == '(') {
                repeat = (int)length;
                length = 0;
                i++;
                skip_parenthesis = 1;
                }


            /*  Get field LENGTH    */
//            scale = 1;
            if (fmt[i] == '#') {
                repeat = atoi(arg[a++]);
                i++;
                }
            else {
                j = 0;
                while(isdigit((int)fmt[i])) {
                    help[j++] = fmt[i++];
                    help[j] = 0;
                    }
                length = (unsigned int)atoi(help);
                }
            if (repeat == 0) repeat = 1;

            /* Process COMMAND   */
            switch (toupper(fmt[i])) {
//** !AS **
                case 'A':
                    i++;
                    if (toupper(fmt[i]) == 'S') {
                        while (repeat--) {
                            if (length == 0) length = strlen(arg[a]);
                            for (j = 0; j < strlen(arg[a]) && j < length; result[r++] = arg[a][j++]) ;
                            for (; j++ < length; result[r++] = ' ') ;
                            a++;
                            }
                        }
                    else
                        for (;s <= i; result[r++] = fmt[s++]) ;
                    break;
//** !OB !OW !OL **
                case 'O':
                    i++;
                    switch (toupper(fmt[i])) {
                        case 'B':
                            if(length == 0) length = 3; /*lint !e616 */
                        case 'W':
                            if(length == 0) length = 6; /*lint !e616 */
                        case 'L':
                            if(length == 0) length = 11; /*lint !e616 */
                            while (repeat--) {
                                ltemp = atol(arg[a++]);
                                r += (unsigned int) sprintf(&result[r],"%*.*lo",
                                                            length,length,(unsigned long) ltemp);
                                }
                            break;
                        default:
                        for (;s <= i; result[r++] = fmt[s++]) ;
                        }
                    break;
//** !XB !XW !XL **
                case 'X':
                    i++;
                    switch (toupper(fmt[i])) {
                        case 'B':
                            if(length == 0) length = 2; /*lint !e616 */
                        case 'W':
                            if(length == 0) length = 4; /*lint !e616 */
                        case 'L':
                            if(length == 0) length = 8; /*lint !e616 */
                            while (repeat--) {
                                ltemp = atol(arg[a++]);
                                r += (unsigned int) sprintf(&result[r],"%*.*lX",
                                                            length,length,(unsigned long) ltemp);
                                }
                            break;
                        default:
                        for (;s <= i; result[r++] = fmt[s++]) ;
                        }
                    break;
//** !ZB !ZW !ZL **
                case 'Z':
                    i++;
                    switch (toupper(fmt[i])) {
                        case 'B':
                        case 'W':
                        case 'L':
                            if(length == 0) length = 1;
                            while (repeat--) {
                                ltemp = atol(arg[a++]);
                                r += (unsigned int) sprintf(&result[r],"%*.*lu",
                                                            length,length,(unsigned long) ltemp);
                                }
                            break;
                        default:
                        for (;s <= i; result[r++] = fmt[s++]) ;
                        }
                    break;
//** !UB !UW !UL **
                case 'U':
                    i++;
                    switch (toupper(fmt[i])) {
                        case 'B':
                        case 'W':
                        case 'L':
                            if(length == 0) length = 1;
                            while (repeat--) {
                                ltemp = atol(arg[a++]);
                                r += (unsigned int)sprintf(&result[r],"%*lu",
                                                           length,(unsigned long) ltemp);
//                                r += sprintf(&result[r],"%*.*lu",length,length,(unsigned long) ltemp);
                                }
                            break;
                        default:
                        for (;s <= i; result[r++] = fmt[s++]) ;
                        }
                    break;
//** !SB !SW !SL **
                case 'S':
                    i++;
                    switch (toupper(fmt[i])) {
                        case 'B':
                        case 'W':
                        case 'L':
                            if(length == 0) length = 1;
                            while (repeat--) {
                                ltemp = atol(arg[a++]);
                                r += (unsigned int)sprintf(&result[r],"% *ld",
                                                           length,(long) ltemp);
//                                r += sprintf(&result[r],"% *.*ld",length,length,(long) ltemp);
                                }
                            break;
                        default:
                        for (;s <= i; result[r++] = fmt[s++]) ;
                        }
                    break;
/* !* */
                case '*':
                    i++;
                    while (length--) {
                        result[r++] = fmt[i];
                        }
                    break;
/* !/ */
                case '/':
                    while (repeat--) {
                        result[r++] = '\r';
                        result[r++] = '\n';
                        }
                    break;
/* !_ */
                case '_':
                    while (repeat--) {
                        result[r++] = '\t';
                        }
                    break;
/* !^ */        case '^':
                    while (repeat--) {
                        result[r++] = '\f';
                        }
                    break;
/* !! */        case '!':
                    while (repeat--) {
                        result[r++] = '!';
                        }
                    break;
/* !%D !%T */   case '%':
                    i++;
                    switch (toupper(fmt[i])) {
                        case 'D':
                            (void)f_time(help);
                            help[strlen(help) - 1] = 0;
                            if (length == 0) length = strlen(&help[1]);
                            r += (unsigned int)sprintf(&result[r],"%-*.*s",
                                                       length,length,&help[1]);
                            a++;
                            break;
                        case 'T':
                            (void) f_time(help);
                            help[strlen(help) - 1] = 0;
                            if (length == 0) length = strlen(&help[13]);
                            r += (unsigned int)sprintf(&result[r],"%-*.*s",
                                                       length,length,&help[13]);
                            a++;
                            break;
                        default:
                        for (;s <= i; result[r++] = fmt[s++]) ;
                        }
                    break;
/* !- */        case '-':
                    while (repeat--) {
                        if (a) a--;
                        }
                    break;
/* !+ */        case '+':
                    while (repeat--) {
                        a++;
                        }
                    break;
                default:
                    for (;s <= i; result[r++] = fmt[s++]) ;
                }

            }
        else {
            result[r++] = fmt[i];
            }
        i++;
        if (skip_parenthesis && fmt[i] == ')')
            i++;
        }
    result[r] = 0;

    return(DCL_OK);
}
// ---------------------------------------------------------------------------
int f_cvui(char *name,char *value)
{
    char    work[MAX_TOKEN];
    int     start;
    int     length;
    char    str[MAX_TOKEN];
    unsigned long result;
    char    *ch;
    char    *w;

    if (name == NULL || value == NULL) return(DCL_ERROR);

    // call cvui
    *str = *work = 0;

    ch = name;
    w = work;
    while (*ch && *ch != '(') ch++;
    if (*ch) ch++;
    while (*ch && *ch != ',' && *ch != ')') *w++ = *ch++;
    *w = 0;
    (void) EXP_compute(work,str);
    start = atoi(str);

    w = work;
    if (*ch) ch++;
    while (*ch && *ch != ',' && *ch != ')') *w++ = *ch++;
    *w = 0;
    (void) EXP_compute(work,str);
    length = atoi(str);

    w = work;
    if (*ch) ch++;
    while (*ch && *ch != ')') *w++ = *ch++;
    *w = 0;
    (void) EXP_compute(work,str);

    result = cvui(start,length,str);
    sprintf(value,"\"%lu\"",result);

    return(DCL_OK);
}

int f_cvsi(char *name,char *value)
{
    char    work[MAX_TOKEN];
    int     start;
    int     length;
    char    str[MAX_TOKEN];
    long    result;
    char    *ch;
    char    *w;

    if (name == NULL || value == NULL) return(DCL_ERROR);

    // call cvui
    *str = *work = 0;

    ch = name;
    w = work;
    while (*ch && *ch != '(') ch++;
    if (*ch) ch++;
    while (*ch && *ch != ',' && *ch != ')') *w++ = *ch++;
    *w = 0;
    (void) EXP_compute(work,str);
    start = atoi(str);

    w = work;
    if (*ch) ch++;
    while (*ch && *ch != ',' && *ch != ')') *w++ = *ch++;
    *w = 0;
    (void) EXP_compute(work,str);
    length = atoi(str);

    w = work;
    if (*ch) ch++;
    while (*ch && *ch != ')') *w++ = *ch++;
    *w = 0;
    (void) EXP_compute(work,str);

    result = (long)cvui(start,length,str);
    sprintf(value,"\"%ld\"",result);

    return(DCL_OK);
}

int     is_bit_set(const char *bmp, int bit)
{
    if (bmp == NULL) return(0);
    return(bmp[bit / 8] & (0x80 >> (bit % 8)));
}

void    set_bit(char *bmp, int bit)
{
    if (bmp == NULL) return;
    bmp[bit / 8] |= (char) (0x80 >> (bit % 8));
}

void    reset_bit(char *bmp, int bit)
{
    if (bmp == NULL) return;
    bmp[bit / 8] &= (char)(~(0x80 >> (bit % 8)));
}

unsigned long cvui(int start,int length,const char *str)
{
    int i,j;
    union {
       unsigned long l;
       char ch[4];
    } result;

    if (str == NULL) return(0);

    result.l = 0;
    i = ((start + length) / 8) + 1;
    if (i > (int)strlen(str) || length > 32) {
        // error invalid range
        return(0L);
    }
    else {
        j = 31;
        result.l = 0L;
        for (i = length - 1; i > 0; i--) {
            if (is_bit_set(str,i + start))
                set_bit(result.ch,j);
            j--;
        }
        i = result.ch[0]; result.ch[0] = result.ch[3]; result.ch[3] = (char)i;
        i = result.ch[1]; result.ch[1] = result.ch[2]; result.ch[2] = (char)i;
        return(result.l);
    }
}

long cvsi(int start,int length,const char *str)
{
    return((long) cvui(start,length,str));
}


#ifdef __linux
int GetMemoryStatus(MEMORYSTATUS *mi)
{
    FILE 	*fp = NULL;
    char  	work[MAX_TOKEN];
    int		rc = 0;

    memset(mi,0,sizeof(MEMORYSTATUS));

    if ((fp = popen("cat /proc/meminfo", "r")) != NULL) {
        while(fgets(work, sizeof(work), fp))  {
        	char name[MAX_TOKEN];
        	DWORD value = 0;
        	rc = sscanf(work, "%s %lu", name, &value);
        	if (rc > 0) {
        		if (strcmp(name, "MemTotal:") == 0)	mi->dwMemTotal = value;
        		if (strcmp(name, "MemFree:") == 0) 	mi->dwMemFree = value;
        		if (strcmp(name, "Buffers:") == 0) 	mi->dwBuffers = value;
        		if (strcmp(name, "Cached:") == 0) 	mi->dwCached = value;
        	}
        }
        pclose(fp);
    }
    else {
        _STATUS = errno;
        _SEVERITY = 2;
        return(-1);
    }

    return(0);
}

float get_cpu_clock_speed()
{
	FILE* fp;
	char buffer[2048];
	size_t bytes_read;
	char* match;
	float clock_speed;
	/* Read the entire contents of /proc/cpuinfo into the buffer. */
	fp = fopen ("/proc/cpuinfo", "r");
	bytes_read = fread (buffer, 1, sizeof (buffer), fp);
	fclose (fp);
	/* Bail if read failed or if buffer isn’t big enough. */
	if (bytes_read == 0)
		return 0;
	/* NUL-terminate the text. */
	buffer[bytes_read] = '\0';
	/* Locate the line that starts with “cpu MHz”. */
	match = strstr (buffer, "cpu MHz");
	if (match == NULL)
		return 0;
	/* Parse the line to extract the clock speed. */
	sscanf (match, "cpu MHz : %f", &clock_speed);
	return clock_speed;
}
#endif
