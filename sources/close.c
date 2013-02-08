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

//#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "platform.h"
#include "dcl.h"

// Close -----------------------------------------------------
RPARAM_T CLOSE_PARAM[] = {
    {1,"File",MANDATORY},
    {0,"",0}
    };
RPARAM_T CLOSE_QUAL[] = {
    {1,"/LOG",0},
    {2,"/NOLOG",0},
    {3,"/ERROR",VALUE},
    {4,"/DISPOSITION",VALUE},
    {0,"",0}
    };

int dcl_close(PARAM_T *p,PARAM_T *q)
{
    char    token[MAX_TOKEN];
    char    lognam[LOGICAL_MAX_NAME];
    char    err_label[MAX_LABEL];
    char    disposition[MAX_TOKEN];
    int     i = 0;
    int     filenum;
    char    fmode;
    int     recl;
    long    rptr;
    long    wptr;
    int     retcod = DCL_OK;
    int     log = 0;
    int     delete = 0;


    NEXT_LINE();
    if (cmd[C].subr) return(DCL_OK);
    if (!dcl[D].cc[dcl[D].ccl]) return(DCL_OK);
    if (p == NULL || q == NULL) return(DCL_ERROR);

    *token = 0; *lognam = 0; *err_label = 0;
    _STATUS = 0;
    retcod = cmd_parse_line(dcl_line,CLOSE_PARAM,CLOSE_QUAL,p,q);
    
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
                case 3:                                 /* /ERROR=  */
                    dcl_string(q[i].value,err_label,MAX_LABEL);
                    if (*err_label == 0){
                        NEXT_LINE();
                        (void)dcl_printf(dcl[D].SYS_OUTPUT,"Invalid argument\n");
                        _SEVERITY = 2;
                        _STATUS = 19;
                        retcod = DCL_ERROR;
                        goto exit_label;
                        }
                    break;
                case 4:                                 /* /DISPOSITION=  */
                    dcl_string(q[i].value,disposition,MAX_TOKEN);
                    if (*disposition != 0){
                        if (strncasecmp("DELETE", disposition, strlen(disposition)) == 0) {
                            delete = 1;
                        }
                        else if (strncasecmp("KEEP", disposition, strlen(disposition)) == 0) {
                            delete = 0;
                        }
                        else if (strncasecmp("PRINT", disposition, strlen(disposition)) == 0) {
                            delete = 0;
                        }
                        else if (strncasecmp("SUBMIT", disposition, strlen(disposition)) == 0) {
                            delete = 0;
                        }
                        else {
                            NEXT_LINE();
                            (void)dcl_printf(dcl[D].SYS_OUTPUT,"Invalid argument\n");
                            _SEVERITY = 2;
                            _STATUS = 19;
                            retcod = DCL_ERROR;
                            goto exit_label;
                            }                         
                    }
                    break;
                default:
                    ;
                } /* end switch */
        }   /* end for */

        dcl_string(p[0].value,lognam,LOGICAL_MAX_NAME);
        if (lognam[strlen(lognam)-1] == ':')
            lognam[strlen(lognam)-1] = 0;
        filenum = -1,
        logical_get_file(lognam,token,&filenum,&fmode,&recl,&rptr,&wptr);

        if (filenum != -1 && dclfile[filenum] != NULL) {
            if (fclose(dclfile[filenum]) != 0) {
                errno = 2;
                _STATUS = errno;
                _SEVERITY = 2;
                if (*err_label) {
                    sprintf(dcl_line,"GOTO %s",err_label);
                    (void)dcl_process_command_line(0);
                    }
                else {
                    if (log) {
                        (void)dcl_printf(dcl[D].SYS_OUTPUT,"Error closing %s\n",lognam);
                        (void)dcl_printf(dcl[D].SYS_OUTPUT,"Reason : %s\n",strerror(errno));
                    }
                }
                retcod = DCL_ERROR;
                goto exit_label;
            }
        }

        if (delete) {
            if (unlink(token) != 0) {
                _STATUS = errno;
                _SEVERITY = 2;
                if (*err_label) {
                    sprintf(dcl_line,"GOTO %s",err_label);
                    (void)dcl_process_command_line(0);
                    }
                else {
                    if (log) {
                        (void)dcl_printf(dcl[D].SYS_OUTPUT,"Error deleting %s\n",lognam);
                        (void)dcl_printf(dcl[D].SYS_OUTPUT,"Reason : %s\n",strerror(errno));
                    }
                }
                retcod = DCL_ERROR;
                goto exit_label;
            }
        }
        
        (void)logical_del(lognam);
        if (filenum != -1)
            dclfile[filenum] = NULL;
    }

exit_label:
    return(retcod);
}

