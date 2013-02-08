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

#include <string.h>
#include <stdlib.h>

#include "platform.h"
#include "dcl.h"
#include "termio.h"

extern int hlpvms(char *help_file);
int help_sort(const void *p1,const void *p2);

#define MAX_TOPICS	256

// Help -----------------------------------------------------
RPARAM_T HELP_PARAM[] = {
    {1,"Topic",0},
    {0,"",0}
    };

int dcl_help(PARAM_T *p,PARAM_T *q)
{
    char    helpdir[MAX_TOKEN];
    char    temp[MAX_TOKEN];
    char    buffer[MAX_TOKEN];
    char    token[MAX_TOKEN];
    char	topic[MAX_TOPICS][MAX_TOKEN];
    int		count  	= 0;
    int     retcod 	= DCL_OK;
    int     recurse = 0;
    long   	handle;
    struct 	_finddata_t ffblk;
	int 	i 		= 0;
	int		pos		= 0;
	int		w		= 0;
    int		loop    = 0;

    NEXT_LINE();
    if (cmd[C].subr) return(DCL_OK);
    if (!dcl[D].cc[dcl[D].ccl]) return(DCL_OK);
    if (p == NULL || q == NULL) return(DCL_ERROR);

    _STATUS = 0;
    *buffer = 0; *token = 0;
    retcod = cmd_parse_line(dcl_line,HELP_PARAM,DUMMY_QUAL,p,q);
    if (retcod == DCL_OK) {
        dcl_string(p[0].value,token,MAX_TOKEN);
        (void) logical_get("SYS$HELPDIR",temp);
        cvfs_vms_to_dos(temp,helpdir, &recurse);
        if (helpdir[strlen(helpdir)-1] != SLASH_CHR) {
            strcat(helpdir, SLASH_STR);
        }
//        if (*token == 0) {
//        	strcpy(token, "*");
//        }
        sprintf(buffer, "%s%s*.hlp", helpdir, token);
        handle = _findfirst(buffer,&ffblk);
        while (handle != -1) {
        	_splitpath(ffblk.name, NULL, NULL, topic[count], NULL);
        	count++;
        	if (_findnext(handle, &ffblk) == -1) {
        		break;
        	}
        }
        (void)_findclose(handle);
        tio_clrscr();
		(void)dcl_printf(dcl[D].SYS_OUTPUT,"PC-DCL Version %s HELP\n\n", VERSION);
        switch (count) {
        case 0:
            (void) dcl_printf(dcl[D].SYS_OUTPUT,"Help topic not found.\n");
            _SEVERITY = 1;
            _STATUS = 2;
            retcod = DCL_ERROR;
        	break;
        case 1:
            sprintf(buffer,"%s%s", helpdir, ffblk.name);
            retcod = hlpvms(buffer);
        	break;
        default:
        	qsort(topic, count, MAX_TOKEN, help_sort);
            (void) dcl_printf(dcl[D].SYS_OUTPUT,"Help is available on :\n");
            for (i = 0; i < count; i++) {
            	w = (int)strlen(topic[i]);
            	w = w / 8;
            	w = w + 1;
            	w = w * 8;
            	if (pos + w > 72) {
            		dcl_printf(dcl[D].SYS_OUTPUT,"\n");
            		pos = 0;
            	}
            	dcl_printf(dcl[D].SYS_OUTPUT,"%-*.*s",w,w,topic[i]);
            	pos = pos + w;
            }
            dcl_printf(dcl[D].SYS_OUTPUT,"\n");
            loop = 1;
        	while(loop) {
        		dcl_printf(dcl[D].SYS_OUTPUT,"\nEnter topic you want help on, or RETURN ");
        		memset(buffer, 0, sizeof(buffer));
        		(void) kbdread(buffer,MAX_TOKEN,data_stack,0);
        		if (*buffer == 0) {
        			break;
        		}
        		for (i = 0; i < count; i++) {
        			if (strncasecmp(topic[i], buffer, strlen(buffer)) == 0) {
            			sprintf(buffer,"%s%s.hlp", helpdir, topic[i]);
            			retcod = hlpvms(buffer);
            			loop = 0;
        				break;
        			}
        		}
        	}
        }
        dcl_interprete_prompt(dcl_org_prompt,dcl_prompt);
    }

    return(retcod);
}

int help_sort(const void *p1,const void *p2)
{
    char *a = (char*) p1;
    char *b = (char*) p2;
    return(strcasecmp(a,b));
}
