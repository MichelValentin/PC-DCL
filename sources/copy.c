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
//#include <sys/io.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "platform.h"
#include "dcl.h"
#include "dcltime.h"
#include "dclfindfile.h"
#include "filecopy.h"

#ifndef S_IFDIR
#define S_IFDIR __S_IFDIR
#endif

// Copy -----------------------------------------------------
RPARAM_T COPY_PARAM[] = {
    {1,"From",MANDATORY},
    {2,"To",0},
    {0,"",0}
    };
RPARAM_T COPY_QUAL[] = {
    {1,"/ALL",0},
    {2,"/BEFORE",VALUE},
    {3,"/CONFIRM",0},
    {4,"/NOCONFIRM",0},
    {5,"/LOG",0},
    {6,"/NOLOG",0},
    {7,"/NEW",0},
    {8,"/SINCE",VALUE},
    {9,"/SUBDIR",0},
    {10,"/NOSUBDIR",0},
    {11,"/REPLACE",0},
    {12,"/NOREPLACE",0},
    {0,"",0}
    };

typedef struct {
    char    all;
    char    confirm;
    char    log;
    char    replace;
    char    wildcard;
    char    ok;
    char    fnew;
    char    recurse;
    int     file_nb;
    int     file_copied;
    time_t  before;
    time_t  since;
    char    vms[MAX_TOKEN];
} COP_PARAM;

long    filetstamp(char *filename);
int     dclcopy_do_it(char *path,DCL_FIND_DATA *ff,void *fn_param, char bdir);
int     vms_copy(char *vms1,char *vms2, COP_PARAM *cop_param);

int dcl_copy(PARAM_T *p,PARAM_T *q)
{
    COP_PARAM cop_param;
    char    vms[MAX_TOKEN];
    char    dos[MAX_TOKEN];
    int     i       = 0;
    int     s_recurse = 0;
    int     retcod  = DCL_OK;
    char    *szCmd = "COPY";

    NEXT_LINE();
    if (cmd[C].subr) return(DCL_OK);
    if (!dcl[D].cc[dcl[D].ccl]) return(DCL_OK);
    if (p == NULL || q == NULL) return(DCL_ERROR);

    SUBDIR = 0;
    _STATUS = 0;
    *vms = 0; *dos = 0;
    cop_param.all = 0;
    cop_param.confirm = 0;
    cop_param.log = 0;
    cop_param.replace = 1;
    cop_param.wildcard = 0;
    cop_param.file_nb = 0;
    cop_param.file_copied = 0;
    cop_param.vms[0] = 0;
    cop_param.before = -1;
    cop_param.since = 0;
    cop_param.ok = 1;
    cop_param.fnew = 0;
    cop_param.recurse = 0;

    retcod = cmd_parse_line(dcl_line,COPY_PARAM,COPY_QUAL,p,q);

    if (retcod == DCL_OK) {
        for (i = 0; q[i].tag; i++) {
            if (q[i].flag & PRESENT) 
                switch (q[i].tag) {
                case 1:                                 /* /ALL     */
                    cop_param.all = TRUE;
                    break;
                case 2:                                 /* /BEFORE  */
                    if (!*q[i].value) strcpy(q[i].value,"TODAY");
                    tm_str_to_long(q[i].value,&cop_param.before);
                    break;
                case 3:                                 /*  /CONFIRM */
                    cop_param.confirm = 1;
                    break;
                case 4:                                 /*  /NOCONFIRM */
                    cop_param.confirm = 0;
                    break;
                case 5:                                 /*  /LOG */
                    cop_param.log = 1;
                    break;
                case 6:                                 /*  /NOLOG */
                    cop_param.log = 0;
                    break;
                case 7:                                 /*  /NEW */
                    cop_param.fnew = 1;
                    break;
                case 8:                                 /*  /SINCE  */
                    if (!*q[i].value) strcpy(q[i].value,"TODAY");
                    tm_str_to_long(q[i].value,&cop_param.since);
                    break;
                case 9:                                 /*  /SUBDIR */
                    cop_param.recurse = 1;
                    break;
                case 10:                                /*  /NOSUBDIR */
                    cop_param.recurse = 0;
                    break;
                case 11:                                 /*  /REPLACE */
                    cop_param.replace = 1;
                    break;
                case 12:                                /*  /NOREPLACE */
                    cop_param.replace = 0;
                    break;
                default:
                    ;
                } /* end switch */
            }   /* end for */

        dcl_string(p[0].value,vms,MAX_TOKEN);
        dcl_string(p[1].value,cop_param.vms,MAX_TOKEN);
        cvfs_lst_to_dos(vms,dos,&s_recurse);
        if (s_recurse) cop_param.recurse = 1;
        if (dos[strlen(dos)-1] == ':')
            strcat(dos,"*.*");
        cop_param.wildcard = (char)wildcard(dos);
        (void)dcl_searchdirlst(szCmd,dos,cop_param.recurse,dclcopy_do_it,&cop_param);
        if (cop_param.file_nb == 0) {
            if (cop_param.wildcard)
                (void)dcl_printf(dcl[D].SYS_OUTPUT,"No file found.\n");
            else
                (void)dcl_printf(dcl[D].SYS_OUTPUT,"File not found.\n");
            _SEVERITY = 1;
            _STATUS = 2;
            retcod = DCL_ERROR;
        }
        else {
            if (cop_param.log && cop_param.file_copied > 1)
                (void)dcl_printf(dcl[D].SYS_OUTPUT,"%d files copied.\n",cop_param.file_copied);
        }
    }

    
    return(retcod);
}

