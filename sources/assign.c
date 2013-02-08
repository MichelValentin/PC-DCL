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

// Assign -----------------------------------------------------
RPARAM_T ASSIGN_PARAM[] = {
    {1,"Value",MANDATORY},
    {2,"Logical name",MANDATORY},
    {0,"",0}
    };
RPARAM_T ASSIGN_QUAL[] = {
    {1,"/LOG",0},
    {2,"/NOLOG",0},
    {0,"",0}
    };

int dcl_assign(PARAM_T *p,PARAM_T *q)
{
	char    token[MAX_TOKEN];
    char    value[LOGICAL_MAX_VALUE];
    char    name[LOGICAL_MAX_NAME];
    char    dosfile[_MAX_PATH];
    char    drive[_MAX_DRIVE];
    char    dir[_MAX_DIR];
    char    fname[_MAX_FNAME];
    char    ext[_MAX_EXT];
    int     i = 0;
    int     log = 0;
    int     retcod = DCL_OK;


    NEXT_LINE();
    if (cmd[C].subr) return(DCL_OK);
    if (!dcl[D].cc[dcl[D].ccl]) return(DCL_OK);

    *token = 0; *value = 0; *name = 0;

    retcod = cmd_parse_line(dcl_line,ASSIGN_PARAM,ASSIGN_QUAL,p,q);

    if (retcod == DCL_OK) {
        for (i = 0; q[i].tag; i++) {
            if (q[i].flag & PRESENT) 
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
            }   /* end for */

        dcl_string(p[0].value,value,MAX_TOKEN);
        dcl_string(p[1].value,name,MAX_TOKEN);

	    (void)logical_get(name,token);

        retcod = logical_put(name,value,LOG_USER);
        if (log && *token && !retcod) {
            (void)dcl_printf(dcl[D].SYS_OUTPUT,"%s superseded\n",name);
        }
        if (retcod == DCL_OK) {
            if (strcasecmp(name,"SYS$OUTPUT") == 0) {
                if (dcl[D].SYS_OUTPUT != stdout) {
                    //(void)fclose(dcl[D].SYS_OUTPUT);
                }
                if (strcasecmp(value, "tt") == 0) {
                    dcl[D].SYS_OUTPUT = stdout;
                    strcpy(dcl[D].outname, name);
                }
                else {
                	cvfs_vms_to_dos(name,dosfile,&i);
	                _splitpath(dosfile,drive,dir,fname,ext);
	                if (strlen(ext) == 0) strcat(dosfile,".lis");
	                dcl[D].SYS_OUTPUT = fopen(dosfile,"at");
	                if (dcl[D].SYS_OUTPUT == NULL) {
	                    dcl[D].SYS_OUTPUT = stdout;
                        strcpy(dcl[D].outname, "SYS$OUTPUT");
	                    }
	                else {
                        strncpy(dcl[D].outname,dosfile,_MAX_PATH);
	                }
            	}
            }
        }
    }
    return(retcod);
}

