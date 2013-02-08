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
#include "dcltime.h"
#include "termio.h"
#include "dbglog.h"

// Type -----------------------------------------------------
RPARAM_T TYPE_PARAM[] = {
    {1,"File",MANDATORY},
    {0,"",0}
    };
RPARAM_T TYPE_QUAL[] = {
    {1,"/ALL",0},
    {2,"/BEFORE",VALUE},
    {3,"/CONFIRM",0},
    {4,"/NOCONFIRM",0},
    {5,"/PAGE",0},
    {6,"/OUTPUT",VALUE},
    {7,"/SINCE",VALUE},
    {8,"/SUBDIR",0},
    {9,"/NOSUBDIR",0},
    {10,"/NUMBERS",0},
    {0,"",0}
    };

typedef struct {
    char    all;
    char    confirm;
    char    page;
    char    wildcard;
    char    ok;
    int     file_nb;
    time_t  before;
    time_t  since;
    char    numbers;
    FILE    *output;
    } PARAM;

static int dcltype_do_it(char *path,DCL_FIND_DATA *ff,void *fn_param, char bdir);
static int dcltype_ask_more(FILE *output);
static int dcltype_sysinput(PARAM *type_param);
static int dcltype_read_sysinput(char *buf);

int dcl_type(PARAM_T *p, PARAM_T *q)
{
    PARAM type_param;
    char    vms[MAX_TOKEN];
    char    outfil[MAX_TOKEN];
    char    dos[MAX_TOKEN];
    char    drive[_MAX_DRIVE];
    char    dir[_MAX_DIR];
    char    file[_MAX_FNAME];
    char    ext[_MAX_EXT];
    int     i       = 0;
    int     recurse = 0;
    int     s_recurse = 0;
    int     retcod  = 0;
    int     f_output = 0;
    char    *type_command = "TYPE";

    NEXT_LINE();
    if (cmd[C].subr) return(0);
    if (!dcl[D].cc[dcl[D].ccl]) return(0);
    if (p == NULL || q == NULL) return(DCL_ERROR);

    _STATUS = 0;
    *vms = 0; *outfil = 0; *dos = 0;
    *drive = 0; *dir = 0; *file = 0; *ext = 0;

    type_param.all = 0;
    type_param.confirm = 0;
    type_param.page = 0;
    type_param.wildcard = 0;
    type_param.file_nb = 0;
    type_param.before = -1;
    type_param.since = 0;
    type_param.output = dcl[D].SYS_OUTPUT;
    type_param.ok = 1;
    type_param.numbers = 0;

    retcod = cmd_parse_line(dcl_line,TYPE_PARAM,TYPE_QUAL,p,q);
    if (retcod != DCL_OK) 
        goto exit_label;

    for (i = 0; q[i].tag; i++) {
        if (q[i].flag & PRESENT) 
            switch (q[i].tag) {
                case 1:                                 /* /ALL     */
                    type_param.all = TRUE;
                    break;
                case 2:                                 /* /BEFORE  */
                    if (!*q[i].value) strcpy(q[i].value,"TODAY");
                    tm_str_to_long(q[i].value,&type_param.before);
                    break;
                case 3:                                 /*  /CONFIRM */
                    type_param.confirm = 1;
                    break;
                case 4:                                 /*  /NOCONFIRM */
                    type_param.confirm = 0;
                    break;
                case 5:                                 /*  /PAGE      */
                    type_param.page = 1;
                    break;
                case 6:                                 /*  /OUTPUT    */
                    dcl_string(q[i].value,outfil,MAX_TOKEN);
                    cvfs_vms_to_dos(outfil,dos,&recurse);
                    if (!cvfs_check_device(dos)){
                        _splitpath(dos,drive,dir,file,ext);
                        if (strlen(file) == 0){
                            strcat(dos,"TYPE");
                            }
                        if (strlen(ext) == 0){
                            strcat(dos,".LIS");
                            }
                        }
                    type_param.output = fopen(dos,"at");
                    if (type_param.output == NULL) {
		                (void)dcl_printf(dcl[D].SYS_OUTPUT,"%s: %s\n",outfil,strerror(errno));
                        _SEVERITY = 2;
                        _STATUS = 19;
                        retcod = -1;
                        goto exit_label;
                        }
                    f_output = 1;
                    break;
                case 7:                                 /*  /SINCE  */
                    if (!*q[i].value) strcpy(q[i].value,"TODAY");
                    tm_str_to_long(q[i].value,&type_param.since);
                    break;
                case 8:                                 /*  /SUBDIR */
                    recurse = 1;
                    break;
                case 9:                                /*  /NOSUBDIR */
                    recurse = 0;
                    break;
                case 10:                                /*  /NUMBERS */
                    type_param.numbers = 1;
                    break;
                default:
                    ;
            } /* end switch */
    }   /* end for */

    dcl_string(p[0].value,vms,MAX_TOKEN);

    if (type_param.page && f_output) {
        (void)dcl_printf(dcl[D].SYS_OUTPUT,"/PAGE and /OUTPUT parameters are mutually exclusives.\n");
        _SEVERITY = 2;
        _STATUS = 19;
        retcod = -1;
        goto exit_label;
    }

    if (strncasecmp(vms,"SYS$INPUT",9)) {
        cvfs_lst_to_dos(vms,dos,&s_recurse);
        if (s_recurse) recurse = 1;
        type_param.wildcard = (char)wildcard(dos);
        (void)dcl_searchdirlst(type_command,dos,recurse,dcltype_do_it,&type_param);
        if (type_param.file_nb == 0){
            if (type_param.wildcard)
                (void)dcl_printf(dcl[D].SYS_OUTPUT,"No file found.\n");
            else
                (void)dcl_printf(dcl[D].SYS_OUTPUT,"File not found.\n");
            _SEVERITY = 1;
            _STATUS = 2;
            retcod = -1;
            goto exit_label;
        }
    }
    else {
        (void)dcltype_sysinput(&type_param);
    }
    
exit_label:
    if (f_output)
        fclose(type_param.output);
    if (HARDERR)
        tio_print_interrupt();

    return(retcod);
}

