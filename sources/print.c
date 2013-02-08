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

// Print -----------------------------------------------------
RPARAM_T PRINT_PARAM[] = {
    {1,"Name",MANDATORY},
    {0,"",0}
    };
RPARAM_T PRINT_QUAL[] = {
    {1,"/ALL",0},
    {2,"/BEFORE",VALUE},
    {3,"/CONFIRM",0},
    {4,"/NOCONFIRM",0},
    {5,"/COPIES",VALUE},
    {6,"/DELETE",0},
    {7,"/NODELETE",0},
    {8,"/DEVICE",VALUE},
    {9,"/FEED",0},
    {10,"/NOFEED",0},
    {11,"/QUEUE",VALUE},
    {12,"/SETUP",VALUE},
    {13,"/SINCE",VALUE},
    {14,"/SUBDIR",0},
    {15,"/NOSUBDIR",0},
    {0,"",0}
    };

typedef struct {
    char    all;
    char    confirm;
    char    delete_f;
    char    feed;
    char    wildcard;
    char    ok;
    int     file_nb;
    int     copies;
    time_t  before;
    time_t  since;
    char    setup[MAX_TOKEN];
    char    device[MAX_TOKEN];
    } PRI_PARAM;

int dclprint_do_it(char *path,DCL_FIND_DATA *ff,void *fn_param, char bdir);
int vms_print(char *vms,PRI_PARAM *pri_param);

int dcl_print(PARAM_T *p,PARAM_T *q)
{
    PRI_PARAM pri_param;
    char    token[MAX_TOKEN];
    char    vms[MAX_TOKEN];
    char    dos[MAX_TOKEN];
    int     i       = 0;
    int     recurse = 0;
    int     s_recurse = 0;
    int     retcod  = 0;
    char    *print_command = "PRINT";

    NEXT_LINE();
    if (cmd[C].subr) return(0);
    if (!dcl[D].cc[dcl[D].ccl]) return(0);
    if (p == NULL || q == NULL) return(DCL_ERROR);

    *token = 0; *vms = 0; *dos = 0;
    pri_param.all = 0;
    pri_param.confirm = 0;
    pri_param.delete_f = 0;
    pri_param.feed = 1;
    pri_param.wildcard = 0;
    pri_param.file_nb = 0;
    pri_param.copies = 1;
    pri_param.setup[0] = 0;
    strcpy(pri_param.device,"LPT1:");
    pri_param.before = -1;
    pri_param.since  = 0;
    pri_param.ok = 1;

    _STATUS = 0;
    retcod = cmd_parse_line(dcl_line,PRINT_PARAM,PRINT_QUAL,p,q);

    if (retcod == DCL_OK) {
        for (i = 0; q[i].tag; i++) {
            if (q[i].flag & PRESENT) 
                switch (q[i].tag) {
                    case 1:                                 /* /ALL     */
                        pri_param.all = TRUE;
                        break;
                    case 2:                                 /* /BEFORE  */
                        if (!*q[i].value) strcpy(q[i].value,"TODAY");
                        tm_str_to_long(q[i].value,&pri_param.before);
                        break;
                    case 3:                                 /*  /CONFIRM */
                        pri_param.confirm = 1;
                        break;
                    case 4:                                 /*  /NOCONFIRM */
                        pri_param.confirm = 0;
                        break;
                    case 5:                                 /*  /COPIES */
                        dcl_string(q[i].value,token,MAX_TOKEN);
                        if (*token)
                            pri_param.copies = atoi(token);
                        break;
                    case 6:                                 /*  /DELETE */
                        pri_param.delete_f = 1;
                        break;
                    case 7:                                 /*  /NODELETE */
                        pri_param.delete_f = 0;
                        break;
                    case 8:                                 /*  /DEVICE */
                    case 11:                                /*  /QUEUE  */
                        dcl_string(q[i].value,pri_param.device,MAX_TOKEN);
                        break;
                    case 9:                                 /*  /FEED   */
                        pri_param.feed = 1;
                        break;
                    case 10:                                /*  /NOFEED */
                        pri_param.feed = 0;
                        break;
                    case 12:                                /*  /SETUP  */
                        dcl_string(q[i].value,pri_param.setup,MAX_TOKEN);
                        break;
                    case 13:                                 /*  /SINCE  */
                        if (!*q[i].value) strcpy(q[i].value,"TODAY");
                        tm_str_to_long(q[i].value,&pri_param.since);
                        break;
                    case 14:                                 /*  /SUBDIR */
                        recurse = 1;
                        break;
                    case 15:                                /*  /NOSUBDIR */
                        recurse = 0;
                        break;
                    default:
                        ;
                    } /* end switch */
            }   /* end for */

        dcl_string(p[0].value,vms,MAX_TOKEN);
        cvfs_lst_to_dos(vms,dos,&s_recurse);
        if (s_recurse) recurse = 1;
        pri_param.wildcard = (char)wildcard(dos);
        (void) dcl_searchdirlst(print_command,dos,recurse,dclprint_do_it,&pri_param);
        if (pri_param.file_nb == 0){
            if (pri_param.wildcard)
                (void) dcl_printf(dcl[D].SYS_OUTPUT,"No file found.\n");
            else
                (void) dcl_printf(dcl[D].SYS_OUTPUT,"File not found.\n");
            _SEVERITY = 1;
            _STATUS = 2;
            retcod = -1;
            }
        }

    return(retcod);
}


