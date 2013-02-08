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
#include <errno.h>

#include "platform.h"
#include "dcl.h"
#include "dcltime.h"

// Rename -----------------------------------------------------
RPARAM_T RENAME_PARAM[] = {
    {1,"From",MANDATORY},
    {2,"To",MANDATORY},
    {0,"",0}
    };
RPARAM_T RENAME_QUAL[] = {
    {1,"/ALL",0},
    {2,"/BEFORE",VALUE},
    {3,"/CONFIRM",0},
    {4,"/NOCONFIRM",0},
    {5,"/LOG",0},
    {6,"/NOLOG",0},
    {7,"/SINCE",VALUE},
    {8,"/SUBDIR",0},
    {9,"/NOSUBDIR",0},
    {0,"",0}
    };

typedef struct {
    char    all;
    char    confirm;
    char    log;
    char    wildcard;
    char    ok;
    int     file_nb;
    time_t  before;
    time_t  since;
    char    vms[MAX_TOKEN];
    } REN_PARAM;

int dclrename_do_it(char *path,DCL_FIND_DATA *ff,void *fn_param, char bdir);
int vms_rename(char *vms1,char *vms2);

int dcl_rename(PARAM_T *p,PARAM_T *q)
{
    REN_PARAM ren_param;
    char    vms[MAX_TOKEN];
    char    dos[MAX_TOKEN];
    int     i       = 0;
    int     recurse = 0;
    int     s_recurse = 0;
    int     retcod  = 0;
    char    *rename_command = "RENAME";

    NEXT_LINE();
    if (cmd[C].subr) return(0);
    if (!dcl[D].cc[dcl[D].ccl]) return(0);
    if (p == NULL || q == NULL) return(DCL_ERROR);

    _STATUS = 0;
    *vms = 0; *dos = 0;
    ren_param.all = 0;
    ren_param.confirm = 0;
    ren_param.log = 0;
    ren_param.wildcard = 0;
    ren_param.file_nb = 0;
    ren_param.vms[0] = 0;
    ren_param.before = -1;
    ren_param.since = 0;
    ren_param.ok = 1;
    (void)cmd_parse_line(dcl_line,RENAME_PARAM,RENAME_QUAL,p,q);

    for (i = 0; q[i].tag; i++) {
        if (q[i].flag & PRESENT) 
            switch (q[i].tag) {
                case 1:                                 /* /ALL     */
                    ren_param.all = TRUE;
                    break;
                case 2:                                 /* /BEFORE  */
                    if (!*q[i].value) strcpy(q[i].value,"TODAY");
                    tm_str_to_long(q[i].value,&ren_param.before);
                    break;
                case 3:                                 /*  /CONFIRM */
                    ren_param.confirm = 1;
                    break;
                case 4:                                 /*  /NOCONFIRM */
                    ren_param.confirm = 0;
                    break;
                case 5:                                 /*  /LOG */
                    ren_param.log = 1;
                    break;
                case 6:                                 /*  /NOLOG */
                    ren_param.log = 0;
                    break;
                case 7:                                 /*  /SINCE  */
                    if (!*q[i].value) strcpy(q[i].value,"TODAY");
                    tm_str_to_long(q[i].value,&ren_param.since);
                    break;
                case 8:                                 /*  /SUBDIR */
                    recurse = 1;
                    break;
                case 9:                                /*  /NOSUBDIR */
                    recurse = 0;
                    break;
                default:
                    ;
            } /* end switch */
    }   /* end for */

    dcl_string(p[0].value,vms,MAX_TOKEN);
    dcl_string(p[1].value,ren_param.vms,MAX_TOKEN);
    cvfs_lst_to_dos(vms,dos,&s_recurse);
    if (s_recurse) recurse = 1;
    if (isdir(dos) && dos[strlen(dos)-1] != SLASH_CHR) {
        strcat(dos,SLASH_STR);
        strcat(dos,"*.*");
    }
    if (dos[strlen(dos)-1] == ':')
        strcat(dos,"*.*");
    ren_param.wildcard = (char)wildcard(dos);
    (void)dcl_searchdirlst(rename_command,dos,recurse,dclrename_do_it,&ren_param);
    if (ren_param.file_nb == 0){
        if (ren_param.wildcard)
            (void)dcl_printf(dcl[D].SYS_OUTPUT,"No file found.\n");
        else
            (void)dcl_printf(dcl[D].SYS_OUTPUT,"File not found.\n");
        _SEVERITY = 1;
        _STATUS = 2;
        retcod = -1;
        goto exit_label;
    }
    
exit_label:
    return(retcod);
}