/********************************************************/
/*                                                      */
/*  dcltype_do_it                                       */
/*                                                      */
/*  Apply the TYPE comand on one file                   */
/*                                                      */
/********************************************************/

static int dcltype_do_it(char *path,DCL_FIND_DATA *ff,void *fn_param, char bdir)
{
    PARAM *type_param;
    FILE *fp;
    char buffer[MAX_TOKEN+2];
    char vms[MAX_TOKEN];
    char msg[MAX_TOKEN];
    int  line   = 0;
    int  row    = terminfo.Size.Y;
    int  column = terminfo.Size.X;
    int  records=0;
    int  hdrlen = 0;
    int  len    = 0;

    if (path == NULL) return(DCL_ERROR);
    if (ff == NULL) return(DCL_ERROR);
    if (fn_param == NULL) return(DCL_ERROR);

    DebugLogV(DEBUGLOG_LEVEL_DEBUG, "row=%d col=%d", row, column);

    if (bdir)
        goto exit_label;

    type_param = (PARAM *) fn_param;
    if (!type_param->all){
        if (ff->dwFileAttributes & _A_HIDDEN ||
            ff->dwFileAttributes & _A_SYSTEM)
            goto exit_label;
    }
    if ((unsigned long)ff->WriteTime < (unsigned long)type_param->since ||
        (unsigned long)ff->WriteTime > (unsigned long)type_param->before) {
        goto exit_label;
    }

    type_param->file_nb++;

    cvfs_dos_to_vms(path,vms);
    if (type_param->confirm) {
        sprintf(msg,"Type %s",vms);
        switch (dcl_confirm(msg)){
            case CONFIRM_YES    :   type_param->ok = 1;
                                    break;
            case CONFIRM_ALL    :   type_param->ok = 1;
                                    type_param->confirm = 0;
                                    break;
            case CONFIRM_QUIT   :   type_param->ok = 0;
                                    type_param->confirm = 0;
                                    break;
            default             :   type_param->ok = 0;
        }
    }
    if (!type_param->ok) {
        goto exit_label;
    }
    if (type_param->page) {
        (void) tio_clrscr();
    }
    if (type_param->wildcard) {
        (void) dcl_printf(type_param->output,"\n");
        if (HARDERR) return(-1);
        (void) dcl_printf(type_param->output,"%s",vms);
        if (HARDERR) return(-1);
        (void) dcl_printf(type_param->output,"\n");
        if (HARDERR) return(-1);
        (void) dcl_printf(type_param->output,"\n");
    }
    if ((fp = fopen(path,"rt"))==NULL) {
		(void) dcl_printf(dcl[D].SYS_OUTPUT,"%s: %s\n",vms,strerror(errno));
        _STATUS = errno;
        _SEVERITY = 2;
        goto exit_label;
    }
    else {
        while (fgets(buffer,MAX_TOKEN,fp) && !HARDERR) {
            records++;
            if (buffer[strlen(buffer)-1] != '\n')
                strcat(buffer,"\n");
            //strcat(buffer,"\r");
            if (type_param->page && line >= row - 2) {
                if (dcltype_ask_more(type_param->output) == 0)
                    goto exit_label;
               (void) tio_clrscr();
               line = 0;
            }
            if (type_param->numbers) {
                (void) dcl_printf(type_param->output,"%5.5d ",records);
                hdrlen = 6;
            }
    		DebugLogV(DEBUGLOG_LEVEL_DEBUG, "%s", buffer);
            (void) dcl_printf(type_param->output,"%s",buffer);
            if (HARDERR) {
                _SEVERITY = 2;
                break;
            }
            len = (int)strlen(buffer) + hdrlen;
            line += (len / column) + 1;
        }

        fclose(fp);

        if (type_param->page && type_param->wildcard) {
            (void) dcltype_ask_more(type_param->output);
        }
    }

exit_label:
    return(0);
}

