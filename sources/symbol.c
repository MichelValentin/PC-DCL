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
/*lint -e818 * could be declared as constant*/
/*lint -e661 off by one */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "dcl.h"


/********************************************************/
/*                                                      */
/*  symbol_put()                                        */
/*                                                      */
/*  Create / change symbol                              */
/*                                                      */
/********************************************************/

int symbol_put(char *name,char *value,int level)
{
    SYMBOL  tmp;
    char    tname[MAX_TOKEN];
    char    work[MAX_TOKEN];
    int     found       = 0;
    int     minlen      = 0;
    int     s           = 0;
    int     i           = 0;
    int     j           = 0;
    char    type        = 0;
    int     offset      = 0;
    int     len         = 0;
    int     substring   = 0;
	int     valuelen    = 0;

    if (name == NULL) return(DCL_ERROR);
    if (value == NULL) return(DCL_ERROR);

	memset(tname, 0, sizeof(tname));
	memset(work,  0, sizeof(work));

    /* start substring handling */
    for(i = j = 0; name[i] && name[i] != '['; i++) ;
    if (name[i] == '[') {
        substring = i++;
        while(name[i] && name[i] != ',' && name[i] != ']') {
            if (isdigit((int)name[i])) {
                work[j++] = name[i];
                work[j] = 0;
            }
            i++;
        }
        offset = atoi(work);
    }
    if (name[i] == ',') {
        i++;
        j = 0;
        while(name[i] && name[i] != ']') {
            if (isdigit((int)name[i])) {
                work[j++] = name[i];
                work[j] = 0;
            }
            i++;
        }
        len = atoi(work);
    }
    if (substring)
        name[substring] = 0;
    /* end of substring handling */


    type = EXP_isnumber(value) ? SYMBOL_INTEGER : SYMBOL_STRING;
    *tname = 0;
    i = j = 0;

    while (name[i]) {
        if (name[i] == '*' && minlen == 0) {
            minlen = (int)i;
            }
        else {
            tname[j++] = name[i];
            tname[j] = 0;
            }
        i++;
        }
    if (minlen == 0) minlen = (int)strlen(tname);
    s = 0;
    i = 0;
	j = 0;
    while (!found && s <= (int)nempty(dcl[level].symbol_list)) {
        if (rcld(dcl[level].symbol_list,&tmp,(unsigned)s)) {
            if (strncasecmp(tname,tmp.name,SYMBOL_MAX_NAME) == 0) {
				if (substring) {
					valuelen = (int) strlen(tmp.value);
					while(i < offset && i < SYMBOL_MAX_VALUE) {
                        if (i >= valuelen) {
                            tmp.value[i] = ' ';
						    tmp.value[i+1] = 0;
                        }
                        i++;
					}
					while(i < offset + len  && i < SYMBOL_MAX_VALUE) {
						if (value[j]) {
							tmp.value[i++] = value[j++];
						}
						else {
                            if (i >= valuelen) {
							    tmp.value[i] = ' ';
        						tmp.value[i+1] = 0;
                            }
                            i++;
						}
					}
				}
				else {
					strncpy(tmp.value,value,SYMBOL_MAX_VALUE);
				}
                tmp.minlen = minlen;
                tmp.type = type;
                (void) putd(dcl[level].symbol_list,&tmp);
                found = 1;
                }
            }
	    s++;
        }
    if (!found) {
        strncpy(tmp.name,tname,SYMBOL_MAX_NAME);
        i = j = 0;
        if (substring) {
            while(i < offset && i < SYMBOL_MAX_VALUE) {
                tmp.value[i++] = ' ';
                tmp.value[i] = 0;
            }
            while(i < offset + len  && i < SYMBOL_MAX_VALUE) {
                if (value[j]) {
                    tmp.value[i++] = value[j++];
                }
                else {
                    tmp.value[i++] = ' ';
                }
                tmp.value[i] = 0;
            }
        }
        else {
            strncpy(tmp.value,value,SYMBOL_MAX_VALUE);
        }
        tmp.minlen = minlen;
        tmp.type = type;
        if (insdptr(dcl[level].symbol_list,&tmp) == NULL) {
			fprintf(dcl[D].SYS_OUTPUT,"ERROR - SY0002\n");
            _SEVERITY = 2;
            return(-1);
            }
        }
    return(0);
}


/********************************************************/
/*                                                      */
/*  symbol_get()                                        */
/*                                                      */
/*  retrieve symbol value                               */
/*                                                      */
/********************************************************/

int symbol_get(char *name,char *value)
{
    SYMBOL  tmp;
    int     found = 0;
    int     i = 0;
    int     l = 0;
    int     len = 0;

    if (name  == NULL) return(DCL_ERROR);
    if (value == NULL) return(DCL_ERROR);

	*value = 0;
    if (strcasecmp(name,"$STATUS")==0) {
        sprintf(value,"%d",_MSTATUS);
        found = 1;
        }
    if (strcasecmp(name,"$SEVERITY")==0) {
        sprintf(value,"%d",_MSEVERITY);
        found = 1;
        }
    while (!found && i <= (int) nempty(dcl[D].symbol_list)) {
        if (rcld(dcl[D].symbol_list,&tmp,(unsigned int)i)) {
            len = MAX(tmp.minlen,(int) strlen(name)); /*lint !e666 */
            if (strncasecmp(name,tmp.name,(size_t)len) == 0) {
                strncpy(value,tmp.value,SYMBOL_MAX_VALUE);
                found = 1;
                }
            }
		i++;
        }
    if (D > 1){
        l = D - 1;
        i = 0;
        while (l && !found && (dcl[D].symbol_scope & SYMSCO_LOCAL)) {
            while (!found && i <= (int) nempty(dcl[l].symbol_list)) {
                if (rcld(dcl[l].symbol_list,&tmp,(unsigned int)i)) {
                    len = MAX(tmp.minlen,(int) strlen(name)); /*lint !e666 */
                    if (strncasecmp(name,tmp.name,(unsigned int)len) == 0) {
                        strncpy(value,tmp.value,SYMBOL_MAX_VALUE);
                        found = 1;
                        }
                    }
		        i++;
                }
            l--;
            }
        }
    i = 0;
    if (!found && (dcl[D].symbol_scope & SYMSCO_GLOBAL)) {
        while (!found && i <= (int) nempty(dcl[0].symbol_list)) {
            if (rcld(dcl[0].symbol_list,&tmp,(unsigned int)i)) {
                len = MAX(tmp.minlen,(int) strlen(name)); /*lint !e666 */
                if (strncasecmp(name,tmp.name,(size_t)len) == 0) {
                    strncpy(value,tmp.value,SYMBOL_MAX_VALUE);
                    found = 1;
                    }
                }
		    i++;
            }
        }
    if (!found) {
        *value = 0;
        }
    return(0);
}


