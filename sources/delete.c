/****************************************************************************/
/*                                                                          */
/*   PC-DCL -  DCL szCommand line interpreter for Windows.                    */
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

//#include <direct.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "platform.h"
#include "dcl.h"
#include "dclfindfile.h"
#include "dcltime.h"

typedef struct {
    char    all;
    char    confirm;
    char    wildcard;
    char    erase;
    char    log;
    char    ok;
    int     file_nb;
    int     file_deleted;
    time_t  before;
    time_t  since;
    } DEL_PARAM;

// delete -----------------------------------------------------
RPARAM_T DELETE_PARAM[] = {
    {1,"Name",0},
    {0,"",0}
    };
RPARAM_T DELETE_QUAL[] = {
    {1,"/SYMBOL",0},
    {2,"/KEY",0},
    {3,"/ALL",0},
    {4,"/BEFORE",VALUE},
    {5,"/CONFIRM",0},
    {6,"/NOCONFIRM",0},
    {7,"/ERASE",0},
    {8,"/NOERASE",0},
    {9,"/LOG",0},
    {10,"/NOLOG",0},
    {11,"/NEW",0},
    {12,"/SINCE",VALUE},
    {13,"/SUBDIR",0},
    {14,"/NOSUBDIR",0},
    {15,"/GLOBAL",0},
    {16,"/LOCAL",0},
    {0,"",0}
    };

int dcl_del_file(PARAM_T *p,PARAM_T *q);
int dcl_del_symbol(PARAM_T *p,PARAM_T *q);
int dcl_del_key(PARAM_T *p,PARAM_T *q);
int dcldelf_do_it(char *path,DCL_FIND_DATA *ff,void *fn_param);
int dcl_del_searchdirlst(char *cmd,char *path_list,int subdir,
                         int fn(char *,DCL_FIND_DATA *,void *),
                         void *fn_param);
int dcl_del_searchdir(char *cmd,char *path_name,int subdir,
                      int fn(char *,DCL_FIND_DATA *,void *),
                      void *fn_param);

#define DELFILE 0
#define DELSYM  1
#define DELKEY  2

