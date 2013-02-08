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
#include <ctype.h>
#include <errno.h>
#ifdef _WIN32
#include <direct.h>
#endif
#include "platform.h"
#include "dcl.h"
#include "dcltime.h"
#include "dclfindfile.h"
#include "termio.h"

#define NONE        0
#define BYTES       1
#define BLOCKS      2
#define KILOBYTES   3
#define MEGABYTES   4
#define GIGABYTES   5

#define O_NONE        0
#define O_NAME        1
#define O_EXTENSION   2
#define O_SIZE        3
#define O_DATE        4

// Dir  -----------------------------------------------------
RPARAM_T DIR_PARAM[] = {
    {1,"FileSpec",0},
    {0,"",0}
    };
RPARAM_T DIR_QUAL[] = {
    {1,"/ALL",0},
    {2,"/ATTRIBUTES",0},
    {3,"/BEFORE",VALUE},
    {4,"/BRIEF",0},
    {5,"/COLUMNS",VALUE},
    {6,"/DATE",0},
    {7,"/FULL",0},
    {8,"/GRAND_TOTAL",0},
    {9,"/HEADING",0},
    {10,"/NOHEADING",0},
    {11,"/OUTPUT",VALUE},
    {12,"/SINCE",VALUE},
    {13,"/SIZE",VALUE},
    {14,"/SUBDIR",0},
    {15,"/NOSUBDIR",0},
    {16,"/TOTAL",0},
    {17,"/TRAILING",0},
    {18,"/NOTRAILING",0},
    {19,"/TIME",0},
    {20,"/PAGE",0},
    {21,"/ORDER",VALUE},
    {22,"/SORT",VALUE},
    {0,"",0}
    };

typedef struct {
    char    all;
    char    attr;
    time_t  before;
    char    brief;
    char    cols;
    char    date;
    char    full;
    char    grand_total;
    char    heading;
    FILE    *output;
    time_t  since;
    char    size;
    char    total;
    char    trailing;
    int     col_counter;
    int     file_counter;
    int     dir_counter;
    long    tot_size;
    int     gt_file_counter;
    long    gt_tot_size;
    char    page;
    char    line_counter;
    char    order;
    } PARAM;

    char    G_drive[_MAX_DRIVE];
    char    G_dir[_MAX_DIR];
    char    G_file1[_MAX_FNAME];
    char    G_file2[_MAX_FNAME];
    char    G_ext1[_MAX_EXT];
    char    G_ext2[_MAX_EXT];

int dir_sort_n(const void *a,const void *b);
int dir_sort_e(const void *a,const void *b);
int dir_sort_s(const void *a,const void *b);
int dir_sort_d(const void *a,const void *b);
void dcldir_searchdir(char * name,int subdir,PARAM *dir_param);
int dcldir_do_it(char *drive,char *dir,char *name,
                 time_t time_create,time_t time_access,time_t time_write,
                 unsigned long fsize,unsigned int fattrib,PARAM *dir_param);
void linefeed(PARAM *dir_param);
void dir_printfilename(FILE *fp, char *format, char *filename);
int handler(int errval,int ax,int bp,int si);

