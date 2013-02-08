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

// Read -----------------------------------------------------
RPARAM_T READ_PARAM[] = {
    {1,"Logical name",MANDATORY},
    {2,"Symbol",MANDATORY},
    {0,"",0}
    };
RPARAM_T READ_QUAL[] = {
    {1,"/ERROR",VALUE},
    {2,"/END_OF_FILE",VALUE},
    {3,"/RRN",VALUE},
    {0,"",0}
    };

int dcl_read(PARAM_T *p,PARAM_T *q)
{
    FILE    *fp;
    char    token[MAX_TOKEN];
    char    lognam[MAX_TOKEN];
    char    symbol[SYMBOL_MAX_NAME];
    char    record[SYMBOL_MAX_VALUE];
    char    err_label[256];
    char    eof_label[256];
    char    rrn_string[256];
    int     i = 0;
    char    open_mode;
    int     filenum;
    int     retcod = 0;
    int     recl = 0;
    long    rptr = 0;
    long    wptr = 0;
    int     rrn = 0;
    char    cc = 0;

    NEXT_LINE();
    if (cmd[C].subr) return(0);
    if (!dcl[D].cc[dcl[D].ccl]) return(0);
    if (p == NULL || q == NULL) return(DCL_ERROR);

    _STATUS = 0;
    *token = 0; *record = 0; *lognam = 0; *err_label = 0; *eof_label = 0;
    (void) cmd_parse_line(dcl_line,READ_PARAM,READ_QUAL,p,q);
    for (i = 0; q[i].tag; i++) {
        if (q[i].flag & PRESENT) 
            switch (q[i].tag) {
                case 1:                                 /*  /ERROR     */
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
                case 2:                                 /*  /EOF     */
                    dcl_string(q[i].value,eof_label,MAX_TOKEN);
                    if (*eof_label == 0){
                        NEXT_LINE();
                        (void)dcl_printf(dcl[D].SYS_OUTPUT,"Invalid /END_OF_FILE argument\n");
                        _SEVERITY = 2;
                        _STATUS = 19;
                        retcod = -1;
                        goto exit_label;
                        }
                    break;
                case 3:                                 /* /RRN      */
                    dcl_string(q[i].value,rrn_string,MAX_TOKEN);
                    if (*rrn_string == 0){
                        NEXT_LINE();
                        (void)dcl_printf(dcl[D].SYS_OUTPUT,"Invalid /RRN argument\n");
                        _SEVERITY = 2;
                        _STATUS = 19;
                        retcod = -1;
                        goto exit_label;
                        }
                    rrn = atoi(rrn_string);
                    break;
                default:
                    ;
                } /* end switch */
        }   /* end for */

    dcl_string(p[0].value,lognam,LOGICAL_MAX_NAME);
	if (lognam[strlen(lognam)-1] == ':')
        lognam[strlen(lognam)-1] = 0;

	dcl_string(p[1].value,symbol,MAX_TOKEN);

    filenum = -1;
    logical_get_file(lognam,token,&filenum,&open_mode,&recl,&rptr,&wptr);

    if (filenum == -1)
        fp = NULL;
    else
        fp = dclfile[filenum];
    
    if (strcasecmp(token,"SYS$INPUT")==0) {
        fp = dcl[D].SYS_INPUT;
        open_mode = OPEN_MODE_READ;
        }
    if (strcasecmp(token,"SYS$OUTPUT")==0) {
        fp = dcl[D].SYS_OUTPUT;
        open_mode = OPEN_MODE_READ;
        }
    
    if (fp == NULL) {
        if (!*err_label)
            (void)dcl_printf(dcl[D].SYS_OUTPUT,"File %s not open.\n",lognam);
        _SEVERITY = 2;
        _STATUS = 101;
        retcod = -1;
        goto exit_label;
        }

    if (!(open_mode & OPEN_MODE_READ)) {
        if (!*err_label)
            (void)dcl_printf(dcl[D].SYS_OUTPUT,"File %s not open in READ mode.\n",lognam);
        _SEVERITY = 2;
        _STATUS = 102;
        retcod = -1;
        goto exit_label;
        }

    if (rrn) {
        rptr = (long)(rrn - 1) * (long)recl;
        }

    wptr = rptr;


    i = 0;
    if (recl){
        (void)fseek(fp,rptr,SEEK_SET);
        cc = (char)fread(record,(size_t)recl,1,fp);
        record[recl] = 0;
        if (cc != 1){
            if (ferror(fp)){
                if (!*err_label)
                    (void)dcl_printf(dcl[D].SYS_OUTPUT,"Error reading file %s",lognam);
		        (void)dcl_printf(dcl[D].SYS_OUTPUT,"%s\n",strerror(errno));
                _STATUS = errno;
                _SEVERITY = 2;
                retcod = -1;
                goto exit_label;
                }
             else {
                 cc = EOF;
                 }
            }
        }
    else {
        if (!fgets(record,SYMBOL_MAX_VALUE,fp)){
            if (ferror(fp)){
                if (!*err_label)
                    (void) dcl_printf(dcl[D].SYS_OUTPUT,"Error reading file %s",lognam);
		        (void) dcl_printf(dcl[D].SYS_OUTPUT,"%s\n",strerror(errno));
                _STATUS = errno;
                _SEVERITY = 2;
                retcod = -1;
                goto exit_label;
            }
            else {
                cc = EOF;
            }
        }
        while (strlen(record) && 
               (record[strlen(record)-1] == '\n' || record[strlen(record)-1] == '\r'))
            record[strlen(record)-1] = 0;
    }
    if (cc == EOF) {
        if (!*eof_label)
            (void) dcl_printf(dcl[D].SYS_OUTPUT,"End of file reading %s",lognam);
        _SEVERITY = 1;
        _STATUS = 100;
        retcod = 1;
        goto exit_label;
        }

    rptr = ftell(fp);
    (void) logical_put_file(lognam,token,LOG_USER,filenum,open_mode,recl,rptr,wptr);
    (void) symbol_put(symbol,record,D);

exit_label:

    switch (retcod) {
        case -1 :   if (*err_label) {
                        sprintf(dcl_line,"GOTO %s",err_label);
                        retcod = dcl_process_command_line(0);
                        }
                    break;
        case  1 :   if (*eof_label) {
                        sprintf(dcl_line,"GOTO %s",eof_label);
                        retcod = dcl_process_command_line(0);
                        }
                    break;
        default :   ;
        }
    return(retcod);
}