//============================================================================
int dclprint_do_it(char *path,DCL_FIND_DATA *ff,void *fn_param, char bdir)
{
    PRI_PARAM *pri_param;
    char temp[MAX_TOKEN];
    char vms[MAX_TOKEN];
    int  retcod = 0;

    if (path == NULL) return(DCL_ERROR);
    if (ff == NULL) return(DCL_ERROR);
    if (fn_param == NULL) return(DCL_ERROR);

    pri_param = (PRI_PARAM *) fn_param;
    *temp = 0; *vms = 0;

    if (bdir)
        goto exit_label;

    if (!pri_param->all){
        if (ff->dwFileAttributes & _A_HIDDEN ||
            ff->dwFileAttributes & _A_SYSTEM)
            goto exit_label;
    }
    if ((unsigned long)ff->WriteTime < (unsigned long)pri_param->since ||
        (unsigned long)ff->WriteTime > (unsigned long)pri_param->before) {
        goto exit_label;
    }

    pri_param->file_nb++;

    cvfs_dos_to_vms(path,vms);
    if (pri_param->confirm) {
        sprintf(temp,"Print %s",vms);
        switch (dcl_confirm(temp)){
            case CONFIRM_YES    :   pri_param->ok = 1;
                                    break;
            case CONFIRM_ALL    :   pri_param->ok = 1;
                                    pri_param->confirm = 0;
                                    break;
            case CONFIRM_QUIT   :   pri_param->ok = 0;
                                    pri_param->confirm = 0;
                                    break;
            default             :   pri_param->ok = 0;
            }
    }
    if (!pri_param->ok) {
        goto exit_label;
    }

    retcod = vms_print(vms,pri_param);

exit_label:
    return(retcod);
}

//============================================================================
int vms_print(char *vms,PRI_PARAM *pri_param)
{
    FILE *fp1 = NULL;
    FILE *fp2 = NULL;
    FILE *fp3 = NULL;
    char dos1[MAX_TOKEN];
    char dos2[MAX_TOKEN];
    char dos3[MAX_TOKEN];
    int  i = 0;
    int  c = 0;

    if (vms == NULL) return(DCL_ERROR);
    if (pri_param == NULL) return(DCL_ERROR);

    cvfs_vms_to_dos(vms,dos1,&i);
    (void) cvfs_check_device(dos1);
    cvfs_vms_to_dos(pri_param->device,dos2,&i);
    (void) cvfs_check_device(dos2);
    cvfs_vms_to_dos(pri_param->setup,dos3,&i);
    (void) cvfs_check_device(dos3);

    for (c = 0; c < pri_param->copies; c++) {
        i = 0;
        if ((fp1 = fopen(dos1,"rt")) == NULL){
		    (void) dcl_printf(dcl[D].SYS_OUTPUT,"%s: %s\n",vms,strerror(errno));
            _STATUS = errno;
            _SEVERITY = 2;
            i = -1;
            goto exit_label;
        }

        if ((fp2 = fopen(dos2,"at"))== NULL) {
		    (void) dcl_printf(dcl[D].SYS_OUTPUT,"%s: %s\n",pri_param->device,strerror(errno));
            pri_param->ok = 0;
            pri_param->confirm = 0;
            _STATUS = errno;
            _SEVERITY = 2;
            i = -1;
            goto exit_label;
        }

        if (pri_param->setup[0]){
            if ((fp3 = fopen(dos3,"rb"))== NULL) {
		        (void) dcl_printf(dcl[D].SYS_OUTPUT,"%s: %s\n",pri_param->setup,strerror(errno));
                _STATUS = errno;
                pri_param->ok = 0;
                pri_param->confirm = 0;
                _SEVERITY = 2;
                i = -1;
                goto exit_label;
            }
            while (!feof(fp3) && !HARDERR)
                fputc(fgetc(fp3),fp2);
            fclose(fp3);
        }

        if (pri_param->feed && !HARDERR)
            fputc('\f',fp2);

        (void) setvbuf(fp1,NULL,_IOFBF,32767);

        while (!feof(fp1) && !HARDERR)
            fputc(fgetc(fp1),fp2);

        fclose(fp1);
        fclose(fp2);
    }

    if (pri_param->delete_f && ! HARDERR)
        if (remove(dos1)) {
		    (void) dcl_printf(dcl[D].SYS_OUTPUT,"%s: %s\n",vms,strerror(errno));
            _STATUS = errno;
            _SEVERITY = 2;
            i = -1;
            goto exit_label;
        }

exit_label:
    fclose(fp1);
    fclose(fp2);
    return(i);
}
