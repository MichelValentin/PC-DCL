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

#include <ctype.h>
#include <fcntl.h>
#include <sys/time.h>
#include <utime.h>
#include <string.h>
#include <errno.h>

#include "platform.h"
#include "dcl.h"
#include "dcltime.h"

typedef struct {
    char    confirm;
    char    log;
    char    attr;
    char    chmod;
    char    ok;
    char    wildcard;
    int     file_nb;
    time_t  date;
    time_t  before;
    time_t  since;
    } SETF_PARAM;

int dclsetf_do_it(char *path,DCL_FIND_DATA *ff,void *fn_param, char bdir);

int dcl_set_file(PARAM_T *p,PARAM_T *q)
{

    SETF_PARAM setf_param;
    char    token[MAX_TOKEN];
    char    vms[MAX_TOKEN];
    char    dos[MAX_TOKEN];
    int     i       = 0;
    int     j       = 0;
    int     recurse = 0;
    int     s_recurse = 0;
    int     retcod  = 0;
    char    *setfile_command = "SETFILE";

    if (cmd[C].subr) return(0);
    if (!dcl[D].cc[dcl[D].ccl]) return(0);
    if (p == NULL || q == NULL) return(DCL_ERROR);

    _STATUS = 0;
    *token  = 0; *vms = 0; *dos = 0;

    setf_param.log = 0;
    setf_param.confirm = 0;
    setf_param.date = 0;
    setf_param.attr = 0;
    setf_param.chmod = 0;
    setf_param.wildcard = 0;
    setf_param.file_nb = 0;
    setf_param.before = -1;
    setf_param.since = 0;
    setf_param.ok = 1;

    for (i = 0; q[i].tag; i++) {
        if (q[i].flag & PRESENT) 
            switch (q[i].tag) {
                case 2:                                 /* /ATTR     */
                    dcl_string(q[i].value,token,MAX_TOKEN);
                    setf_param.chmod = 1;
                    for (j=0;j < (int) strlen(token);j++){
                        switch (toupper(token[j])){
                            case 'R':   setf_param.attr |= _A_RDONLY;
                                        break;
                            case 'H':   setf_param.attr |= _A_HIDDEN;
                                        break;
                            case 'S':   setf_param.attr |= _A_SYSTEM;
                                        break;
                            case 'A':   setf_param.attr |= _A_ARCH;
                                        break;
                            default:    ;
                        }
                    }
                    break;
                case 3:                                 /* /BEFORE     */
                    dcl_string(q[i].value,token,MAX_TOKEN);
                    if (!*token) strcpy(token,"TODAY");
                    tm_str_to_long(token,&setf_param.before);
                    break;
                case 4:                                 /*  /CONFIRM   */
                    setf_param.confirm = 1;
                    break;
                case 5:                                 /*  /NOCONFIRM */
                    setf_param.confirm = 0;
                    break;
                case 6:                                 /*  /DATE      */
                    dcl_string(q[i].value,token,MAX_TOKEN);
                    tm_str_to_long(token,&setf_param.date);
                    break;
                case 7:                                 /*  /LOG       */
                    setf_param.log = 1;
                    break;
                case 8:                                 /*  /NOLOG     */
                    setf_param.log = 0;
                    break;
                case 9:                                 /*  /SINCE     */
                    dcl_string(q[i].value,token,MAX_TOKEN);
                    if (!*token) strcpy(token,"TODAY");
                    tm_str_to_long(token,&setf_param.since);
                    break;
                case 10:                                /*  /SUBDIR    */
                    recurse = 1;
                    break;
                case 11:                                /*  /NOSUBDIR  */
                    recurse = 0;
                    break;
                default:
                    ;
            } /* end switch */
    }   /* end for */

    dcl_string(p[1].value,vms,MAX_TOKEN);
    cvfs_lst_to_dos(vms,dos,&s_recurse);
    if (s_recurse) recurse = 1;
    setf_param.wildcard = (char) wildcard(dos);
    (void) dcl_searchdirlst(setfile_command,dos,recurse,dclsetf_do_it,&setf_param);
    if (setf_param.file_nb == 0){
        if (setf_param.wildcard)
            (void) dcl_printf(dcl[D].SYS_OUTPUT,"No file found.\n");
        else
            (void) dcl_printf(dcl[D].SYS_OUTPUT,"File not found.\n");
        _SEVERITY = 1;
        _STATUS = 2;
        retcod = -1;
    }
    
    return(retcod);
}