int dcl_delete(PARAM_T *p,PARAM_T *q)
{
    int     retcod = DCL_OK;
    int     deltype = DELFILE;
    int     i;

    NEXT_LINE();
    if (cmd[C].subr) return(DCL_OK);
    if (!dcl[D].cc[dcl[D].ccl]) return(DCL_OK);

    _STATUS = 0;
    retcod = cmd_parse_line(dcl_line,DELETE_PARAM,DELETE_QUAL,p,q);
    if (retcod == DCL_OK) {
        for (i = 0; q[i].tag; i++) {
            if (q[i].flag & PRESENT)
                switch (q[i].tag) {
                    case 1:                                 /* /SYMBOL  */
                        deltype = DELSYM;
                        break;
                    case 2:                                 /* /KEY     */
                        deltype = DELKEY;
                        break;
                    default:
                        ;
                    } /* end switch */
            }   /* end for */

        switch (deltype) {
            case DELSYM:
                retcod = dcl_del_symbol(p,q);
                break;
            case DELKEY:
                retcod = dcl_del_key(p,q);
                break;
            default:
                retcod = dcl_del_file(p,q);
            }
        }

    return(retcod);
}
//===========================================================================
int dcl_del_file(PARAM_T *p,PARAM_T *q)
{
    DEL_PARAM del_param;
    char    vms[MAX_TOKEN];
    char    dos[MAX_TOKEN];
    int     i       = 0;
    int     recurse = 0;
    int     s_recurse = 0;
    int     retcod  = DCL_OK;
    char    *szCommand = "DELETE";

    del_param.all = 0;
    del_param.confirm = 0;
    del_param.log = 0;
    del_param.erase = 0;
    del_param.wildcard = 0;
    del_param.file_nb = 0;
    del_param.file_deleted = 0;
    del_param.before = -1;
    del_param.since = 0;
    del_param.ok = 1;

    retcod = cmd_parse_line(dcl_line,DELETE_PARAM,DELETE_QUAL,p,q);

    if (retcod == DCL_OK) {
        for (i = 0; q[i].tag; i++) {
            if (q[i].flag & PRESENT)
                switch (q[i].tag) {
                    case 3:                                 /* /ALL     */
                        del_param.all = TRUE;
                        break;
                    case 4:                                 /* /BEFORE  */
                        if (q[i].value == 0)
                            strcpy(q[i].value,"TODAY");
                        tm_str_to_long(q[i].value,&del_param.before);
                        break;
                    case 5:                                 /* /CONFIRM */
                        del_param.confirm = TRUE;
                        break;
                    case 6:                                /* /NOCONFIRM*/
                        del_param.confirm = FALSE;
                        break;
                    case 7:                                 /* /ERASE   */
                        del_param.erase = TRUE;
                        break;
                    case 8:                                 /* /NOERASE */
                        del_param.erase = FALSE;
                        break;
                    case 9:                                 /* /LOG     */
                        del_param.log = TRUE;
                        break;
                    case 10:                                /* /NOLOG   */
                        del_param.log = FALSE;
                        break;
                    case 12:                                 /* /SINCE  */
                        if (q[i].value == 0)
                            strcpy(q[i].value,"TODAY");
                        tm_str_to_long(q[i].value,&del_param.since);
                        break;
                    case 13:                                 /* /SUBDIR  */
                        recurse = TRUE;
                        break;
                    case 14:                                 /* /NOSUBDIR  */
                        recurse = FALSE;
                        break;
                    default:
                        (void)dcl_printf(dcl[D].SYS_OUTPUT,"Invalid szCommand qualifier\n");
                        _SEVERITY = 2;
                        _STATUS = 19;
                        return(DCL_ERROR);
                    } /* end switch */
            }   /* end for */

        dcl_string(p[0].value,vms,MAX_TOKEN);
        cvfs_lst_to_dos(vms,dos,&s_recurse);
        if (dos[strlen(dos)-1] == SLASH_CHR) dos[strlen(dos)-1] = 0;
        if (s_recurse) recurse = 1;
        del_param.wildcard = (char)wildcard(dos);
        //dcl_del_searchdirlst(szCommand,dos,recurse,dcldelf_do_it,&del_param);
        (void)dcl_del_searchdir(szCommand,dos,recurse,dcldelf_do_it,&del_param);
        if (del_param.file_nb == 0){
            if (del_param.wildcard)
                (void)dcl_printf(dcl[D].SYS_OUTPUT,"No file found.\n");
            else
                (void)dcl_printf(dcl[D].SYS_OUTPUT,"File not found.\n");
            _SEVERITY = 1;
            _STATUS = 2;
            retcod = DCL_ERROR;
            goto exit_label;
        }
        else {
            if (del_param.log && del_param.file_deleted > 1)
                (void)dcl_printf(dcl[D].SYS_OUTPUT,"%d files deleted.\n",del_param.file_deleted);
        }
    }
exit_label:
    return(retcod);
}

