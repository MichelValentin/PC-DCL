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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "platform.h"
#include "dcl.h"
#include "termio.h"

// Write -----------------------------------------------------
RPARAM_T WRITE_PARAM[] = {
    {1,"Logical name",MANDATORY},
    {2,"Expression",EXPR},
    {0,"",0}
    };
RPARAM_T WRITE_QUAL[] = {
    {1,"/ERROR",VALUE},
    {2,"/POSITION",VALUE},
    {0,"",0}
    };

int dcl_write(PARAM_T *p, PARAM_T *q)
{
    FILE    *fp;
    char    token[MAX_TOKEN];
    char    lognam[MAX_TOKEN];
    char    expr[1024];
    char    err_label[MAX_TOKEN];
    char    work[MAX_TOKEN];
    char    temp[MAX_TOKEN];
    char    *w;
    int     i = 0;
    char    openmode;
    int     filenum;
    int     retcod  = 0;
    int     recl    = 0;
    long    rptr    = 0;
    long    wptr    = 0;
    char    line    = 0;
    char    column  = 0;
    char    *ch;

    NEXT_LINE();
    if (cmd[C].subr) return(0);
    if (!dcl[D].cc[dcl[D].ccl]) return(0);
    if (p == NULL || q == NULL) return(DCL_ERROR);

    _STATUS = 0;
    memset(token,    0, sizeof(token));
    memset(expr,     0, sizeof(expr));
    memset(lognam,   0, sizeof(lognam));
    memset(err_label,0, sizeof(err_label));
    memset(work     ,0, sizeof(work));
    memset(temp,     0, sizeof(temp));

    (void) cmd_parse_line(dcl_line,WRITE_PARAM,WRITE_QUAL,p,q);
    for (i = 0; q[i].tag; i++) {
        if (q[i].flag & PRESENT)
            switch (q[i].tag) {
                case 1:                                 /*  /ERROR     */
                    dcl_string(q[i].value,err_label,MAX_TOKEN);
                    if (*err_label == 0){
                        NEXT_LINE();
                        (void) dcl_printf(dcl[D].SYS_OUTPUT,"Invalid /ERROR argument\n");
                        _SEVERITY = 2;
                        _STATUS = 19;
                        retcod = -1;
                        goto exit_label;
                        }
                    break;
                case 2:                                 /*  /POSITION  */
                    *work = 0;
                    w = work;
                    ch = q[i].value;
                    while (*ch && (*ch == '(' || isspace((int)*ch))) ch++;
                    while (*ch && *ch != ',' && !isspace((int)*ch))
                        *w++ = *ch++;
                    *w = 0;
                    (void) EXP_compute(work,temp);
                    line = (char) atoi(temp);
                    if (line == 0) {
                        (void) dcl_printf(dcl[D].SYS_OUTPUT,"Invalid LINE POSITION qualifier\n");
                        _SEVERITY = 2;
                        _STATUS = 19;
                        retcod = DCL_ERROR;
                    }
                    line--;
                    *work = 0;
                    w = work;
                    while (*ch && (*ch == ',' || isspace((int)*ch))) ch++;
                    while (*ch && *ch != ')' && !isspace((int)*ch))
                        *w++ = *ch++;
                    *w = 0;
                    (void) EXP_compute(work,temp);
                    column = (char) atoi(temp);
                    if (column == 0) {
                        (void) dcl_printf(dcl[D].SYS_OUTPUT,"Invalid COLUMN POSITION qualifier\n");
                        _SEVERITY = 2;
                        _STATUS = 19;
                        retcod = DCL_ERROR;
                    }
                    column--;
                    break;
                default:
                    ;
                } /* end switch */
        }   /* end for */

    dcl_string(p[0].value,lognam,LOGICAL_MAX_NAME);
	if (lognam[strlen(lognam)-1] == ':')
        lognam[strlen(lognam)-1] = 0;

    if (p[1].expr != NULL)
        (void) EXP_compute(p[1].expr,expr);
    else
        *expr = 0;

    filenum = -1;

    if (strcasecmp(lognam,"SYS$INPUT")==0) {
        fp = dcl[D].SYS_INPUT;
        openmode = OPEN_MODE_READ;
        strcpy(token, lognam);
    }
    else if (strcasecmp(lognam,"SYS$OUTPUT")==0) {
        fp = dcl[D].SYS_OUTPUT;
        openmode = OPEN_MODE_WRITE;
        strcpy(token, lognam);
    }
    else {
        logical_get_file(lognam,token,&filenum,&openmode,&recl,&rptr,&wptr);

        if (filenum == -1)
            fp = NULL;
        else
            fp = dclfile[filenum];
    }


    if (fp == NULL) {
        if (!*err_label)
            (void) dcl_printf(dcl[D].SYS_OUTPUT,"File %s not open.\n",lognam);
        _SEVERITY = 2;
        _STATUS = 101;
        retcod = -1;
        goto exit_label;
    }

    if (!((openmode & OPEN_MODE_WRITE) || (openmode & OPEN_MODE_APPEND))) {
        if (!*err_label)
            (void) dcl_printf(dcl[D].SYS_OUTPUT,"File %s not open in WRITE mode.\n",lognam);
        _SEVERITY = 2;
        _STATUS = 103;
        retcod = -1;
        goto exit_label;
    }

    fseek(fp,wptr,SEEK_SET);

    if (recl){
        if (fwrite(expr,(size_t) recl,1,fp) != 1) {
            if (!*err_label)
                (void) dcl_printf(dcl[D].SYS_OUTPUT,"Error writing file %s ",lognam);
		    (void) dcl_printf(dcl[D].SYS_OUTPUT,"%s\n",strerror(errno));
            _STATUS = errno;
            _SEVERITY = 2;
            retcod = -1;
            goto exit_label;
        }
    } else {
        if (fp == stdout || fp == stderr) {
            if (line || column)
                tio_gotoxy(column,line);
            (void)tio_printf("%s\n",expr);
        } else {
            if (fputs(expr,fp) == EOF){
                if (!*err_label)
                    (void) dcl_printf(dcl[D].SYS_OUTPUT,"Error writing file %s ",lognam);
		        (void) dcl_printf(dcl[D].SYS_OUTPUT,"%s\n",strerror(errno));
                _STATUS = errno;
                _SEVERITY = 2;
                retcod = -1;
                goto exit_label;
            }
            fputc('\n',fp);
        }
    }
    wptr = ftell(fp);
    (void) logical_put_file(lognam,token,LOG_USER,filenum,openmode,recl,rptr,wptr);

exit_label:

    if (retcod == -1) {
        if (*err_label) {
            sprintf(dcl_line,"GOTO %s",err_label);
            retcod = dcl_process_command_line(0);
        }
    }
    return(retcod);
}