//============================================================================
int dclcopy_do_it(char *path,DCL_FIND_DATA *ff,void *fn_param, char bdir)
{
    COP_PARAM *cop_param = (COP_PARAM *) fn_param;
    char temp[MAX_TOKEN];
    char vms[MAX_TOKEN];
    int  retcod = 0;

    UNREFERENCED_PARAM(bdir);

    if (HARDERR) goto exit_label;

    *temp = 0; *vms = 0;

    if (!cop_param->all){
        if (ff->dwFileAttributes & _A_HIDDEN || 
            ff->dwFileAttributes & _A_SYSTEM)
            goto exit_label;
        }
    if ((unsigned long)ff->WriteTime < (unsigned long)cop_param->since || 
        (unsigned long)ff->WriteTime > (unsigned long)cop_param->before) {
        goto exit_label;
        }

    cop_param->file_nb++;

    cvfs_dos_to_vms(path,vms);
    if (cop_param->confirm) {
        sprintf(temp,"Copy %s",vms);
        switch (dcl_confirm(temp)){
            case CONFIRM_YES    :   cop_param->ok = 1;
                                    break;
            case CONFIRM_ALL    :   cop_param->ok = 1;
                                    cop_param->confirm = 0;
                                    break;
            case CONFIRM_QUIT   :   cop_param->ok = 0;
                                    cop_param->confirm = 0;
                                    break;
            default             :   cop_param->ok = 0;
            }
        }
    if (cop_param->ok) {
        strcpy(temp,cop_param->vms);
        retcod = vms_copy(vms,temp,cop_param);
    }

exit_label:
    return(retcod);
}

//============================================================================

int vms_copy(char *vms1,char *vms2,COP_PARAM *cop_param)
{
    int     bFailIfExists   = FALSE;
    int     nError          = 0;
    char    dos1[MAX_TOKEN];
    char    dos2[MAX_TOKEN];
    char    vms0[MAX_TOKEN];
    char    directory1[_MAX_PATH];
    char    directory2[_MAX_PATH];
    char    drive1[_MAX_DRIVE];
    char    dir1[_MAX_DIR];
    char    file1[_MAX_FNAME];
    char    ext1[_MAX_EXT];
    char    drive2[_MAX_DRIVE];
    char    dir2[_MAX_DIR];
    char    file2[_MAX_FNAME];
    char    ext2[_MAX_EXT];
    size_t  i = 0;
    int    retcod = DCL_OK;

    *dos1 = 0; *drive1 = 0; *dir1 = 0; *file1 = 0; *ext1 =0; *directory1 = 0;
    *dos2 = 0; *drive2 = 0; *dir2 = 0; *file2 = 0; *ext2 =0; *directory2 = 0;

    cvfs_vms_to_dos(vms1,dos1,(int *)&i);
    _splitpath(dos1,drive1,dir1,file1,ext1);
    cvfs_vms_to_dos(vms2,dos2,(int *)&i);
    if (isdir(dos2) && dos2[strlen(dos2)-1] != SLASH_CHR)
            strcat(dos2,SLASH_STR);
    if (dos1[strlen(dos1)-1] == SLASH_CHR)
        dos1[strlen(dos1)-1] = '\0';
    if (!cvfs_check_device(dos2)) {
        _splitpath(dos2,drive2,dir2,file2,ext2);
        if (!*file2)
            strcpy(file2,file1);
        if (*file2 == '*')
            strcpy(file2,file1);
        if (!*ext2)
            strcpy(ext2,ext1);
        if (strncmp(ext2,".*",2)==0)
            strcpy(ext2,ext1);
        for (i = 0;i < strlen(file2); i++)
            if (file2[i] == '?') file2[i] = file1[i];
        for (i = 0;i < strlen(ext2); i++)
            if (ext2[i] == '?') ext2[i] = ext1[i];
        _makepath(dos2,drive2,dir2,file2,ext2);
        cvfs_dos_to_vms(dos2,vms2);

        if (cop_param->fnew) {
            if (filetstamp(dos1) <= filetstamp(dos2)) {
                goto exit_label;
            }
        }
    }

    _makepath(directory1,drive1,dir1,NULL,NULL);
    if (SUBDIR) {
        for(i = 1; i < strlen(dir1) && dir1[i] != SLASH_CHR; i++) ;
        if (dir1[i] == SLASH_CHR) i++;
        strcat(dir2, &dir1[i]);
        _makepath(dos2,drive2,dir2,file2,ext2);
    }
    _makepath(directory2,drive2,dir2,NULL,NULL);

    bFailIfExists = cop_param->replace ? FALSE : TRUE;
    
    (void)_mkdir(directory2);
    if (*file2) {
        nError = filecopy(dos1, dos2, bFailIfExists);
    }
    
    if (nError) {
        (void)dcl_printf(dcl[D].SYS_OUTPUT,"%s: %s\n",vms1,strerror(errno));
        _STATUS = nError;
        _SEVERITY = 2;
        retcod = DCL_ERROR;
    }
    else {
        cop_param->file_copied++;
        if (cop_param->log) {
            cvfs_dos_to_vms(dos2,vms0);
            (void)dcl_printf(dcl[D].SYS_OUTPUT,"%s copied to %s.\n",vms1,vms0);
        }
    }

exit_label:
    return(retcod);
}

long filetstamp(char *fn)
{
   struct stat buf;
   int    result;

   result = stat(fn, &buf);
   return result == 0  ? buf.st_mtime : 0;
}

int isdir(const char *pathname)
{
    int     result = 0;
    int     rc = 0;
    struct  stat sb;

    rc = stat(pathname, &sb);
    if (rc == 0) {
        result = sb.st_mode & S_IFDIR;
    }
    return(result);
}
