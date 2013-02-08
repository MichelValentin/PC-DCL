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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "platform.h"
#include "dcl.h"
#include "dclfindfile.h"
#include "dcltime.h"

// Append -----------------------------------------------------
RPARAM_T APPEND_PARAM[] = {
    {1,"From",MANDATORY},
    {2,"To",MANDATORY},
    {0,"",0}
    };
RPARAM_T APPEND_QUAL[] = {
    {1,"/ALL",0},
    {2,"/BEFORE",VALUE},
    {3,"/CONFIRM",0},
    {4,"/NOCONFIRM",0},
    {5,"/LOG",0},
    {6,"/NOLOG",0},
    {7,"/SINCE",VALUE},
    {8,"/SUBDIR",0},
    {9,"/NOSUBDIR",0},
    {10,"/BINARY",0},
    {11,"/TEXT",0},
    {0,"",0}
    };

typedef struct {
    char    all;
    char    confirm;
    char    log;
    char    binary;
    char    wildcard;
    char    ok;
    int     file_nb;
    time_t  before;
    time_t  since;
    char    vms[MAX_TOKEN];
    } APP_PARAM;

int dclappend_do_it(char *path,DCL_FIND_DATA *ff,void *fn_param, char bdir);
int vms_append(char *vms1,char *vms2,char binary);

int dcl_append(PARAM_T *p,PARAM_T *q)
{
    APP_PARAM app_param;
    char    vms[MAX_TOKEN];
    char    dos[MAX_TOKEN];
    int     i       = 0;
    int     recurse = 0;
    int     s_recurse = 0;
    int     retcod  = DCL_OK;
    char    *szCmd = "APPEND";

    NEXT_LINE();
    if (cmd[C].subr) return(DCL_OK);
    if (!dcl[D].cc[dcl[D].ccl]) return(DCL_OK);
    if (p == NULL || q == NULL) return(DCL_ERROR);

    _STATUS             = 0;
    *vms = 0; *dos      = 0;
    app_param.all       = 0;
    app_param.confirm   = 0;
    app_param.log       = 0;
    app_param.binary    = 0;
    app_param.wildcard  = 0;
    app_param.file_nb   = 0;
    app_param.vms[0]    = 0;
    app_param.before    = -1;
    app_param.since     =  0;
    app_param.ok = 1;

    retcod = cmd_parse_line(dcl_line,APPEND_PARAM,APPEND_QUAL,p,q);
    
    if (retcod == DCL_OK) {
        for (i = 0; q[i].tag; i++) {
            if (q[i].flag & PRESENT) 
                switch (q[i].tag) {
                case 1:                                 /* /ALL     */
                    app_param.all = TRUE;
                    break;
                case 2:                                 /* /BEFORE  */
                    if (!*q[i].value) strcpy(q[i].value,"TODAY");
                    tm_str_to_long(q[i].value,&app_param.before);
                    break;
                case 3:                                 /*  /CONFIRM */
                    app_param.confirm = 1;
                    break;
                case 4:                                 /*  /NOCONFIRM */
                    app_param.confirm = 0;
                    break;
                case 5:                                 /*  /LOG */
                    app_param.log = 1;
                    break;
                case 6:                                 /*  /NOLOG */
                    app_param.log = 0;
                    break;
                case 7:                                 /*  /SINCE  */
                    if (!*q[i].value) strcpy(q[i].value,"TODAY");
                    tm_str_to_long(q[i].value,&app_param.since);
                    break;
                case 8:                                 /*  /SUBDIR */
                    recurse = 1;
                    break;
                case 9:                                 /*  /NOSUBDIR */
                    recurse = 0;
                    break;
                case 10:                                /*  /BIN */
                    app_param.binary = 1;
                    break;
                case 11:                                /*  /TEXT */
                    app_param.binary = 0;
                    break;
                default:
                    ;
                } /* end switch */
        }   /* end for */

        dcl_string(p[0].value,vms,MAX_TOKEN);
        dcl_string(p[1].value,app_param.vms,MAX_TOKEN);
        cvfs_lst_to_dos(vms,dos,&s_recurse);
        if (s_recurse) recurse = 1;
        if (isdir(dos) && dos[strlen(dos)-1] != SLASH_CHR) {
            strcat(dos,SLASH_STR);
            strcat(dos,"*.*");
        }
        if (dos[strlen(dos)-1] == ':')
            strcat(dos,"*.*");
        app_param.wildcard = (char)wildcard(dos);
        (void) dcl_searchdirlst(szCmd,dos,recurse,dclappend_do_it,&app_param);
        if (app_param.file_nb == 0){
            if (app_param.wildcard)
                (void) dcl_printf(dcl[D].SYS_OUTPUT,"No file found.\n");
            else
                (void) dcl_printf(dcl[D].SYS_OUTPUT,"File not found.\n");
            _SEVERITY = 1;
            _STATUS = 2;
            retcod = DCL_ERROR;
        }
    } /* retcod == DCL_OK */

    return(retcod);
}

