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

#include <string.h>
#include <stdlib.h>

#include "platform.h"
#include "dcl.h"

// If -----------------------------------------------------
RPARAM_T IF_PARAM[] = {
    {1,"Condition",EXPR},
    {0,"",0}
    };

int dcl_if(PARAM_T *p,PARAM_T *q)
{
    char    token[MAX_TOKEN];
    char    result[MAX_TOKEN];
    char    *ch;
    char    *s1;
    int     cci;
    int     retcod = DCL_OK;

    NEXT_LINE();
    if (cmd[C].subr) return(DCL_OK);

    if (p == NULL || q == NULL) return(DCL_ERROR);

    if (!dcl[D].cc[dcl[D].ccl]) {
        if (dcl[D].ccl < 8) {
            dcl[D].ccl++;
            dcl[D].cc[dcl[D].ccl] = 0;
            }
        else
            {
            (void) dcl_printf(dcl[D].SYS_OUTPUT,"ERROR - Too much nested IF conditions\n");
            retcod = DCL_ERROR;
            _SEVERITY = 2;
            }
        return (DCL_OK);
        }

    *token = 0; *result = 0;
    s1 = token;
    retcod = cmd_parse_line(dcl_line,IF_PARAM,DUMMY_QUAL,p,q);
    if (retcod == DCL_OK) {
        ch = p[0].expr;
        while (*ch && strncasecmp(ch,"THEN",4))
            *s1++ = *ch++;
        *s1 = 0;
        (void) EXP_compute(token,result);
        cci = atoi(result) ? 1 : 0;

        ch = dcl_get_token(ch,token);   /* THEN */
        if (*ch) {
            if (strncasecmp(token,"THEN",4)) {
                (void) dcl_printf(dcl[D].SYS_OUTPUT,"ERROR - Unexpected token\n");
                retcod = DCL_ERROR;
                _SEVERITY = 2;
            }
            (void) dcl_get_token(ch,token);
        }

        if (*token){
            if (cci) {
                strcpy(dcl_line,ch);
                (void) dcl_process_command_line(2);
                }
            }
        else {
            if (dcl[D].ccl < 8) {
                dcl[D].ccl++;
                dcl[D].cc[dcl[D].ccl] = (char)cci;
                }
            else
                {
                (void) dcl_printf(dcl[D].SYS_OUTPUT,"ERROR - Too much nested IF conditions\n");
                retcod = DCL_ERROR;
                _SEVERITY = 2;
                }
            }
        }
    return(retcod);
}
