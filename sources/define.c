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
/*lint -e818 * could be declared as constant*/
#include <string.h>
#include <stdlib.h>

#include "platform.h"
#include "dcl.h"

// define -----------------------------------------------------
RPARAM_T DEFINE_PARAM[] = {
    {1,"Name",MANDATORY},
    {2,"Value",MANDATORY},
    {0,"",0}
    };
RPARAM_T DEFINE_QUAL[] = {
    {1,"/KEY",0},
    {2,"/ECHO",0},
    {3,"/NOECHO",0},
    {4,"/ERASE",0},
    {5,"/NOERASE",0},
    {6,"/LOG",0},
    {7,"/NOLOG",0},
    {8,"/TERMINATE",0},
    {9,"/NOTERMINATE",0},
    {0,"",0}
    };

int dcl_define_logical(PARAM_T *p,PARAM_T *q);
int dcl_define_key(PARAM_T *p,PARAM_T *q);

int dcl_define(PARAM_T *p,PARAM_T *q)
{
    int     retcod;
    int     key = 0;
    int     i;

    NEXT_LINE();
    if (cmd[C].subr) return(DCL_OK);
    if (!dcl[D].cc[dcl[D].ccl]) return(DCL_OK);

    _STATUS = 0;
    retcod = cmd_parse_line(dcl_line,DEFINE_PARAM,DEFINE_QUAL,p,q);
    if (retcod == DCL_OK) {
        for (i = 0; q[i].tag; i++) {
            if (q[i].flag & PRESENT) 
                if (q[i].tag == 1) {
                    key = 1;
                    } 
            }   /* end for */

        if (key)
            retcod = dcl_define_key(p,q);
        else
            retcod = dcl_define_logical(p,q);
        }

    return(retcod);
}

//=====================================================================
int dcl_define_logical(PARAM_T *p,PARAM_T *q)
{
    char    token[MAX_TOKEN];
    char    value[LOGICAL_MAX_VALUE];
    char    name[LOGICAL_MAX_NAME];
    char    dosfile[_MAX_PATH];
    char    drive[_MAX_DRIVE];
    char    dir[_MAX_DIR];
    char    fname[_MAX_FNAME];
    char    ext[_MAX_EXT];
    int     i       = 0;
    int     log     = 1;
    int     retcod  = DCL_OK;

    *token = 0; *value = 0; *name = 0;

    for (i = 0; q[i].tag; i++) {
        if (q[i].flag & PRESENT) 
            switch (q[i].tag) {
            case 6:                                 /* /LOG     */
                log = TRUE;
                break;
            case 7:                                 /* /NOLOG   */
                log = FALSE;
                break;
            default:
                (void)dcl_printf(dcl[D].SYS_OUTPUT,"Invalid command qualifier\n");
                _SEVERITY = 2;
                _STATUS = 19;
                retcod = DCL_ERROR;
            } /* end switch */
    }   /* end for */

    if (retcod == DCL_OK) {
        dcl_string(p[0].value,name,MAX_TOKEN);
        dcl_string(p[1].value,value,MAX_TOKEN);
        (void)logical_get(name,token);
        retcod = logical_put(name,value,LOG_USER);
        if (log && *token && retcod == 0) {
            (void)dcl_printf(dcl[D].SYS_OUTPUT,"%s superseded\n",name);
        }
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

    return(retcod);
}

//=====================================================================
int dcl_define_key(PARAM_T *p,PARAM_T *q)
{
    char    *keys[25] = {"F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","F11","F12",
                         "SF1","SF2","SF3","SF4","SF5","SF6","SF7","SF8","SF9","SF10",
                         "SF11","SF12",NULL};
    char    value[LOGICAL_MAX_VALUE];
    char    name[LOGICAL_MAX_NAME];
    int     i       = 0;
    int     log     = 1;
    int     echo    = 1;
    int     erase   = 0;
    int     terminate = 0;
    int     retcod = DCL_OK;


    *value = 0; *name = 0;

    for (i = 0; q[i].tag; i++) {
        if (q[i].flag & PRESENT) 
            switch (q[i].tag) {
            case 1:                                 /* /KEY     */
                break;
            case 2:                                 /* /ECHO    */
                echo = TRUE;
                break;
            case 3:                                 /* /NOECHO  */
                echo = FALSE;
                break;
            case 4:                                 /* /ERASE    */
                erase = TRUE;
                break;
            case 5:                                 /* /NOERASE  */
                erase = FALSE;
                break;
            case 6:                                 /* /LOG     */
                log = TRUE;
                break;
            case 7:                                 /* /NOLOG   */
                log = FALSE;
                break;
            case 8:                                 /* /TERMINATE*/
                terminate = TRUE;
                break;
            case 9:                                 /* /NOTERM   */
                terminate = FALSE;
                break;
            default:
                ;
            } /* end switch */
    }   /* end for */

    dcl_string(p[0].value,name,MAX_TOKEN);
    dcl_string(p[1].value,value,MAX_TOKEN);

    if (!(echo + terminate)) {
        (void)dcl_printf(dcl[D].SYS_OUTPUT,"/NOECHO and /NOTERMINATE are mutually exclusive.\n");
        _SEVERITY = 2;
        _STATUS = 19;
        retcod = DCL_ERROR;
    }

    if (retcod == DCL_OK) {
        i = dcl_search_key(keys,name);
        if (i < 0) {
            (void)dcl_printf(dcl[D].SYS_OUTPUT,"Invalid KEY parameter\n");
            _SEVERITY = 2;
            _STATUS = 19;
            retcod = DCL_ERROR;
        }
        else {
            if (keydef[i].value) {
                free(keydef[i].value);
                keydef[i].value = NULL;
                if (log) {
                    (void)dcl_printf(dcl[D].SYS_OUTPUT,"Key definition %s deleted.\n",keys[i]);
                }
            }
            keydef[i].value = (char *) calloc(1,strlen(value)+1);
            if (keydef[i].value == NULL){
                (void)dcl_printf(dcl[D].SYS_OUTPUT,"Cannot allocate memory for KEY value\n");
                _SEVERITY = 2;
                _STATUS = 19;
                retcod = DCL_ERROR;
            }
            else {
                strcpy(keydef[i].value,value);
                keydef[i].echo = (char)echo;
                keydef[i].erase = (char)erase;
                keydef[i].terminate = (char)terminate;
                if (log) {
                    (void)dcl_printf(dcl[D].SYS_OUTPUT,"Key definition %s created.\n",keys[i]);
                }
            }
        }
    }
    return(retcod);
}
