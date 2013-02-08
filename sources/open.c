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

// Open -----------------------------------------------------
RPARAM_T OPEN_PARAM[] = {
    {1,"Logical name",MANDATORY},
    {2,"File name",MANDATORY},
    {0,"",0}
    };
RPARAM_T OPEN_QUAL[] = {
    {1,"/READ",0},
    {2,"/WRITE",0},
    {3,"/APPEND",0},
    {4,"/ERROR",VALUE},
    {5,"/LENGTH",VALUE},
    {0,"",0}
    };

long filesize(FILE *stream);

int dcl_open(PARAM_T *p,PARAM_T *q)
{
    FILE    *fp;
    char    token[256];
    char    physnam[256];
    char    lognam[256];
    char    err_label[256];
    char    len_string[256];
    char    drive[_MAX_DRIVE];
    char    dir[_MAX_DIR];
    char    fname[_MAX_FNAME];
    char    ext[_MAX_EXT];
    int     i       = 0;
    char    open_mode    = 0;
    int     filenum = 0;
    int     recl    = 0;
    long    rptr    = 0;
    long    wptr    = 0;
    int     retcod  = 0;

    NEXT_LINE();
    if (cmd[C].subr) return(0);
    if (!dcl[D].cc[dcl[D].ccl]) return(0);
    if (p == NULL || q == NULL) return(DCL_ERROR);

    _STATUS = 0;
    *token = 0; *physnam = 0; *lognam = 0; *err_label = 0;

    (void)cmd_parse_line(dcl_line,OPEN_PARAM,OPEN_QUAL,p,q);
    for (i = 0; q[i].tag; i++) {
        if (q[i].flag & PRESENT) 
            switch (q[i].tag) {
            case 1:                                 /* /READ    */
                open_mode |= OPEN_MODE_READ;
                break;
            case 2:                                 /* /WRITE    */
                open_mode |= OPEN_MODE_WRITE;
                break;
            case 3:                                 /*  /APPEND    */
                open_mode |= OPEN_MODE_APPEND;
                break;
            case 4:                                 /*  /ERROR     */
                dcl_string(q[i].value,err_label,MAX_TOKEN);
                if (*err_label == 0){
                    NEXT_LINE();
                    (void)dcl_printf(dcl[D].SYS_OUTPUT,"Invalid /ERROR argument\n");
                    _SEVERITY = 2;
                    _STATUS = 19;
                    retcod = -1;
                    goto exit_label;
                    }
                break;
            case 5:                                 /*  /LENGTH   */
                dcl_string(q[i].value,len_string,MAX_TOKEN);
                recl = atoi(len_string);
                if (recl == 0){
                    NEXT_LINE();
                    (void)dcl_printf(dcl[D].SYS_OUTPUT,"Invalid /LENGTH argument\n");
                    _SEVERITY = 2;
                    _STATUS = 19;
                    retcod = -1;
                    goto exit_label;
                    }
                break;
            default:
                ;
            } /* end switch */
    }   /* end for */

    if (open_mode == 0) {
        open_mode = OPEN_MODE_READ;
    }
    if ((open_mode & OPEN_MODE_APPEND) && (open_mode & OPEN_MODE_WRITE)) {
        (void)dcl_printf(dcl[D].SYS_OUTPUT,"/WRITE and /APPEND are mutually exclusive.\n");
        _SEVERITY = 2;
        _STATUS = 19;
        retcod = -1;
        goto exit_label;
    }

    dcl_string(p[0].value,lognam,LOGICAL_MAX_NAME);
    if (lognam[strlen(lognam)-1] == ':')
        lognam[strlen(lognam)-1] = 0;

    dcl_string(p[1].value,physnam,MAX_TOKEN);
    cvfs_vms_to_dos(physnam,token,&i);
    _splitpath(token,drive,dir,fname,ext);
    if (strlen(ext) == 0) {
        strcat(token,".DAT");
        strcat(physnam,".DAT");
    }

    if (recl) {
        switch (open_mode){
        case OPEN_MODE_APPEND:
                fp = fopen(token,"ab");
                break;
        case OPEN_MODE_WRITE:
                fp = fopen(token,"wb");
                break;
        case OPEN_MODE_READ + OPEN_MODE_WRITE:
                fp = fopen(token,"rb+");
                break;
        case OPEN_MODE_READ + OPEN_MODE_APPEND:
                fp = fopen(token,"rb+");
                break;
        default:
                fp = fopen(token,"rb");
        }
    }
    else {
    switch (open_mode){
        case OPEN_MODE_APPEND:
                fp = fopen(token,"at");
                break;
        case OPEN_MODE_WRITE:
                fp = fopen(token,"wt");
                break;
        case OPEN_MODE_READ + OPEN_MODE_WRITE:
                fp = fopen(token,"rt+");
                break;
        case OPEN_MODE_READ + OPEN_MODE_APPEND:
                fp = fopen(token,"rt+");
                break;
        default:
                fp = fopen(token,"rt");
        }
    }
    if (fp == NULL) {
        _STATUS = errno;
        _SEVERITY = 2;
        if (*err_label) {
            sprintf(dcl_line,"GOTO %s",err_label);
            retcod = dcl_process_command_line(0);
        }
        else {
            (void)dcl_printf(dcl[D].SYS_OUTPUT,"Error opening %s",physnam);
            (void)dcl_printf(dcl[D].SYS_OUTPUT,"%s\n",strerror(errno));
            retcod = DCL_ERROR;
        }
        goto exit_label;
    }

    while (dclfile[filenum]) filenum++;
    if (filenum >= 64) {
        if (*err_label) {
            sprintf(dcl_line,"GOTO %s",err_label);
            retcod = dcl_process_command_line(0);
        }
        else {
            (void)dcl_printf(dcl[D].SYS_OUTPUT,"Too much DCL files opened.\n");
            _SEVERITY = 2;
            _STATUS = 4;
            retcod = -1;
        }
        goto exit_label;
    }

    dclfile[filenum] = fp;
    rptr = 0;
    wptr = 0;
    if (open_mode & OPEN_MODE_APPEND){
        wptr = filesize(fp);
    }

    (void)logical_put_file(lognam,physnam,LOG_USER,filenum,open_mode,recl,rptr,wptr);

exit_label:
    return(retcod);
}


long filesize(FILE *stream)
{
   long curpos, length;

   if (stream == NULL) return(0);

   curpos = ftell(stream);
   fseek(stream, 0L, SEEK_END);
   length = ftell(stream);
   fseek(stream, curpos, SEEK_SET);

   return length;
}