//============================================================================
int dclsetf_do_it(char *path,DCL_FIND_DATA *ff,void *fn_param, char bdir)
{
    SETF_PARAM *setf_param;
    char temp[MAX_TOKEN];
    char vms[MAX_TOKEN];

    if (path == NULL)     return(DCL_ERROR);
    if (ff == NULL)       return(DCL_ERROR);
    if (fn_param == NULL) return(DCL_ERROR);

    *temp = 0; *vms = 0;
    setf_param = (SETF_PARAM *) fn_param;
    if ((unsigned long)ff->WriteTime < (unsigned long)setf_param->since ||
        (unsigned long)ff->WriteTime > (unsigned long)setf_param->before) {
        goto exit_label;
    }

    setf_param->file_nb++;

    cvfs_dos_to_vms(path,vms);
    if (setf_param->confirm) {
        sprintf(temp,"Set file %s",vms);
        switch (dcl_confirm(temp)){
            case CONFIRM_YES    :   setf_param->ok = 1;
                                    break;
            case CONFIRM_ALL    :   setf_param->ok = 1;
                                    setf_param->confirm = 0;
                                    break;
            case CONFIRM_QUIT   :   setf_param->ok = 0;
                                    setf_param->confirm = 0;
                                    break;
            default             :   setf_param->ok = 0;
        }
    }
    if (!setf_param->ok) {
        goto exit_label;
    }

    if (setf_param->chmod) {
#ifdef _WIN32    	
        DWORD xattr;
        xattr = GetFileAttributes(path);
        if (setf_param->attr & _A_RDONLY)
            xattr |= _A_RDONLY;
        else
            xattr &= ~_A_RDONLY;
        if (setf_param->attr & _A_HIDDEN)
            xattr |= _A_HIDDEN;
        else
            xattr &= ~_A_HIDDEN;
        if (setf_param->attr & _A_SYSTEM)
            xattr |= _A_SYSTEM;
        else
            xattr &= ~_A_SYSTEM;
        if (setf_param->attr & _A_ARCH)
            xattr |= _A_ARCH;
        else
            xattr &= ~_A_ARCH;
        if (SetFileAttributes(path,(unsigned long)xattr) == 0) {
            (void) dcl_printf(dcl[D].SYS_OUTPUT,"%s: %s\n",vms,strerror(errno));
            _STATUS = (int) GetLastError();
            _SEVERITY = 2;
            goto exit_label;
        }
        if (setf_param->log)
            (void) dcl_printf(dcl[D].SYS_OUTPUT,"Attributes of %s modified.\n",vms);
#endif            
    }

    if (bdir)
        goto exit_label;

    if (setf_param->date) {
        struct utimbuf ts;
        ts.actime = setf_param->date;
        ts.modtime = setf_param->date;
        
    	if (utime(path, &ts) == 0) {
			if (setf_param->log) {
            (void) dcl_printf(dcl[D].SYS_OUTPUT,"Timestamp of %s modified.\n",vms);
			}
    	}
    	else {
            (void) dcl_printf(dcl[D].SYS_OUTPUT,"%s: %s\n",vms,strerror(errno));
            _STATUS = errno;
            _SEVERITY = 2;
            goto exit_label;
    	}
    }
exit_label:
    return(0);
}

//============================================================================
