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
#include "platform.h"
#include "dcl.h"

// Allocate -----------------------------------------------------
RPARAM_T ALLOCATE_PARAM[] = {
    {1,"Device",MANDATORY},
    {2,"Logical name",MANDATORY},
    {0,"",0}
    };
RPARAM_T ALLOCATE_QUAL[] = {
    {1,"/LOG",0},
    {2,"/NOLOG",0},
    {0,"",0}
    };

int dcl_allocate(PARAM_T *p,PARAM_T *q)
{
    char    token[MAX_TOKEN];
    char    devnam[LOGICAL_MAX_VALUE];
    char    lognam[LOGICAL_MAX_NAME];
    int     i       = 0;
    int     log     = 1;
    int     retcod  = DCL_OK;

    NEXT_LINE();
    if (cmd[C].subr) return(DCL_OK);
    if (!dcl[D].cc[dcl[D].ccl]) return(DCL_OK);
    if (p == NULL || q == NULL) return(DCL_ERROR);
    

    *token = 0; *devnam = 0; *lognam = 0;

    retcod = cmd_parse_line(dcl_line,ALLOCATE_PARAM,ALLOCATE_QUAL,p,q);

    if (retcod == DCL_OK) {
        for (i = 0; q[i].tag; i++) {
            if (q[i].flag & PRESENT) {
                switch (q[i].tag) {
                case 1:                                 /* /LOG     */
                    log = TRUE;
                    break;
                case 2:                                 /* /NOLOG   */
                    log = FALSE;
                    break;
                default:
                    ;
                } /* end switch */
            } /* end if */
        }   /* end for */

        dcl_string(p[0].value,devnam,MAX_TOKEN);
        dcl_string(p[1].value,lognam,MAX_TOKEN);

        if (lognam[strlen(lognam)-1] == ':')
            lognam[strlen(lognam)-1] = 0;
        (void) logical_get(lognam,token);

        retcod = logical_put(lognam,devnam,LOG_ALLOC);
        if (log && *token && retcod == 0) {
            (void) dcl_printf(dcl[D].SYS_OUTPUT,"%s superseded\n",lognam);
        }
        if (log && retcod == 0) {
            (void) dcl_printf(dcl[D].SYS_OUTPUT,"%s allocated as %s.\n",devnam,lognam);
        }
    } /* retcod == DCL_OK */

    return(retcod);
}