//---------------------------------------------------------------------------
int dcldelf_do_it(char *path,DCL_FIND_DATA *ff,void *fn_param)
{
    DEL_PARAM *del_param = (DEL_PARAM *) fn_param;
    char vms[MAX_TOKEN];
    char msg[MAX_TOKEN];
    int  retcod = DCL_OK;

    if (ff->dwFileAttributes & _A_SUBDIR) {
        if ((strcmp(ff->cFileName,".") == 0) || (strcmp(ff->cFileName,"..") == 0))
            goto exit_label;
        //++++if (path[strlen(path)-1] != '\\') strcat(path, "\\");
    }

    if (!del_param->all){
        if (ff->dwFileAttributes & _A_HIDDEN ||
            ff->dwFileAttributes & _A_SYSTEM)
            goto exit_label;
    }
    if ((unsigned long)ff->WriteTime < (unsigned long)del_param->since ||
        (unsigned long)ff->WriteTime > (unsigned long)del_param->before) {
        goto exit_label;
    }

    del_param->file_nb++;

    cvfs_dos_to_vms(path,vms);
    if (del_param->confirm) {
        sprintf(msg,"delete %s",vms);
        switch (dcl_confirm(msg)){
            case CONFIRM_YES    :   del_param->ok = 1;
                                    break;
            case CONFIRM_ALL    :   del_param->ok = 1;
                                    del_param->confirm = 0;
                                    break;
            case CONFIRM_QUIT   :   del_param->ok = 0;
                                    del_param->confirm = 0;
                                    break;
            default             :   del_param->ok = 0;
            }
        }
    if (!del_param->ok) {
        goto exit_label;
    }
    if (ff->dwFileAttributes & _A_SUBDIR) {
        if (_rmdir(path)) {
            (void)dcl_printf(dcl[D].SYS_OUTPUT,"%s: %s\n",vms,strerror(errno));
            _STATUS = errno;
            _SEVERITY = 2;
            goto exit_label;
        }
        else {
            if (del_param->log)
                (void)dcl_printf(dcl[D].SYS_OUTPUT,"%s deleted.\n",vms);
        }
    }
    else {
        if (del_param->erase){
            int handle;
            long l,fl;
            char fillbuf[2] = {0x00,0x00};
            fillbuf[0] = (char) 255;
/*            if ((handle = open(path,O_RDWR+O_BINARY)) == -1) {*/
            if ((handle = open(path,O_RDWR)) == -1) {
                (void)dcl_printf(dcl[D].SYS_OUTPUT,"%s: %s\n",vms,strerror(errno));
                _STATUS = errno;
                _SEVERITY = 2;
                goto exit_label;
            }
            fl = filelength(handle);
            for (l = 0; l < fl; l++){
                if (write(handle,fillbuf,1) < 1) {
                    (void)dcl_printf(dcl[D].SYS_OUTPUT,"%s: %s\n",vms,strerror(errno));
                    _STATUS = errno;
                    _SEVERITY = 2;
                    goto exit_label;
                }
            }
            close(handle);
        }

        if (remove(path)) {
            (void)dcl_printf(dcl[D].SYS_OUTPUT,"%s: %s\n",vms,strerror(errno));
            _STATUS = errno;
            _SEVERITY = 2;
            goto exit_label;
        }
        else {
            del_param->file_deleted++;
            if (del_param->log)
                (void)dcl_printf(dcl[D].SYS_OUTPUT,"%s deleted.\n",vms);
        }
    }

exit_label:
    return(retcod);
}