//============================================================================
int dclrename_do_it(char *path,DCL_FIND_DATA *ff,void *fn_param, char bdir)
{
    REN_PARAM *ren_param;
    char temp[MAX_TOKEN];
    char vms[MAX_TOKEN];

    if (path     == NULL) return(DCL_ERROR);
    if (ff       == NULL) return(DCL_ERROR);
    if (fn_param == NULL) return(DCL_ERROR);

    ren_param = (REN_PARAM *) fn_param;

    if (!ren_param->all){
        if (ff->dwFileAttributes & _A_HIDDEN ||
            ff->dwFileAttributes & _A_SYSTEM)
            goto exit_label;
    }
    if ((unsigned long)ff->WriteTime < (unsigned long)ren_param->since ||
        (unsigned long)ff->WriteTime > (unsigned long)ren_param->before) {
        goto exit_label;
    }

    ren_param->file_nb++;

    cvfs_dos_to_vms(path,vms);
    if (ren_param->confirm) {
        sprintf(temp,"Rename %s",vms);
        switch (dcl_confirm(temp)){
            case CONFIRM_YES    :   ren_param->ok = 1;
                                    break;
            case CONFIRM_ALL    :   ren_param->ok = 1;
                                    ren_param->confirm = 0;
                                    break;
            case CONFIRM_QUIT   :   ren_param->ok = 0;
                                    ren_param->confirm = 0;
                                    break;
            default             :   ren_param->ok = 0;
        }
    }
    if (!ren_param->ok) {
        goto exit_label;
    }

    strcpy(temp,ren_param->vms);
    if (vms_rename(vms,temp)) {
		(void)dcl_printf(dcl[D].SYS_OUTPUT,"%s: %s\n",vms,strerror(errno));
        _SEVERITY = 2;
        goto exit_label;
    }

    if (ren_param->log)
        (void)dcl_printf(dcl[D].SYS_OUTPUT,"%s renamed to %s.\n",vms,temp);

exit_label:
    return(0);
}

//============================================================================

int vms_rename(char *vms1,char *vms2)
{
    char dos1[MAX_TOKEN];
    char dos2[MAX_TOKEN];
    char drive1[_MAX_DRIVE];
    char dir1[_MAX_DIR];
    char file1[_MAX_FNAME];
    char ext1[_MAX_EXT];
    char drive2[_MAX_DRIVE];
    char dir2[_MAX_DIR];
    char file2[_MAX_FNAME];
    char ext2[_MAX_EXT];
    int  i;

    if (vms1 == NULL) return(DCL_ERROR);
    if (vms2 == NULL) return(DCL_ERROR);

    *dos1 = 0; *drive1 = 0; *dir1 = 0; *file1 = 0; *ext1 = 0;
    *dos2 = 0; *drive2 = 0; *dir2 = 0; *file2 = 0; *ext2 = 0;
    cvfs_vms_to_dos(vms1,dos1,&i);
    _splitpath(dos1,drive1,dir1,file1,ext1);
    cvfs_vms_to_dos(vms2,dos2,&i);
    if (isdir(dos2) && dos2[strlen(dos2)-1] != SLASH_CHR)
            strcat(dos2,SLASH_STR);
    _splitpath(dos2,drive2,dir2,file2,ext2);
    if (*file2 == '*' || !*file2)
        strcpy(file2,file1);
    if ((strncmp(ext2,".*",2)==0) || ! *ext2)
        strcpy(ext2,ext1);
    for (i = 0;i < (int) strlen(file2); i++)
        if (file2[i] == '?') file2[i] = file1[i];
    for (i = 0;i < (int) strlen(ext2); i++)
        if (ext2[i] == '?') ext2[i] = ext1[i];
    _makepath(dos2,drive2,dir2,file2,ext2);
    cvfs_dos_to_vms(dos2,vms2);
    i = rename(dos1,dos2);
    if (i == -1){
        _STATUS = errno;
        _SEVERITY = 2;
    }
    return(i);
}
