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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "platform.h"
#include "dcl.h"


// Call -----------------------------------------------------
RPARAM_T CALL_PARAM[] = {
    {1,"Subroutine",MANDATORY},
    {2,"P1",0},
    {2,"P2",0},
    {2,"P3",0},
    {2,"P4",0},
    {2,"P5",0},
    {2,"P6",0},
    {2,"P7",0},
    {2,"P8",0},
    {0,"",0}
    };
RPARAM_T CALL_QUAL[] = {
    {4,"/OUTPUT",VALUE},
    {0,"",0}
    };

int dcl_call(PARAM_T *p,PARAM_T *q)
{
    char            label[MAX_LABEL];
    char            outfile[MAX_TOKEN];
    CMD_LABEL       LABEL;
    char            *parm[8] = {NULL};
    int             l       = 0;
    unsigned int    i       = 0;
    int             found   = 0;
    char            subr    = 0;
    int             retcod  = DCL_OK;

    if (cmd[C].subr) {
        NEXT_LINE();
        return(DCL_OK);
    }
    if (!dcl[D].cc[dcl[D].ccl]) {
        NEXT_LINE();
        return(DCL_OK);
    }
    
    if (C <= 1) {
        (void) dcl_printf(dcl[D].SYS_OUTPUT,"Invalid use of CALL command.\n");
        _SEVERITY = 2;
        _STATUS = 19;
        return(DCL_ERROR);
    }

    if (p == NULL || q == NULL) return(DCL_ERROR);

    *label = 0; *outfile = 0;

    retcod = cmd_parse_line(dcl_line,CALL_PARAM,CALL_QUAL,p,q);
    
    if (retcod == DCL_OK) {
        if (q[0].flag & PRESENT) {
            dcl_string(q[0].value,outfile,MAX_TOKEN);
            if (*outfile == 0){
                NEXT_LINE();
                (void) dcl_printf(dcl[D].SYS_OUTPUT,"Invalid argument\n");
                _SEVERITY = 2;
                _STATUS = 19;
                retcod = DCL_ERROR;
                goto exit_label;
            }
        }

        dcl_string(p[0].value,label,MAX_LABEL-1);
        strcat(label,":");
        while (!found && i <= nempty(cmd[C].label)) {
            if (rcld(cmd[C].label,&LABEL,i)) {
                if (strcasecmp(label,LABEL.name) == 0) {
                    l = LABEL.line;
                    subr = LABEL.subr;
                    found = 1;
                }
            }
            i++;
        }
    
        if (!found) {
            NEXT_LINE();
            (void) dcl_printf(dcl[D].SYS_OUTPUT,"CALL target not found.\n");
            _SEVERITY = 2;
            _STATUS = 19;
            retcod = DCL_ERROR;
            goto exit_label;
        }
    
        if (!subr) {
            NEXT_LINE();
            (void) dcl_printf(dcl[D].SYS_OUTPUT,"CALL target not a subroutine.\n");
            _SEVERITY = 2;
            _STATUS = 19;
            retcod = DCL_ERROR;
            goto exit_label;
        }

        for (i = 0; i < 8; i++) {
            parm[i] = (char *) malloc(MAX_TOKEN);
            if (parm[i] == NULL) {
                NEXT_LINE();
                (void) dcl_printf(dcl[D].SYS_OUTPUT,"ERROR - CA0002\n");
                _SEVERITY = 4;
                _STATUS = 8;
                retcod = DCL_ERROR;
                goto exit_label;
                }
            *parm[i] = 0;
        }

        for (i = 1; i <= 8; i++)
            if (p[i].flag & PRESENT)
                strcpy(parm[i-1],p[i].value);

        i = (unsigned)cmd[C].ln + 1;
        (void) pushdptr(cmd[C].stack,&i);
        cmd[C].ln = l;
        dcl_subr++;
        retcod = dcl_enter_proc_level(outfile,parm);
    }

exit_label:
    for (l = 0;l < 8; l++)
        if (parm[l] != NULL) free(parm[l]);
    return(retcod);
}
