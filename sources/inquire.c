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
#include "platform.h"
#include "dcl.h"
#include "dcltime.h"
#include "termio.h"

// Inquire -----------------------------------------------------
RPARAM_T INQUIRE_PARAM[] = {
    {1,"Symbol",MANDATORY},
    {2,"Prompt",0},
    {0,"",0}
    };
RPARAM_T INQUIRE_QUAL[] = {
    {1,"/LOCAL",0},
    {2,"/GLOBAL",0},
    {3,"/PUNCTUATION",0},
    {4,"/NOPUNCTUATION",0},
    {5,"/TIMEOUT",VALUE},
    {6,"/NOTIMEOUT",0},
    {7,"/POSITION",VALUE},
    {8,"/LENGTH",VALUE},
    {0,"",0}
    };

void dclinquire_read_sysinput(char *buf,long timeout,int length);

int dcl_inquire(PARAM_T *p,PARAM_T *q)
{
    char    name[SYMBOL_MAX_NAME];
    char    value[SYMBOL_MAX_VALUE];
    char    prompt[MAX_TOKEN];
    int     i       = 0;
    int     retcod  = DCL_OK;
    int     f_punct = 1;
    int     f_local = 0;
    int     f_global= 0;
    int     lvl     = 0;
    long    timeout = 0L;
    char    delta[MAX_TOKEN];
    time_t  time_l;
    char    work[MAX_TOKEN];
    char    temp[MAX_TOKEN];
    char    *w;
    char    *ch;
    char    line = 0;
    char    column = 0;
    int     length = 0;;

    NEXT_LINE();
    if (cmd[C].subr) return(DCL_OK);
    if (!dcl[D].cc[dcl[D].ccl]) return(DCL_OK);

    if (p == NULL || q == NULL) return(DCL_ERROR);

    _STATUS = 0;
    *name = 0; *value = 0; *prompt = 0;
    time_l = 0;
    retcod = cmd_parse_line(dcl_line,INQUIRE_PARAM,INQUIRE_QUAL,p,q);

    if (retcod == DCL_OK) {
        for (i = 0; q[i].tag; i++) {
            if (q[i].flag & PRESENT)
                switch (q[i].tag) {
                    case 1:                                 /* /LOCAL   */
                        f_local  = TRUE;
                        f_global = FALSE;
                        break;
                    case 2:                                 /* /GLOBAL   */
                        f_local  = FALSE;
                        f_global = TRUE;
                        break;
                    case 3:                                 /* /PUNCT    */
                        f_punct  = TRUE;
                        break;
                    case 4:                                 /* /NOPUNCT  */
                        f_punct  = FALSE;
                        break;
                    case 5:                                 /* /TIMEOUT  */
                        dcl_string(q[i].value,delta,MAX_TOKEN);
                        tm_str_to_delta(delta,&time_l);
                        timeout = (long) time_l;
                        break;
                    case 6:                                 /* /NOTIMEOUT*/
                        timeout = 0L;
                        break;
                    case 7:                                 /*  /POSITION  */
                        *work = 0;
                        w = work;
                        ch = q[i].value;
                        while (*ch && (*ch == '(' || isspace((int)*ch))) ch++;
                        while (*ch && *ch != ',' && !isspace((int)*ch))
                            *w++ = *ch++;
                        *w = 0;
                        (void) EXP_compute(work,temp);
                        line = (char) atoi(temp);
                        if (line == 0) {
                            (void) dcl_printf(dcl[D].SYS_OUTPUT,"Invalid LINE POSITION qualifier\n");
                            _SEVERITY = 2;
                            _STATUS = 19;
                            retcod = DCL_ERROR;
                        }
                        line--;
                        *work = 0;
                        w = work;
                        while (*ch && (*ch == ',' || isspace((int)*ch))) ch++;
                        while (*ch && *ch != ')' && !isspace((int)*ch))
                            *w++ = *ch++;
                        *w = 0;
                        (void) EXP_compute(work,temp);
                        column = (char) atoi(temp);
                        if (column == 0) {
                            (void) dcl_printf(dcl[D].SYS_OUTPUT,"Invalid COLUMN POSITION qualifier\n");
                            _SEVERITY = 2;
                            _STATUS = 19;
                            retcod = DCL_ERROR;
                        }
                        column--;
                        break;
                    case 8:                                /*  /LENGTH    */
                        length = atoi(q[i].value);
                        if (length == 0) {
                            (void) dcl_printf(dcl[D].SYS_OUTPUT,"WARNING: Invalid /LENGTH qualifier.\n");
                            _SEVERITY = _WARNING;
                            _STATUS = 19;
                            }
                        break;
                    default:
                        ;
                    } /* end switch */
            }   /* end for */

        dcl_string(p[0].value,name,SYMBOL_MAX_NAME);
        strcpy(prompt,p[1].value);
        if (f_local && f_global) {
            (void) dcl_printf(dcl[D].SYS_OUTPUT,"/LOCAL and /GLOBAL parameters are mutually exclusives.\n");
            _SEVERITY = 2;
            _STATUS = 19;
            retcod = DCL_ERROR;
            goto exit_label;
            }
        if (!(f_local | f_global))
            f_local = 1;
        if (line | column)
            (void) tio_gotoxy(column,line);
        if (*prompt)
	        (void) dcl_printf(dcl[D].SYS_OUTPUT,"%s",prompt);
        else
	        (void) dcl_printf(dcl[D].SYS_OUTPUT,"%s",name);
        if (f_punct)
            (void) dcl_printf(dcl[D].SYS_OUTPUT,": ");
        dclinquire_read_sysinput(value,timeout,length);
        if (f_global)
            lvl = 0;
        else
            lvl = D;
        (void) symbol_put(name,value,lvl);
        }


exit_label:
    return(retcod);
}


void dclinquire_read_sysinput(char *buf,long timeout,int length)
{
    (void) memset(buf,0,MAX_TOKEN);
    if (length == 0 || length > MAX_TOKEN)
        length = MAX_TOKEN;
    (void) kbdread(buf,length,data_stack,timeout);
}
