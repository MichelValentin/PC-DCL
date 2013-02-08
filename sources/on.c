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

#include "platform.h"
#include "dcl.h"

// On -----------------------------------------------------
RPARAM_T ON_PARAM[] = {
    {1,"Severity",MANDATORY},
    {2,"Then",0},
    {3,"Command",EXPR},
    {0,"",0}
    };

int dcl_on(PARAM_T *p,PARAM_T *q)
{
    char    *severity_q[7] = {"SUCCESS","WARNING","ERROR","SEVERE_ERROR","CONTROL_C","CONTROL_Y",NULL};
    int     severity;
    int     retcod = 0;

    NEXT_LINE();
    if (cmd[C].subr) {
        return(0);
    }

    if (p == NULL || q == NULL) return(DCL_ERROR);

    _STATUS = 0;
    (void) cmd_parse_line(dcl_line,ON_PARAM,DUMMY_QUAL,p,q);
    severity = dcl_search(severity_q,p[0].value);
    if (severity < 1 || severity > 5) {
        (void) dcl_printf(dcl[D].SYS_OUTPUT,"Invalid command parameters\n");
        _SEVERITY = 2;
        _STATUS = 19;
        retcod = -1;
        goto exit_label;
        }

    if (strcasecmp(p[1].value,"THEN")!=0){
        (void) dcl_printf(dcl[D].SYS_OUTPUT,"Invalid command parameters\n");
        _SEVERITY = 2;
        _STATUS = 19;
        retcod = -1;
        goto exit_label;
        }
    
    if (severity > 3)
        strcpy(dcl[D].ON_break_command,p[2].expr);
    else {
        dcl[D].ON_severity = (char) severity;
        strcpy(dcl[D].ON_error_command,p[2].expr);
        }

exit_label:
    return(retcod);
}
