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

int dcl_then(PARAM_T *p,PARAM_T *q)
{
    char token[MAX_TOKEN];
    char *ch;

    UNREFERENCED_PARAM(p);
    UNREFERENCED_PARAM(q);

    NEXT_LINE();
    if (cmd[C].subr) return(DCL_OK);
    if (!dcl[D].cc[dcl[D].ccl]) return(DCL_OK);
    ch = dcl_get_token(dcl_line,token);
    if (*token) {
        strcpy(dcl_line,ch);
        (void) dcl_process_command_line(2);
        }
    return(DCL_OK);
}