int dcl_dir(PARAM_T *p,PARAM_T *q)
{
    char    *size_q[7]  = {"NONE","BYTES","BLOCKS","KILOBYTES","MEGABYTES","GIGABYTES",NULL};
    char    *order_q[6] = {"NONE","NAME","EXTENSION","SIZE","DATE",NULL};

    PARAM   dir_param;
    char    vms[MAX_TOKEN];
    char    dos[MAX_TOKEN];
    char    drive[_MAX_DRIVE];
    char    dir[_MAX_DIR];
    char    file[_MAX_FNAME];
    char    ext[_MAX_EXT];
    int     i       = 0;
    int     j       = 0;
    int     recurse = 0;
    int     s_recurse = 0;
    int     retcod  = DCL_OK;
    int     f_output = 0;

    NEXT_LINE();
    if (cmd[C].subr) return(DCL_OK);
    if (!dcl[D].cc[dcl[D].ccl]) return(DCL_OK);

    _STATUS = 0;
    memset(&dir_param,0,sizeof(dir_param));
    dir_param.brief = 1;
    dir_param.cols = (char)(terminfo.Size.X / 15);
    if (dir_param.cols == 0) dir_param.cols = 1;
    dir_param.heading = 1;
    dir_param.output = dcl[D].SYS_OUTPUT;
    dir_param.before = -1;
    dir_param.since  = 0;
    dir_param.trailing = 1;
    dir_param.order = O_NAME;
    dir_param.line_counter = tio_wherey();

    retcod = cmd_parse_line(dcl_line,DIR_PARAM,DIR_QUAL,p,q);

    for (i = 0; q[i].tag && retcod == DCL_OK; i++) {
        if (q[i].flag & PRESENT)
            switch (q[i].tag) {
                case 1:                                 /* /ALL     */
                    dir_param.all = TRUE;
                    break;
                case 2:                                 /* /ATTR    */
                    dir_param.attr = TRUE;
                    dir_param.brief = 0;
                    dir_param.full = 0;
                    break;
                case 3:                                 /* /BEFORE  */
                    if (!*q[i].value) strcpy(q[i].value,"TODAY");
                    tm_str_to_long(q[i].value,&dir_param.before);
                    break;
                case 4:                                 /*  /BRIEF  */
                    dir_param.brief = 1;
                    dir_param.attr  = 0;
                    dir_param.full  = 0;
                    dir_param.date = 0;
                    dir_param.size = 0;
                    break;
                case 5:                                /*  /COLUMNS    */
                    j = atoi(q[i].value);
                    if (j > 0) {
                        dir_param.cols = j;
                        }
                    else {
                        dcl_printf(dcl[D].SYS_OUTPUT,"WARNING: Invalid /COLUMNS parameter.\n");
                        _SEVERITY = _WARNING;
                        _STATUS = 19;
                        }
                    break;
                case 6:                                 /*  /DATE   */
                case 19:                                /*  /TIME   */
                    dir_param.date = 1;
                    dir_param.brief = 0;
                    dir_param.full  = 0;
                    break;
                case 7:                                 /*  /FULL   */
                    dir_param.full  = 1;
                    dir_param.brief = 0;
                    dir_param.date = 0;
                    dir_param.size = 0;
                    dir_param.attr  = 0;
                    break;
                case 8:                                 /*  /TOTAL  */
                    dir_param.total = 1;
                    dir_param.grand_total = 1;
                    break;
                case 9:                                 /*  /HEAD   */
                    dir_param.heading = 1;
                    break;
                case 10:                                /* /NOHEAD  */
                    dir_param.heading = 0;
                    break;
                case 11:                                /* /OUT=    */
                    dcl_string(q[i].value,vms,MAX_TOKEN);
                    cvfs_vms_to_dos(vms,dos,&recurse);
                    if (!cvfs_check_device(dos)){
                        _splitpath(dos,drive,dir,file,ext);
                        if (strlen(file) == 0){
                            strcat(dos,"DIRECTORY");
                            }
                        if (strlen(ext) == 0){
                            strcat(dos,".LIS");
                            }
                        }
                    dir_param.output = fopen(dos,"at");
                    if (dir_param.output == NULL) {
                        dcl_printf(dcl[D].SYS_OUTPUT,"%s: %s\n",vms,strerror(errno));
                        _SEVERITY = 2;
                        _STATUS = 19;
                        retcod = DCL_ERROR;
                        }
                    else
                        f_output = 1;
                    *vms = 0;
                    break;
                case 12:                            /*  /SINCE  */
                    if (!*q[i].value) strcpy(q[i].value,"TODAY");
                    tm_str_to_long(q[i].value,&dir_param.since);
                    break;
                case 13:                            /*  /SIZE   */
                    if (!*q[i].value) strcpy(q[i].value,"BYTES");
                    j = dcl_search(size_q,q[i].value);
                    switch (j) {
                        case AMBIGUOUS:
                            dcl_printf(dcl[D].SYS_OUTPUT,"Ambiguous /SIZE qualifier\n");
                            _SEVERITY = 2;
                            _STATUS = 19;
                            retcod = DCL_ERROR;
                            break;
                        case NOT_FOUND:
                            dcl_printf(dcl[D].SYS_OUTPUT,"Invalid /SIZE qualifier\n");
                            _SEVERITY = 2;
                            _STATUS = 19;
                            retcod = DCL_ERROR;
                            break;
                        case BYTES:
                        case BLOCKS:
                        case KILOBYTES:
                        case MEGABYTES:
                        case GIGABYTES:
                            dir_param.size  = j;
                            dir_param.brief = 0;
                            dir_param.full  = 0;
                            break;
                        default:
                            ;
                        }
                    break;
                case 14:                            /*  /SUBDIR */
                    s_recurse = 1;
                    break;
                case 15:                            /*  /NOSUBDIR   */
                    s_recurse = 0;
                    break;
                case 16:                            /*  /TOTAL  */
                    dir_param.total = 1;
                    break;
                case 17:                            /* /TRAIL   */
                    dir_param.trailing = 1;
                    break;
                case 18:                            /*  /NOTRAIL    */
                    dir_param.trailing = 0;
                    break;
                case 20:                            /* /PAGE    */
                    if (f_output) {
                        dcl_printf(dcl[D].SYS_OUTPUT,"/PAGE and /OUTPUT parameters are mutually exclusives.\n");
                        _SEVERITY = 2;
                        _STATUS = 19;
                        retcod = DCL_ERROR;
                        }
                    dir_param.page = 1;
                    break;
                case 21:                            /*  /ORDER   */
                case 22:                            /*  /SORT    */
                    if (!*q[i].value) strcpy(q[i].value,"NAME");
                    j = dcl_search(order_q,q[i].value);
                    switch (j) {
                        case AMBIGUOUS:
                            dcl_printf(dcl[D].SYS_OUTPUT,"Ambiguous /ORDER qualifier\n");
                            _SEVERITY = 2;
                            _STATUS = 19;
                            retcod = DCL_ERROR;
                            break;
                        case NOT_FOUND:
                            dcl_printf(dcl[D].SYS_OUTPUT,"Invalid /ORDER qualifier\n");
                            _SEVERITY = 2;
                            _STATUS = 19;
                            retcod = DCL_ERROR;
                            break;
                        default:
                            dir_param.order = j;
                            ;
                        }
                    break;
                default:
                    ;
                } /* end switch */
        }   /* end for */

    if (retcod == DCL_OK) {
        if (dir_param.heading == 0)
            dir_param.cols = 1;
        dcl_string(p[0].value,vms,MAX_TOKEN);
        if (strcmp(vms,"..") == 0)
            strcpy(vms,"[-]");
        cvfs_vms_to_dos(vms,dos,&recurse);
        if (isdir(dos) && dos[strlen(dos)-1] != SLASH_CHR)
            strcat(dos,SLASH_STR);
        if (s_recurse) recurse = s_recurse;
        dcldir_searchdir(dos,recurse,&dir_param);

        if (CTRL_Y) {
            goto ExitLabel;
        }

        if (dir_param.gt_file_counter == 0) {
            dcl_printf(dir_param.output,"File not found.\n");
            }
        if ((dir_param.dir_counter > 1 && dir_param.trailing)
            || dir_param.grand_total) {
            dcl_printf(dir_param.output,"Grand total of %d directories, %d files",
                    dir_param.dir_counter,dir_param.gt_file_counter);
            if (dir_param.size) {
                dcl_printf(dir_param.output,", %ld",dir_param.gt_tot_size);
                switch (dir_param.size) {
                    case BYTES:
                        dcl_printf(dir_param.output," bytes");
                        break;
                    case BLOCKS:
                        dcl_printf(dir_param.output," blocks");
                        break;
                    case KILOBYTES:
                        dcl_printf(dir_param.output," KBytes");
                        break;
                    case MEGABYTES:
                        dcl_printf(dir_param.output," MBytes");
                        break;
                    case GIGABYTES:
                        dcl_printf(dir_param.output," GBytes");
                        break;
                    default:
                        dcl_printf(dir_param.output," ***");
                    }
                }
            if (dir_param.full) {
                dcl_printf(dir_param.output,", %ld bytes",dir_param.gt_tot_size);
                }
            dcl_printf(dir_param.output,".\n");
            }
        }

ExitLabel:
    if (f_output)
        fclose(dir_param.output);
    if (HARDERR)
        tio_print_interrupt();

    return(retcod);
}

