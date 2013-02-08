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

#include "platform.h"
#include "dcl.h"

// Recall -----------------------------------------------------
RPARAM_T RECALL_PARAM[] = {
    {1,"Select",0},
    {0,"",0}
    };
RPARAM_T RECALL_QUAL[] = {
    {1,"/ALL",0},
    {2,"/ERASE",0},
    {3,"/SAVE",0},
    {0,"",0}
    };

int dcl_recall(PARAM_T *p,PARAM_T *q)
{
    char    token[MAX_TOKEN];
    char    sel[MAX_TOKEN];
    int     i       = 0;
    int     retcod  = 0;
    int     f_all   = 0;
    int     f_erase = 0;
    int     f_save  = 0;

    NEXT_LINE();
    if (cmd[C].subr) return(0);
    if (!dcl[D].cc[dcl[D].ccl]) return(0);
    if (mode != MODE_INTERACTIVE || D != 1) return(0);
    if (p == NULL || q == NULL) return(DCL_ERROR);

    (void)mkcdptr(cmd_stack,1);
    (void)deln(cmd_stack);

    *token = 0; *sel = 0;

    (void)cmd_parse_line(dcl_line,RECALL_PARAM,RECALL_QUAL,p,q);

    for (i = 0; q[i].tag; i++) {
        if (q[i].flag & PRESENT)
            switch (q[i].tag) {
                case 1:                                 /* /ALL     */
                    f_all = TRUE;
                    break;
                case 2:                                 /* /ERASE   */
                    f_erase = TRUE;
                    break;
                case 3:                                 /* /SAVE    */
                    f_save = TRUE;
                    break;
                default:
                    ;
                } /* end switch */
        }   /* end for */

    dcl_string(p[0].value,sel,MAX_TOKEN);

    if (f_all){
        (void)mkcdptr(cmd_stack,0);
        while (nextdptr(cmd_stack,token)) {
            (void) dcl_printf(dcl[D].SYS_OUTPUT,"%3d %s\n",ncur(cmd_stack),token);
            }
        goto exit_label;
    }

    if (f_save){
        FILE    *fp = fopen(history_filename, "w");
        char    buffer[1024];
        if (fp != NULL) {
            (void)mkcdptr(cmd_stack,0); //cmd_stack->nodes
            while(prevdptr(cmd_stack,buffer)) {
                fprintf(fp, "%s\n", buffer);
            }
            (void)fclose(fp);
        }
        goto exit_label;
    }

    if (f_erase){
        (void) clrFlist(cmd_stack);
        goto exit_label;
    }

    i = 1;
    if (*sel){
        i = atoi(sel);
        if (i) {
            if (i < 1 || i > MAX_KBD_STACK){
                (void) dcl_printf(dcl[D].SYS_OUTPUT,"Invalid value\n");
                _SEVERITY = 2;
                _STATUS = 19;
                retcod = -1;
                goto exit_label;
                }
            if (i > (int) nempty(cmd_stack))
                i = (int) nempty(cmd_stack);
            }
        else {
            (void) mkcdptr(cmd_stack,0);
            while (nextdptr(cmd_stack,token)) {
                if (strncasecmp(token,sel,strlen(sel)) == 0) {
                    i = (int)ncur(cmd_stack);
                    break;
                    }
                }
            if (i == 0) {
                (void) dcl_printf(dcl[D].SYS_OUTPUT,"No command found\n");
                _SEVERITY = 2;
                _STATUS = 19;
                retcod = -1;
                goto exit_label;
                }
            }
        }

    if (rcld(cmd_stack,dcl_line,(unsigned int)i)){
        recall = 1;
        }
    else {
        dcl_line[0] = 0;
        }

exit_label:
    return(retcod);
}