/********************************************************/
/*                                                      */
/*  symbol_del()                                        */
/*                                                      */
/*  delete symbol (internal)                            */
/*                                                      */
/********************************************************/

void symbol_del(char *name)
{
    SYMBOL  tmp;
    int     found = 0;
    int     i = 0;
    int     l = 0;
    int     len = 0;

    if (name == NULL) return;

    while (!found && i <= (int) nempty(dcl[D].symbol_list)) {
        if (rcld(dcl[D].symbol_list,&tmp,(unsigned int)i)) {
            len = MAX(tmp.minlen,(int) strlen(name)); /*lint !e666 */
            if (strncasecmp(name,tmp.name,(size_t)len) == 0) {
                (void) deln(dcl[D].symbol_list);
                found = 1;
                }
            }
		i++;
        }
    l = D - 1;
    while (l && !found && (dcl[D].symbol_scope & SYMSCO_LOCAL)) {
        while (!found && i <= (int) nempty(dcl[l].symbol_list)) {
            if (rcld(dcl[l].symbol_list,&tmp,(unsigned int)i)) {
                len = MAX(tmp.minlen,(int) strlen(name)); /*lint !e666 */
                if (strncasecmp(name,tmp.name,(size_t)len) == 0) {
                    (void) deln(dcl[l].symbol_list);
                    found = 1;
                    }
                }
		    i++;
            }
        l--;
        }
    if (!found && (dcl[D].symbol_scope & SYMSCO_GLOBAL)) {
        while (!found && i <= (int) nempty(dcl[0].symbol_list)) {
            if (rcld(dcl[0].symbol_list,&tmp,(unsigned int)i)) {
                len = MAX(tmp.minlen,(int) strlen(name)); /*lint !e666 */
                if (strncasecmp(name,tmp.name,(size_t)len) == 0) {
                    (void) deln(dcl[0].symbol_list);
                    found = 1;
                    }
                }
		    i++;
            }
        }
}


/********************************************************/
/*                                                      */
/*  symbol_del()                                        */
/*                                                      */
/*  delete symbol (DELETE /SYMBOL)                      */
/*                                                      */
/********************************************************/

void symbol_del_lvl(char *name,int lvl)
{
    SYMBOL  tmp;
    int     found = 0;
    int     i = 0;
    int     len = 0;

    if (name == NULL) return;

    while (!found && i <= (int) nempty(dcl[lvl].symbol_list)) {
        if (rcld(dcl[lvl].symbol_list,&tmp,(unsigned int)i)) {
            len = MAX(tmp.minlen,(int) strlen(name)); /*lint !e666 */
            if (strncasecmp(name,tmp.name,(size_t)len) == 0) {
                (void) deln(dcl[lvl].symbol_list);
                found = 1;
                }
            }
		i++;
        }
}


/********************************************************/
/*                                                      */
/*  symbol_first()/symbol_next()                        */
/*                                                      */
/*  Search symbol                                       */
/*                                                      */
/********************************************************/

void symbol_first(SYMSCAN *ss)
{
    SYMBOL  tmp;
    int     found = 0;
    int     i = 0;
    int     l = 0;

    if (ss == NULL) return;

    while (!found && l <= D){
        while (!found && i <= (int) nempty(dcl[l].symbol_list)) {
            if (rcld(dcl[l].symbol_list,&tmp,(unsigned int)i)) {
                strncpy(ss->name,tmp.name,SYMBOL_MAX_NAME);
                strncpy(ss->value,tmp.value,SYMBOL_MAX_VALUE);
                ss->level = l;
                ss->num = i;
                found = 1;
                }
		    i++;
            }
        i = 0;
        l++;
        }

    if (!found) {
        ss->name[0] = 0;
        ss->value[0] = 0;
        ss->level = l;
        ss->num = i;
        }
}

void symbol_next(SYMSCAN *ss)
{
    SYMBOL  tmp;
    int     found = 0;
    int     i = 0;
    int     l = 0;

    if (ss == NULL) return;

    l = ss->level;
    i = ss->num + 1;

    while (!found && l <= D){
        while (!found && i <= (int) nempty(dcl[l].symbol_list)) {
            if (rcld(dcl[l].symbol_list,&tmp,(unsigned int)i)) {
                strncpy(ss->name,tmp.name,SYMBOL_MAX_NAME);
                strncpy(ss->value,tmp.value,SYMBOL_MAX_VALUE);
                ss->level = l;
                ss->num = i;
                found = 1;
                }
		    i++;
            }
        l++;
        i = 0;
        }

    if (!found) {
        ss->name[0] = 0;
        ss->value[0] = 0;
        ss->level = l;
        ss->num = i;
        }
}