//============================================================================

void dcldir_searchdir(char * name,int subdir,PARAM *dir_param)
{
    DCL_FIND_DATA ff;
    int     ok;
    int     handle;
    int     attrib = _A_SUBDIR;
    int     i = 0;
    int     n = 0;
    int     d = 0;
    char    temp[_MAX_PATH];
    char    path[_MAX_PATH];
    char    drive[_MAX_DRIVE];
    char    dir[_MAX_DIR];
    char    file[_MAX_FNAME];
    char    ext[_MAX_EXT];
    DCL_FIND_DATA *dd;
    size_t  s;

    n = 1024;
    do
        {
        dd = (DCL_FIND_DATA *) calloc(n,sizeof(ff));
        if (dd == NULL) n /= 2;
        }
    while (n >= 128 && dd == NULL);
    if (dd == NULL){
        dcl_printf(dcl[D].SYS_OUTPUT,"ERROR - DIR002\n");
        _SEVERITY = 4;
        _STATUS = 8;
        return;
        }

    strcpy(path,name);
    //for (i = 0; i < (int) strlen(path); i++)
    //    path[i] = toupper(path[i]);
    if (dir_param->all)
        attrib = attrib + _A_HIDDEN + _A_SYSTEM;
    _splitpath(path,drive,dir,file,ext);
    if (strlen(file) == 0)
        strcat(path,"*");
//    if (strlen(ext) == 0)
//        strcat(path,".*");
    handle = Dcl_FindFirstFile(path,&ff);
    ok = handle == (int)INVALID_HANDLE_VALUE ? 0 : 1;
    dir_param->dir_counter++;
    dir_param->file_counter = 0;
    dir_param->col_counter = 0;
    dir_param->tot_size = 0;

    if (CTRL_Y) {
        ok = 0;
    }
    while (ok) {
        _splitpath(path,drive,dir,file,ext);
        if (strlen(drive)==0 && strlen(dir)==0) {
           if (_getcwd(temp,_MAX_PATH) != NULL) {
               s = strlen(temp)-1;
           }
           else {
               s = 0;
           }
           if (temp[s] != SLASH_CHR) {
              strcat(temp,SLASH_STR);
              _splitpath(temp,drive,dir,file,ext);
              }
           }
        if ((unsigned long)ff.WriteTime >= (unsigned long)dir_param->since &&
            (unsigned long)ff.WriteTime <  (unsigned long)dir_param->before)
            {
            if (d < n){
               memcpy(&dd[d],&ff,sizeof(ff));
               d++;
               }
            }
        ok = Dcl_FindNextFile(handle, &ff);
        if (CTRL_Y) {
            ok = 0;
        }
    }
    Dcl_FindClose(handle);

    if (CTRL_Y) {
        free(dd);
        return;
    }

    --d;
    if (d) {
        switch (dir_param->order) {
            case O_NAME:
                //for(i = 0; i < d; printf("%s\n",dd[i++].cFileName));
                qsort((DCL_FIND_DATA*) dd,d+1,sizeof(DCL_FIND_DATA),dir_sort_n);
                //for(i = 0; i < d; printf("%s\n",dd[i++].cFileName));
                break;
            case O_EXTENSION:
                qsort((DCL_FIND_DATA*) dd,d+1,sizeof(DCL_FIND_DATA),dir_sort_e);
                break;
            case O_SIZE:
                qsort((DCL_FIND_DATA*) dd,d+1,sizeof(DCL_FIND_DATA),dir_sort_s);
                break;
            case O_DATE:
                qsort((DCL_FIND_DATA*) dd,d+1,sizeof(DCL_FIND_DATA),dir_sort_d);
                break;
            default:
                ;
        }
    }

    for(i = 0;i <= d;i++) {
        dcldir_do_it(drive,dir,dd[i].cFileName,dd[i].CreationTime,dd[i].AccessTime,dd[i].WriteTime,dd[i].nFileSize,
            dd[i].dwFileAttributes,dir_param);
    }
    if (CTRL_Y) {
        free(dd);
        return;
    }

    if (dir_param->brief && (!dir_param->total) && dir_param->file_counter) {
        if (dir_param->col_counter != 0)
            linefeed(dir_param);
/*        linefeed(dir_param);*/
    }
    if (dir_param->file_counter > 0 && dir_param->trailing
        && (!dir_param->grand_total)) {
        dcl_printf(dir_param->output,"Total of %d file",dir_param->file_counter);
        if (dir_param->file_counter > 1)
            dcl_printf(dir_param->output,"s");
        if (dir_param->size) {
            dcl_printf(dir_param->output,", %ld",dir_param->tot_size);
            switch (dir_param->size) {
                case BYTES:
                    dcl_printf(dir_param->output," bytes");
                    break;
                case BLOCKS:
                    dcl_printf(dir_param->output," blocks");
                    break;
                case KILOBYTES:
                    dcl_printf(dir_param->output," KBytes");
                    break;
                case MEGABYTES:
                    dcl_printf(dir_param->output," MBytes");
                    break;
                case GIGABYTES:
                    dcl_printf(dir_param->output," GBytes");
                    break;
                default:
                    dcl_printf(dir_param->output," ***");
            }
        }
        if (dir_param->full)
            dcl_printf(dir_param->output,",  %ld bytes",dir_param->tot_size);
        linefeed(dir_param);
    }

    free(dd);

    strcpy(temp,path);
    _splitpath(path,drive,dir,file,ext);
//    _makepath(path,drive,dir,"*",".*");
    _makepath(path,drive,dir,"*",NULL);

    if (subdir) {
        handle = Dcl_FindFirstFile(path,&ff);
        ok = handle == (int)INVALID_HANDLE_VALUE ? 0 : 1;
        if (CTRL_Y) {
            ok = 0;
        }
        while (ok){
            if (ff.dwFileAttributes & _A_SUBDIR) {
            	if (strcmp(ff.cFileName, ".") && strcmp(ff.cFileName, "..")) {
	                _splitpath(temp,drive,dir,file,ext);
	                strcat(dir,ff.cFileName);
	                strcat(dir,SLASH_STR);
	                _makepath(path,drive,dir,file,ext);
	                dcldir_searchdir(path,1,dir_param);
            	}
            }
            ok = Dcl_FindNextFile(handle, &ff);
            if (CTRL_Y) {
                ok = 0;
            }
        }
        Dcl_FindClose(handle);
        strcpy(path,temp);
    }
}
//============================================================================
int dir_sort_n(const void *p1,const void *p2)
{
    DCL_FIND_DATA *a = (DCL_FIND_DATA*) p1;
    DCL_FIND_DATA *b = (DCL_FIND_DATA*) p2;
    int fa,fb;
    fa = a->dwFileAttributes & _A_SUBDIR;
    fb = b->dwFileAttributes & _A_SUBDIR;
    if (fa != fb)
        return(fb - fa);
    return(strcasecmp(a->cFileName,b->cFileName));
}
int dir_sort_e(const void *p1,const void *p2)
{
    DCL_FIND_DATA *a = (DCL_FIND_DATA*) p1;
    DCL_FIND_DATA *b = (DCL_FIND_DATA*) p2;
    int fa,fb,i;
    fa = a->dwFileAttributes & _A_SUBDIR;
    fb = b->dwFileAttributes & _A_SUBDIR;
    if (fa != fb)
        return(fb - fa);
    _splitpath(a->cFileName,G_drive,G_dir,G_file1,G_ext1);
    _splitpath(b->cFileName,G_drive,G_dir,G_file2,G_ext2);

    i = strcasecmp(G_ext1,G_ext2);
    if (i) return(i);
    return(strcasecmp(G_file1,G_file2));
}
int dir_sort_s(const void *p1,const void *p2)
{
    DCL_FIND_DATA  *a = (DCL_FIND_DATA *) p1;
    DCL_FIND_DATA  *b = (DCL_FIND_DATA *) p2;
    int            i  = 0;

    i = a->nFileSize - b->nFileSize;
    if (i) return(i);
    return(strcasecmp(a->cFileName,b->cFileName));
}
int dir_sort_d(const void *p1,const void *p2)
{
    DCL_FIND_DATA *a = (DCL_FIND_DATA*) p1;
    DCL_FIND_DATA *b = (DCL_FIND_DATA*) p2;
    int            i;

    i = 0;
    if (a->WriteTime < b->WriteTime) i = -1;
    if (a->WriteTime > b->WriteTime) i = 1;
    if (i) return(i);
    return(strcasecmp(a->cFileName,b->cFileName));

}

