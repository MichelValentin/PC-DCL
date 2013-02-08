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
#include "platform.h"
#include "dcl.h"

#define MAX_REPEAT 8
static int repeat_table[MAX_REPEAT];
static int repeat_idx = 0;

int dcl_repeat(PARAM_T *p,PARAM_T *q)
{
    int     retcod = DCL_OK;

    UNREFERENCED_PARAM(p);
    UNREFERENCED_PARAM(q);

    NEXT_LINE();
    if (cmd[C].subr) return(DCL_OK);
    if (!dcl[D].cc[dcl[D].ccl]) return(0);

    if (repeat_idx < MAX_REPEAT) {
        repeat_idx++;
        repeat_table[repeat_idx] = cmd[C].ln;
    }
    else {
        (void)dcl_printf(dcl[D].SYS_OUTPUT,"Too much nested REPEAT statements.\n");
        _SEVERITY = 2;
        retcod = DCL_ERROR;
    }

    return(retcod);
}

int dcl_until(PARAM_T *p,PARAM_T *q)
{
    int     cci;
    int     retcod = DCL_OK;
    char    result[MAX_TOKEN];

    NEXT_LINE();
    if (cmd[C].subr) return(DCL_OK);
    if (!dcl[D].cc[dcl[D].ccl]) return(0);
    if (p == NULL || q == NULL) return(DCL_ERROR);

    retcod = cmd_parse_line(dcl_line,IF_PARAM,DUMMY_QUAL,p,q);
    if (retcod == DCL_OK) {
        (void)EXP_compute(p[0].expr,result);
        cci = atoi(result) ? 1 : 0;
        if (cci == 0) {
            cmd[C].ln = repeat_table[repeat_idx];
            }
        else {
            repeat_idx--;
            }
        }
    return(retcod);
}
