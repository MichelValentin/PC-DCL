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

#include "platform.h"
#include "dcl.h"

LOGICAL LOGICAL_TABLE[LOGICAL_MAX];

static int xlogical_put(char *name,char *value,int system_flag,int filenum,
                    char op_mode,int recl, long rptr, long wptr);
static int xlogical_get(char *name,char *value,int *filenum,char *op_mode,
                    int *recl, long *rptr, long *wptr);

/****************************************************************/
/*                         LOGICALS                             */
/****************************************************************/

int logical_put(char *name,char *value,int system_flag)
{
     int    filenum = 0;
     char   op_mode = 0;
     int    recl    = 0;
     long   rptr    = 0;
     long   wptr    = 0;

     return(xlogical_put(name,value,system_flag,filenum,op_mode,recl,rptr,wptr));
}

int logical_put_file(char *name,char *value,int system_flag,int filenum,char op_mode,
                     int recl, long rptr, long wptr)
{
    return(xlogical_put(name,value,system_flag,filenum,op_mode,recl,rptr,wptr));
}

static int xlogical_put(char *name,char *value,int system_flag,int filenum,char op_mode,
                        int recl, long rptr, long wptr)
{
    int     found = 0;
    int     i = 0;

    if (name == NULL)  return(DCL_ERROR);
    if (value == NULL) return(DCL_ERROR);

    while (!found && i < LOGICAL_MAX) {
        if (LOGICAL_TABLE[i].flag != 0) {
            if (strncasecmp(name,LOGICAL_TABLE[i].name,LOGICAL_MAX_NAME) == 0) {
                found = 1;
                if (LOGICAL_TABLE[i].system_flag <= (char)system_flag) {
                    strncpy(LOGICAL_TABLE[i].value,value,LOGICAL_MAX_VALUE);
                    LOGICAL_TABLE[i].system_flag = (char)system_flag;
                    LOGICAL_TABLE[i].filenum = filenum;
                    LOGICAL_TABLE[i].mode = op_mode;
                    LOGICAL_TABLE[i].recl = recl;
                    LOGICAL_TABLE[i].rptr = rptr;
                    LOGICAL_TABLE[i].wptr = wptr;
                }
                else {
                    return(-1);
                }
            }
        }
	    i++;
    }
    if (!found) {
        for (i = 0; i < LOGICAL_MAX && LOGICAL_TABLE[i].flag != 0; i++) ;
        if (i < LOGICAL_MAX) {
            strncpy(LOGICAL_TABLE[i].name,name,LOGICAL_MAX_NAME);
            strncpy(LOGICAL_TABLE[i].value,value,LOGICAL_MAX_VALUE);
            LOGICAL_TABLE[i].system_flag = (char)system_flag;
            LOGICAL_TABLE[i].filenum = filenum;
            LOGICAL_TABLE[i].mode = op_mode;
            LOGICAL_TABLE[i].recl = recl;
            LOGICAL_TABLE[i].rptr = rptr;
            LOGICAL_TABLE[i].wptr = wptr;
            LOGICAL_TABLE[i].flag = 1;
        }
        else {
			fprintf(stderr,"ERROR - LO0002\n");
            _SEVERITY = 4;
            return(-1);
        }
    }
    return(0);
}

int logical_get(char *name,char *value)
{
    int     filenum;
    char    op_mode;
    int     recl;
    long    rptr;
    long    wptr;

    (void)xlogical_get(name,value,&filenum,&op_mode,&recl,&rptr,&wptr);
    return(0);
}

void logical_get_file(char *name,char *value,int *filenum,char *op_mode,
                      int *recl, long *rptr, long *wptr)
{
    (void) xlogical_get(name,value,filenum,op_mode,recl,rptr,wptr);
}

static int xlogical_get(char *name,char *value,int *filenum,char *op_mode,
                        int *recl, long *rptr, long *wptr)
{
    int      found = 0;
    int      i = 0;

    if (name == NULL) return(DCL_ERROR);
    if (value == NULL) return(DCL_ERROR);
    if (filenum == NULL) return(DCL_ERROR);
    if (op_mode == NULL) return(DCL_ERROR);
    if (recl == NULL) return(DCL_ERROR);
    if (rptr == NULL) return(DCL_ERROR);
    if (wptr == NULL) return(DCL_ERROR);

    while (!found && i < LOGICAL_MAX) {
        if (LOGICAL_TABLE[i].flag != 0) {
            if (strncasecmp(name,LOGICAL_TABLE[i].name,LOGICAL_MAX_NAME) == 0) {
                strncpy(value,LOGICAL_TABLE[i].value,LOGICAL_MAX_VALUE);
                *filenum = LOGICAL_TABLE[i].filenum;
                *op_mode = LOGICAL_TABLE[i].mode;
                *recl = LOGICAL_TABLE[i].recl;
                *rptr = LOGICAL_TABLE[i].rptr;
                *wptr = LOGICAL_TABLE[i].wptr;
                found = 1;
            }
        }
        i++;
    }
    if (!found) {
        *value = 0;
    }
    return(0);
}

int logical_del(char *name)
{
    int     found = 0;
    int     i = 0;

    while (!found && i < LOGICAL_MAX) {
        if (LOGICAL_TABLE[i].flag != 0) {
            if (strncasecmp(LOGICAL_TABLE[i].name,name,LOGICAL_MAX_NAME) == 0) {
                if (LOGICAL_TABLE[i].system_flag != LOG_SYSTEM) {
                    (void)memset(&LOGICAL_TABLE[i], 0, sizeof(LOGICAL));
                }
                found = 1;
            }
        }
        i++;
    }
    return(0);
}