int dcldir_do_it(char *drive,char *dir,char *name,
                 time_t time_create,time_t time_access,time_t time_write,
                 unsigned long fsize,unsigned int fattrib,PARAM *dir_param)
{
    char    temp[MAX_TOKEN];
    char    vms[MAX_TOKEN];
    char    longname[MAX_TOKEN];
    char    shortname[MAX_TOKEN];
    char    xname[MAX_TOKEN];
    char    xattr[5];
    long    sz = 0;
#ifdef _WIN32
    DCL_FIND_DATA FindFileData;
    int     handle;
#endif

    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
        (void) logical_get("INI$FTYPE",INI_VALUE);
        if (INI_VALUE[0] == 'V') {
            return(DCL_OK);
        }
    }
#ifdef _WIN32
    if (!*drive) {
        drive[0] = _getdrive() + 'A' - 1;
        drive[1] = ':';
        drive[2] = 0;
        }
    strcpy(vms,drive);
    strcat(vms,dir);
    _fullpath(temp,vms,MAX_TOKEN);
#else
    strcpy(temp,dir);
#endif
    cvfs_dos_to_vms(temp,vms);

    if (vms[strlen(vms)-1] == SLASH_CHR && strlen(vms) > 1)
        vms[strlen(vms)-1] = 0;

    dir_param->file_counter++;
    dir_param->gt_file_counter++;
    dir_param->col_counter++;

    if (dir_param->file_counter == 1 && dir_param->heading
        && (!dir_param->grand_total)) {
        dcl_printf(dir_param->output,"\nDirectory %s",vms);
        linefeed(dir_param);
        linefeed(dir_param);
    }
