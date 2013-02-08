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

// Run -----------------------------------------------------
RPARAM_T RUN_PARAM[] = {
    {1,"Command",EXPR},
    {0,"",0}
    };

RPARAM_T RUN_QUAL[] = {
    {1,"/WAIT",0},
    {2,"/NOWAIT",0},
    {0,"",0}
    };

int dcl_run(PARAM_T *p, PARAM_T *q)
{
    char    buffer[MAX_TOKEN];
    int     retcod;
    int     i;
    int     nowait = 0;

    NEXT_LINE();
    if (cmd[C].subr) return(DCL_OK);
    if (!dcl[D].cc[dcl[D].ccl]) return(DCL_OK);
    if (p == NULL || q == NULL) return(DCL_ERROR);

    retcod = cmd_parse_line(dcl_line,RUN_PARAM,RUN_QUAL,p,q);
    for (i = 0; q[i].tag; i++) {
        if (q[i].flag & PRESENT) 
            switch (q[i].tag) {
            case 1:                                 /* /WAIT    */
                nowait = 0;
                break;
            case 2:                                 /* /NOWAIT   */
                nowait = 1;
                break;
            default:
                ;
            } /* end switch */
    }   /* end for */
    if (retcod == DCL_OK) {
		if (p[0].expr != NULL) {
			strcpy(buffer,p[0].expr);
			if (dcl[D].SYS_OUTPUT != stdout) {
				strcat(buffer," >>");
				strcat(buffer,dcl[D].outname);
			}
			retcod = dcl_spawn(buffer, nowait);
		}
    }
    return(retcod);
}

