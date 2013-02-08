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
/*lint -e801 use of goto is deprecated*/
#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "dcl.h"

#define MAX_WHILE 8
static int  while_table[MAX_WHILE];
static int  while_idx = 0;
static int  while_flag = 0;


int dcl_while(PARAM_T *p,PARAM_T *q)
{
    char    token[MAX_TOKEN];
    char    result[MAX_TOKEN];
    char    *ch;
    char    *s1;
    int     cci;
    int     retcod = DCL_OK;

    if (p == NULL) {
        _SEVERITY = 2;
        return(DCL_ERROR);
    }

    if (cmd[C].subr) return(DCL_OK);

    if (!dcl[D].cc[dcl[D].ccl]) {    //if already in negative condition...
        if (dcl[D].ccl < 8) {
            dcl[D].ccl++;
            dcl[D].cc[dcl[D].ccl] = 0;
            }
        else
            {
            (void)dcl_printf(dcl[D].SYS_OUTPUT,"ERROR - Too much nested IF conditions\n");
            retcod = DCL_ERROR;
            _SEVERITY = 2;
            }
        goto exit_label;
        }

    *token = 0; *result = 0;
    s1 = token;
    retcod = cmd_parse_line(dcl_line,IF_PARAM,DUMMY_QUAL,p,q);
    if (retcod == DCL_OK) {
        ch = p[0].expr;
        while (*ch && strncasecmp(ch,"DO",2))
            *s1++ = *ch++;
        *s1 = 0;
        (void)EXP_compute(token,result);
        cci = atoi(result) ? 1 : 0;

        ch = dcl_get_token(ch,token);   /* DO */

        if (cci) {
            if (while_flag == 0) {
                if (while_idx < MAX_WHILE) {
                    while_idx++;
                    while_table[while_idx] = cmd[C].ln;
                    }
                }
            }
        else {
            while_idx--;
            }

        if (dcl[D].ccl < 8) {
            dcl[D].ccl++;
            dcl[D].cc[dcl[D].ccl] = (char)cci;
            }
        else
            {
            (void)dcl_printf(dcl[D].SYS_OUTPUT,"ERROR - Too much nested IF conditions\n");
            retcod = DCL_ERROR;
            _SEVERITY = 2;
            }
        }
exit_label:
    NEXT_LINE();
    while_flag = 0;
    return(retcod);
}

int dcl_endwhile(PARAM_T *p,PARAM_T *q)
{
    UNREFERENCED_PARAM(p);
    UNREFERENCED_PARAM(q);

    NEXT_LINE();
    if (cmd[C].subr) return(DCL_OK);
    if (dcl[D].ccl) {
        if (dcl[D].cc[dcl[D].ccl]) {       // if cond code OK loop
            cmd[C].ln = while_table[while_idx];
            while_flag = 1;
            }
        dcl[D].cc[dcl[D].ccl] = 1;         // pop condition level
        dcl[D].ccl--;
        }
    else {
        (void)dcl_printf(dcl[D].SYS_OUTPUT,"invalid use of ENDWHILE statement.\n");
        _SEVERITY = 2;
        return(DCL_ERROR);
        }
    return(DCL_OK);
}