//============================================================================
int dclappend_do_it(char *path,DCL_FIND_DATA *ff,void *fn_param, char bdir)
{
    APP_PARAM *app_param;
    char temp[MAX_TOKEN];
    char vms[MAX_TOKEN];
    int  retcod = DCL_OK;

    if (path == NULL || ff == NULL || fn_param == NULL) return(DCL_ERROR);
    
    if (HARDERR) goto exit_label;

    app_param = (APP_PARAM *) fn_param;
    *temp = 0; *vms = 0;

    if (bdir){
        goto exit_label;
    }
    if (!app_param->all){
        if (ff->dwFileAttributes & _A_HIDDEN || 
            ff->dwFileAttributes & _A_SYSTEM)
            goto exit_label;
    }
    if ((unsigned long)ff->WriteTime < (unsigned long)app_param->since || 
        (unsigned long)ff->WriteTime > (unsigned long)app_param->before) {
        goto exit_label;
    }

    app_param->file_nb++;

    cvfs_dos_to_vms(path,vms);
    if (app_param->confirm) {
        sprintf(temp,"Append %s",vms);
        switch (dcl_confirm(temp)){
        case CONFIRM_YES    :   app_param->ok = 1;
                                break;
        case CONFIRM_ALL    :   app_param->ok = 1;
                                app_param->confirm = 0;
                                break;
        case CONFIRM_QUIT   :   app_param->ok = 0;
                                app_param->confirm = 0;
                                break;
        default             :   app_param->ok = 0;
        }
    }
    if (app_param->ok) {
        retcod = vms_append(vms,app_param->vms,app_param->binary);
        if (app_param->log && !retcod && !HARDERR)
            (void) dcl_printf(dcl[D].SYS_OUTPUT,"%s appended to %s.\n",vms,app_param->vms);
    }

exit_label:
    return(retcod);
}

//============================================================================

int vms_append(char *vms1,char *vms2,char binary)
{
    FILE *fp1;
    FILE *fp2;
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
    size_t i    = 0;
    int  rc     = 0;
    int  fexist = 1;
    char rmode[3];
    char amode[3];
    char ch;
    int  recurse;

    *dos1 = 0; *drive1 = 0; *dir1 = 0; *file1 = 0; *ext1 = 0;
    *dos2 = 0; *drive2 = 0; *dir2 = 0; *file2 = 0; *ext2 = 0;

    if (binary){
        strcpy(rmode,"rb");
        strcpy(amode,"ab");
    }
    else {
        strcpy(rmode,"rt");
        strcpy(amode,"at");
    }

    cvfs_vms_to_dos(vms1,dos1,&recurse);
    _splitpath(dos1,drive1,dir1,file1,ext1);
    cvfs_vms_to_dos(vms2,dos2,&recurse);
    if (isdir(dos2) && dos2[strlen(dos2)-1] != SLASH_CHR) {
        strcat(dos2,SLASH_STR);
        strcat(dos2,"*.*");
    }
    if (dos2[strlen(dos2)-1] == ':')
        strcat(dos2,"*.*");
    if (!cvfs_check_device(dos2)) {
        fexist = 0;
        _splitpath(dos2,drive2,dir2,file2,ext2);
        if (*file2 == '*')
            strcpy(file2,file1);
        if (strncmp(ext2,".*",2)==0)
            strcpy(ext2,ext1);
        for (i = 0;i < strlen(file2); i++)
            if (file2[i] == '?') file2[i] = file1[i];
        for (i = 0;i < strlen(ext2); i++)
            if (ext2[i] == '?') ext2[i] = ext1[i];
        _makepath(dos2,drive2,dir2,file2,ext2);
        cvfs_dos_to_vms(dos2,vms2);
        if ((fp2 = fopen(dos2,"r")) != NULL){
            fexist = 1;
            fclose(fp2);
        }
    }    
 
    rc = DCL_OK;
    if ((fp1 = fopen(dos1,rmode)) == NULL){
		(void)dcl_printf(dcl[D].SYS_OUTPUT,"%s: %s\n",vms1,strerror(errno));
        _SEVERITY = 2;
        _STATUS = errno;
        rc = DCL_ERROR;
        goto exit_label;
    }

    if (!fexist)
        (void)dcl_printf(dcl[D].SYS_OUTPUT,"%s created.\n",vms2);

    if ((fp2 = fopen(dos2,amode))== NULL) {
		(void)dcl_printf(dcl[D].SYS_OUTPUT,"%s: %s\n",vms2,strerror(errno));
        _SEVERITY = 2;
        _STATUS = errno;
        rc = DCL_ERROR;
        fclose(fp1);
        goto exit_label;
    }

    (void)setvbuf(fp1,NULL,_IOFBF,32767);
    (void)setvbuf(fp2,NULL,_IOFBF,32767);

    while (!feof(fp1) && !HARDERR){
        ch = (char)fgetc(fp1);
        if (!feof(fp1))
           fputc(ch,fp2);
    }

    fclose(fp2);
    fclose(fp1);

    if (HARDERR){
        rc = DCL_ERROR;
        _SEVERITY = 2;
    }

exit_label:
    return(rc);
}
