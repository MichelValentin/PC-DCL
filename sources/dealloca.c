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

#include "platform.h"
#include "dcl.h"

// Deallocate -----------------------------------------------------
RPARAM_T DEALLOCATE_PARAM[] = {
    {2,"Device name",MANDATORY},
    {0,"",0}
    };
RPARAM_T DEALLOCATE_QUAL[] = {
    {1,"/ALL",0},
    {0,"",0}
    };

int dcl_deallocate(PARAM_T *p,PARAM_T *q)
{
    LOGICAL tmp;
    char    name[LOGICAL_MAX_NAME];
    int     i       = 0;
    int     all     = 0;
    int     retcod  = DCL_OK;


    NEXT_LINE();
    if (cmd[C].subr) return(DCL_OK);
    if (!dcl[D].cc[dcl[D].ccl]) return(DCL_OK);

    _STATUS = 0;
    *name = 0;
    retcod = cmd_parse_line(dcl_line,DEALLOCATE_PARAM,DEALLOCATE_QUAL,p,q);

    if (retcod == DCL_OK) {
        for (i = 0; q[i].tag; i++) {
            if (q[i].flag & PRESENT)
                switch (q[i].tag) {
                    case 1:                                 /* /ALL     */
                        all = TRUE;
                        break;
                    default:
                        ;
                    } /* end switch */
            }   /* end for */

        dcl_string(p[0].value,name,LOGICAL_MAX_NAME);
        if (name[strlen(name)-1] == ':')
            name[strlen(name)-1] = 0;
        if (*name && all == 1) {
            (void)dcl_printf(dcl[D].SYS_OUTPUT,"/ALL and Logical name are mutually exclusive.\n");
            _SEVERITY = 2;
            _STATUS = 19;
            retcod = DCL_ERROR;
            goto exit_label;
            }

        if (all) {
            for (i = 0; i < LOGICAL_MAX; i++) {
                if (LOGICAL_TABLE[i].flag != 0) {
                    (void)logical_del(tmp.name);
                }
            }
        }
        else {
            if (strcasecmp(name,"SYS$OUTPUT") == 0) {
                //(void)fclose(dcl[D].SYS_OUTPUT);
                dcl[D].SYS_OUTPUT = stdout;
                strcpy(dcl[D].outname, name);
        		logical_put(name,name,LOG_SYSTEM);
            }
            else {
            	(void)logical_del(name);
            }
        }
    }

exit_label:
    return(retcod);
}