#ifdef _WIN32
    strcpy(longname,drive);
    strcat(longname,dir);
    strcat(longname,name);
    handle = Dcl_FindFirstFile(longname, &FindFileData);
    if (handle != (int)INVALID_HANDLE_VALUE &&
        *FindFileData.cAlternateFileName && *name != '.')
        strcpy(shortname, FindFileData.cAlternateFileName);
    else
#endif
        strcpy(shortname,name);

    if (dir_param->heading) {
        if (fattrib & _A_SUBDIR) {
            sprintf(xname,"[%s]",shortname);
            strcpy(shortname, xname);
            sprintf(xname,"[%s]",name);
            strcpy(name, xname);
        }
    }

    if (dir_param->brief) {
        if (!dir_param->total) {
            int len = strlen(name);
            int col_inc = len / 15;
            int col_width = (col_inc * 15) + 15;
            char wrk[256];
            snprintf(wrk, 256,"%-*.*s",col_width,col_width,name);
            dir_printfilename(dir_param->output,"%s",wrk);
            dir_param->col_counter += col_inc;
        }
        if (dir_param->col_counter >= dir_param->cols) {
            if (!dir_param->total)
                linefeed(dir_param);
            dir_param->col_counter = 0;
        }
    }

    if (dir_param->full) {
        strcpy(xattr,"W...");
        if (fattrib & _A_RDONLY) xattr[0] = 'R';
        if (fattrib & _A_SUBDIR) xattr[0] = 'D';
        if (fattrib & _A_HIDDEN) xattr[1] = 'H';
        if (fattrib & _A_SYSTEM) xattr[2] = 'S';
        if (fattrib & _A_ARCH)   xattr[3] = 'A';
        xattr[4] = 0;
        if (!dir_param->total){
            dir_printfilename(dir_param->output,"%s",name);
            linefeed(dir_param);
            if (fattrib & _A_SUBDIR)
                dcl_printf(dir_param->output,"Size         : <DIR>");
            else
                dcl_printf(dir_param->output,"Size         : %ld",fsize);
            linefeed(dir_param);
            tm_long_to_str(&time_create, temp);
            temp[20] = 0;
            dcl_printf(dir_param->output,"Date created : %s",temp);
            linefeed(dir_param);
            tm_long_to_str(&time_access, temp);
            temp[20] = 0;
            dcl_printf(dir_param->output,"Date accessed: %s",temp);
            linefeed(dir_param);
            tm_long_to_str(&time_write, temp);
            temp[20] = 0;
            dcl_printf(dir_param->output,"Date written : %s",temp);
            linefeed(dir_param);
            dcl_printf(dir_param->output,"Attribute    : %s",xattr);
            linefeed(dir_param);
            linefeed(dir_param);
            }
        dir_param->tot_size += fsize;
        dir_param->gt_tot_size += fsize;
        }


    if (dir_param->full == 0 && dir_param->brief == 0) {
        if (!dir_param->total)
//            if (dir_param->heading)
                dir_printfilename(dir_param->output,"%-15.15s",shortname);
//            else
//                dir_printfilename(dir_param->output,"%s\t",xname);
        if (dir_param->date) {
            tm_long_to_str(&time_write, temp);
            temp[20] = 0;
            if (!dir_param->total)
                dcl_printf(dir_param->output,"    %s",temp);
            }
        if (dir_param->size) {
            switch (dir_param->size) {
                case BYTES:
                    sz = fsize;
                    break;
                case BLOCKS:
                    sz = fsize / 512;
                    if (fsize % 512) sz++;
                    break;
                case KILOBYTES:
                    sz = fsize / 1024;
                    if (fsize % 1024) sz++;
                    break;
                case MEGABYTES:
                    sz = fsize / (1024 * 1000);
                    break;
                case GIGABYTES:
                    sz = fsize / (1024 * 1000000);
                    break;
                default:
                    ;
                }
            if (!dir_param->total) {
                if (fattrib & _A_SUBDIR)
                    dcl_printf(dir_param->output,"    <DIR>    ");
                else
                    dcl_printf(dir_param->output,"    %9ld",sz);
            }
            dir_param->tot_size += sz;
            dir_param->gt_tot_size += sz;
            }
        if (dir_param->attr) {
            strcpy(xattr,"W...");
            if (fattrib & _A_RDONLY) xattr[0] = 'R';
            if (fattrib & _A_SUBDIR)  xattr[0] = 'D';
            if (fattrib & _A_HIDDEN) xattr[1] = 'H';
            if (fattrib & _A_SYSTEM) xattr[2] = 'S';
            if (fattrib & _A_ARCH)   xattr[3] = 'A';
            xattr[4] = 0;
            if (!dir_param->total)
                dcl_printf(dir_param->output,"  %s",xattr);
            }
        if (!dir_param->total) {
            if (dir_param->heading)
                dir_printfilename(dir_param->output," %s",name);
            else
                dir_printfilename(dir_param->output," %s",longname);
        }
        if (!dir_param->total)
            linefeed(dir_param);
    }