//============================================================================
int dcl_del_symbol(PARAM_T *p,PARAM_T *q)
{
    char    name[SYMBOL_MAX_NAME];
    SYMSCAN ss;
    int     f_all   = 0;
    int     f_global= 0;
    int     f_local = 0;
    int     f_log   = 0;
    int     len     = 0;
    int     i       = 0;

    *name = 0;

    if (DCL_OK == cmd_parse_line(dcl_line,DELETE_PARAM,DELETE_QUAL,p,q)) {
        for (i = 0; q[i].tag; i++) {
            if (q[i].flag & PRESENT)
                switch (q[i].tag) {
                    case 1:                                 /* /SYMBOL     */
                        break;
                    case 3:                                 /* /ALL     */
                        f_all = TRUE;
                        break;
                    case 9:                                 /* /LOG     */
                        f_log = TRUE;
                        break;
                    case 10:                                /* /NOLOG   */
                        f_log = FALSE;
                        break;
                    case 15:                                 /* /GLOBAL  */
                        f_global = TRUE;
                        f_local  = FALSE;
                        break;
                    case 16:                                /* /LOCAL    */
                        f_global = FALSE;
                        f_local  = TRUE;
                        break;
                    default:
                        (void)dcl_printf(dcl[D].SYS_OUTPUT,"Invalid szCommand qualifier\n");
                        _SEVERITY = 2;
                        _STATUS = 19;
                        return(DCL_ERROR);
                    } /* end switch */
            }   /* end for */

        dcl_string(p[0].value,name,SYMBOL_MAX_NAME);
        if (f_all && *name) {
            (void)dcl_printf(dcl[D].SYS_OUTPUT,"/ALL and symbol-name are mutually exclusive.\n");
            _SEVERITY = 2;
            _STATUS = 19;
            return(DCL_ERROR);
            }

        if (!(f_all || *name)) {
            (void)dcl_printf(dcl[D].SYS_OUTPUT,"ERROR - Missing szCommand parameter.\n");
            _SEVERITY = 2;
            _STATUS = 19;
            return(DCL_ERROR);
            }

        while(name[len] && name[len] != '*') len++;
        symbol_first(&ss);
        while (ss.name[0]) {
            if (f_all ||
                (!f_all && !len) ||
                strncasecmp(name,ss.name,(unsigned)len)==0) {
                    if (!(f_global || f_local) ||
                        (f_global && ss.level == 0) ||
                        (f_local && ss.level == D)) {
                            symbol_del_lvl(ss.name,ss.level);
                            if (ss.num) ss.num--;
                            if (f_log) {
                                if (ss.level == 0)
                                    dcl_printf(dcl[D].SYS_OUTPUT,"GLOBAL");
                                else
                                    dcl_printf(dcl[D].SYS_OUTPUT,"LOCAL");
                                dcl_printf(dcl[D].SYS_OUTPUT," symbol %s has been deleted.\n"
                                    ,ss.name);
                                }
                            }
                    }
            symbol_next(&ss);
            }
        return(DCL_OK);
        }
    else
        return(DCL_ERROR);
}

//============================================================================
int dcl_del_key(PARAM_T *p,PARAM_T *q)
{
    char    *keys[25] = {"F1","F2","F3","F4","F5","F6","F7","F8","F9","F10",
                         "SF1","SF2","SF3","SF4","SF5","SF6","SF7","SF8","SF9","SF10",
                         "F11","F12","SF11","SF12",NULL};
    char    name[SYMBOL_MAX_NAME];
    int     f_all   = 0;
    int     f_log   = 0;
    int     i       = 0;
    int     first   = 0;
    int     last    = 0;

    *name = 0;
    cmd_parse_line(dcl_line,DELETE_PARAM,DELETE_QUAL,p,q);

    for (i = 0; q[i].tag; i++) {
        if (q[i].flag & PRESENT)
            switch (q[i].tag) {
                case 2:                                 /* /KEY     */
                    break;
                case 3:                                 /* /ALL     */
                    f_all = TRUE;
                    break;
                case 9:                                 /* /LOG     */
                    f_log = TRUE;
                    break;
                case 10:                                /* /NOLOG   */
                    f_log = FALSE;
                    break;
                default:
                    dcl_printf(dcl[D].SYS_OUTPUT,"Invalid szCommand qualifier\n");
                    _SEVERITY = 2;
                    _STATUS = 19;
                    return(-1);
                } /* end switch */
        }   /* end for */

    dcl_string(p[0].value,name,MAX_TOKEN);

    if (f_all && *name) {
        dcl_printf(dcl[D].SYS_OUTPUT,"/ALL and key-name are mutually exclusive.\n");
        _SEVERITY = 2;
        _STATUS = 19;
        return(DCL_ERROR);
        }

    if (!(f_all || *name)) {
        dcl_printf(dcl[D].SYS_OUTPUT,"ERROR - Missing szCommand parameter.\n");
        _SEVERITY = 2;
        _STATUS = 19;
        return(DCL_ERROR);
        }


    if (f_all){
        first = 0;
        last  = 23;
        }
    else {
        i = dcl_search_key(keys,name);
        if (i < 0) {
            dcl_printf(dcl[D].SYS_OUTPUT,"Invalid KEY parameter\n");
            _SEVERITY = 2;
            _STATUS = 19;
            return(-1);
            }
        first = i;
        last = i;
        }

    for (i = first; i <= last; i++){
        if (keydef[i].value) {
            free(keydef[i].value);
            keydef[i].value = NULL;
            if (f_log) {
                dcl_printf(dcl[D].SYS_OUTPUT,"Key definition %s deleted.\n",keys[i]);
                }
            }
        else {
            if (f_log && !f_all) {
                dcl_printf(dcl[D].SYS_OUTPUT,"Key %s undefined.\n",keys[i]);
                }
            }
        }

    return(0);
}

