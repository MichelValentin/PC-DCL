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
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "platform.h"
#include "dcl.h"
//#include <direct.h>

// Create -----------------------------------------------------
RPARAM_T CREATE_PARAM[] = {
    {1,"File",MANDATORY},
    {0,"",0}
    };
RPARAM_T CREATE_QUAL[] = {
    {1,"/DIRECTORY",0},
    {2,"/LOG",0},
    {3,"/NOLOG",0},
    {0,"",0}
    };

void dclcreate_read_sysinput(char *buf);

int dcl_create(PARAM_T *p,PARAM_T *q)
{
    char token[MAX_TOKEN];
    char vms[MAX_TOKEN];
    char dos[MAX_TOKEN];
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char file[_MAX_FNAME];
    char ext[_MAX_EXT];
    FILE    *fp;
    int     i       = 0;
    int     retcod  = DCL_OK;
    int     f_log   = 0;
    int     f_dir   = 0;

    NEXT_LINE();
    if (cmd[C].subr) return(DCL_OK);
    if (!dcl[D].cc[dcl[D].ccl]) return(DCL_OK);

    _STATUS = 0;
    *token  = 0; *vms    = 0; *dos    = 0;
    *drive  = 0; *dir    = 0; *file   = 0; *ext    = 0;

    retcod = cmd_parse_line(dcl_line,CREATE_PARAM,CREATE_QUAL,p,q);

    if (retcod == DCL_OK) {
        for (i = 0; q[i].tag; i++) {
            if (q[i].flag & PRESENT) 
                switch (q[i].tag) {
                    case 1:                                 /* /DIR     */
                        f_dir = TRUE;
                        break;
                    case 2:                                 /*  /LOG */
                        f_log = TRUE;
                        break;
                    case 3:                                 /*  /NOLOG */
                        f_log = FALSE;
                        break;
                    default:
                        ;
                    } /* end switch */
            }   /* end for */

        dcl_string(p[0].value,vms,MAX_TOKEN);
        cvfs_vms_to_dos(vms,dos,&i);
        if (f_dir) {
            if (dos[strlen(dos)-1] == SLASH_CHR)
                dos[strlen(dos)-1] = 0;
            if (_mkdir(dos)) {
                (void)dcl_printf(dcl[D].SYS_OUTPUT,"%s: %s\n",vms,strerror(errno));
                _STATUS = errno;
                _SEVERITY = 2;
                goto exit_label;
                }
            }
        else {
            _splitpath(dos,drive,dir,file,ext);
            if (strlen(ext)==0) {
                strcat(dos,".DAT");
                strcat(vms,".DAT");
                }
            if ((fp = fopen(dos,"wb"))==NULL) {
                (void)dcl_printf(dcl[D].SYS_OUTPUT,"%s: %s\n",vms,strerror(errno));
                _STATUS = errno;
                _SEVERITY = 2;
                goto exit_label;
                }
            dclcreate_read_sysinput(token);
            while (*token) {
                if (dcl_printf(fp,"%s\n",token)==EOF) {
                    (void)dcl_printf(dcl[D].SYS_OUTPUT,"%s: %s\n",vms,strerror(errno));
                    _STATUS = errno;
                    _SEVERITY = 2;
                    goto exit_label;
                    }
                dclcreate_read_sysinput(token);
                }
            fclose(fp);
            }

        if (f_log) {
            (void)dcl_printf(dcl[D].SYS_OUTPUT,"%s created.\n",vms);
            }
        }

exit_label:
    return(retcod);
}

//============================================================================
void dclcreate_read_sysinput(char *buf)
{
    (void)memset(buf,0,MAX_TOKEN);
    if (C == 1){
        (void)kbdread(buf,MAX_TOKEN,data_stack,INFINITE_TIMEOUT);
        }
    else {
        (void)rcld(cmd[C].cmd,buf,(unsigned)cmd[C].ln);
        if (*buf == '$')
            *buf = 0;
        else
            NEXT_LINE();
        }
    if (buf[strlen(buf)-1] == '\n' || buf[strlen(buf)-1] == '\r')
        buf[strlen(buf)-1] = 0;
}