#ifdef _WIN32
    Dcl_FindClose(handle);
#endif

    return(DCL_OK);
}
#ifdef _WIN32
int diskinfo(int drive,DISKINFO *di)
{
    char            DirectoryName[16];
    DWORD           dwDummy1, dwDummy2;
    ULARGE_INTEGER  ullFreeBytesAvailable;
    ULARGE_INTEGER  ullTotalNumberOfBytes;
    ULARGE_INTEGER  ullTotalNumberOfFreeBytes;

    DirectoryName[0] = drive + '@';
    DirectoryName[1] = ':';
    DirectoryName[2] = SLASH_CHR;
    DirectoryName[3] = 0;

    memset(di, 0, sizeof(DISKINFO));
    if (GetVolumeInformation(DirectoryName,
                             di->szName,
                             sizeof(di->szName),
                             &di->dwVolumeSerialNumber,
                             &dwDummy1,
                             &dwDummy2,
                             di->szFileSystemName,
                             sizeof(di->szFileSystemName)
                             )) {
        if (GetDiskFreeSpaceEx(DirectoryName,
                               &ullFreeBytesAvailable,
                               &ullTotalNumberOfBytes,
                               &ullTotalNumberOfFreeBytes)) {
            di->FreeBytesAvailable = ullFreeBytesAvailable.QuadPart;
            di->TotalNumberOfBytes = ullTotalNumberOfBytes.QuadPart;
            di->TotalNumberOfFreeBytes = ullTotalNumberOfFreeBytes.QuadPart;
            switch (GetDriveType(DirectoryName)) {
            case DRIVE_REMOVABLE:
                strcpy(di->szType, "Removable disk");
                break;
            case DRIVE_FIXED:
                strcpy(di->szType, "Fixed disk");
                break;
            case DRIVE_REMOTE:
                strcpy(di->szType, "Network disk");
                break;
            case DRIVE_CDROM:
                strcpy(di->szType, "CD-ROM disk");
                break;
            case DRIVE_RAMDISK:
                strcpy(di->szType, "RAM disk");
                break;
            default:
                strcpy(di->szType, "Unknown");
            }
            return(DCL_OK);
        }
    }
	memset(di, 0, sizeof(DISKINFO));
    return(DCL_ERROR);
}
#else
int diskinfo(char *drive,DISKINFO *di)
{
	FILE	*fp = NULL;
	char	line[MAX_TOKEN];
	char	name[MAX_TOKEN];
	char    type[MAX_TOKEN];
	char    mount[MAX_TOKEN];
	int		blocks		= 0;
	long	used 		= 0;
	long	available 	= 0;
	int 	pct 		= 0;
	int		rc 			= 0;
	char	df[MAX_TOKEN];

	memset(di, 0, sizeof(DISKINFO));
	strcpy(df, "df -T ");
	strcat(df, drive);
	strcat(df, " 2>/dev/null");
	fp = popen(df, "r");
	if (fp != NULL) {
    	while(fgets(line, sizeof(line), fp))  {
    		if (*line == '/') {
            	rc = sscanf(line, "%s %s %d %ld %ld %d%% %s ",
            			name, type, &blocks, &used, &available, &pct, mount);
            	if (rc > 0) {
            		di->Available1kBlocks = available;
            		di->Total1kBlocks = blocks;
            		di->Used1kBlocks = used;
            		di->pctUsage = pct;
            		strcpy(di->szMountPoint, mount);
            		strcpy(di->szName, name);
            		strcpy(di->szType, type);
            	}
            	return(DCL_OK);
    		}
        }
		pclose(fp);
	}
    return(DCL_ERROR);
}
#endif

void linefeed(PARAM *dir_param)
{
    int row = terminfo.Size.Y;
    char answer[16];

    dir_param->line_counter++;
    dcl_printf(dir_param->output,"\n");
    if (dir_param->page && dir_param->line_counter >= row - 2) {
        dcl_printf(dir_param->output,"\nPress RETURN for more ");
        *answer = 0;
        kbdread(answer,16,data_stack,INFINITE_TIMEOUT);
        dir_param->line_counter = 0;
        tio_clrscr();
    }
}

void dir_printfilename(FILE *fp, char *format, char *filename)
{
//    char    vms[MAX_TOKEN];
//
//    cvfs_dos_to_vms(filename, vms);
//    dcl_printf(fp,format,vms);
    dcl_printf(fp,format,filename);
}