int dcl_del_searchdirlst(char *cmd,char *path_list,int subdir,
                         int fn(char *,DCL_FIND_DATA *,void *),
                         void *fn_param)
{
    char    path_name[_MAX_PATH];
    int     i   = 0;
    int     j   = 0;

    *path_name = 0;

    for (i = 0; path_list[i]; i++) {
        if (path_list[i] == ',') {
            (void) dcl_del_searchdir(cmd, path_name, subdir, fn, fn_param);
            *path_name = 0;
            j = 0;
        }
        else {
            path_name[j++] = path_list[i];
            path_name[j] = 0;
        }
    }
    (void) dcl_del_searchdir(cmd, path_name, subdir, fn, fn_param);

    return DCL_OK;
}

int dcl_del_searchdir(char *cmd,char *path,int subdir,
                      int fn(char *,DCL_FIND_DATA *,void *),
                      void *fn_param)
{
    DCL_FIND_DATA ff;
    int     ok;
    int     rc;
    char    *fname = (char *) calloc(1,_MAX_PATH);
    char    *spath = (char *) calloc(1,_MAX_PATH);
    char    *temp  = (char *) calloc(1,_MAX_PATH);
    char    *sub   = (char *) calloc(1,_MAX_PATH);
    char    *drive = (char *) calloc(1,_MAX_DRIVE);
    char    *dir   = (char *) calloc(1,_MAX_DIR);
    char    *dir0  = (char *) calloc(1,_MAX_DIR);
    char    *file  = (char *) calloc(1,_MAX_FNAME);
    char    *ext   = (char *) calloc(1,_MAX_EXT);
    int     handle;

//    _splitpath(strupr(path),drive,dir,file,ext);
    _splitpath(path,drive,dir,file,ext);
    strcat(strcpy(fname, file), ext);
//    _makepath(spath,drive,dir,"*",".*");
    _makepath(spath,drive,dir,"*","");

    handle = Dcl_FindFirstFile(spath,&ff);
    ok = handle == (int)INVALID_HANDLE_VALUE ? 0 : 1;
    while (ok && !CTRL_Y && !HARDERR) {
        if ((ff.dwFileAttributes & _A_SUBDIR)) {
            if ((strcmp(ff.cFileName,".") != 0) &&
                (strcmp(ff.cFileName,"..") != 0)) {
                if (subdir) {
                    strcat(strcpy(dir0, dir), ff.cFileName);
                    _makepath(temp,drive,dir0,file,ext);
                    rc = dcl_del_searchdir(cmd,temp,1,fn, fn_param);
                    ok = rc == DCL_OK ? 1 : 0;
                    if (cvfs_cmp_file_name(fname, ff.cFileName)) {
                        sprintf(temp,"%s%s%s",drive,dir,ff.cFileName);
                        fn(temp,&ff,fn_param);
                    }
                }
                else {
                    if (!wildcard(path)) {
                        if (cvfs_cmp_file_name(fname, ff.cFileName)) {
                            sprintf(temp,"%s%s%s",drive,dir,ff.cFileName);
                            fn(temp,&ff,fn_param);
                        }
                    }
                }
            }
        }
        else {
            if (cvfs_cmp_file_name(fname, ff.cFileName)) {
                sprintf(temp,"%s%s%s",drive,dir,ff.cFileName);
                fn(temp,&ff,fn_param);
            }
        }
        ok = Dcl_FindNextFile(handle, &ff);
    }
    Dcl_FindClose(handle);

    free(fname); free(spath); free(temp); free(sub);
    free(drive); free(dir);   free(dir0); free(file); free(ext);

    return(1);
}