static int dcltype_ask_more(FILE *output)
{
    char answer[2];

    (void) dcl_printf(output,"\nPress RETURN for more ");
    *answer = 0;
    (void) kbdread(answer,2,data_stack,INFINITE_TIMEOUT);
    if (CTRL_Z || CTRL_Y) return(0);
    return (1);
}

/********************************************************/
/*                                                      */
/*  dcltype_sysinput()                                  */
/*                                                      */
/*  Used when input file is sys$input                   */
/*                                                      */
/********************************************************/

static int dcltype_sysinput(PARAM *type_param)
{
    char buf[MAX_TOKEN];

    if (type_param == NULL) return(DCL_ERROR);

    if (type_param->page) {
        (void) tio_clrscr();
    }
    (void) dcltype_read_sysinput(buf);
    while (*buf && !HARDERR) {
        (void) dcl_printf(type_param->output,"%s",buf);
        if (HARDERR) break;
        (void) dcl_printf(type_param->output,"\n");
        (void) dcltype_read_sysinput(buf);
        }
    return(0);
}

static int dcltype_read_sysinput(char *buf)
{
    if (buf == NULL) return(DCL_ERROR);

    (void) memset(buf,0,MAX_TOKEN);
    if (C == 1){
        (void) kbdread(buf,MAX_TOKEN,data_stack,INFINITE_TIMEOUT);
        }
    else {
        (void) rcld(cmd[C].cmd,buf,(unsigned int)cmd[C].ln);
        if (*buf == '$')
            *buf = 0;
        else
            NEXT_LINE();
        }
    if (buf[strlen(buf)-1] == '\n' || buf[strlen(buf)-1] == '\r')
        buf[strlen(buf)-1] = 0;
    return(0);
}
