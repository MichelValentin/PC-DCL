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
#include "platform.h"
#include "dcl.h"

// return -----------------------------------------------------
RPARAM_T RETURN_PARAM[] = {
    {1,"Status code",0},
    {0,"",0}
    };

int dcl_return(PARAM_T *p, PARAM_T *q)
{
    int     i;
    char    work[MAX_TOKEN];
    int     retcod = DCL_OK;

    NEXT_LINE();
    if (cmd[C].subr) return(DCL_OK);
    if (p == NULL || q == NULL) return(DCL_ERROR);

    if (!dcl[D].cc[dcl[D].ccl]) return(DCL_OK);

    retcod = cmd_parse_line(dcl_line,RETURN_PARAM,DUMMY_QUAL,p,q);
    if ((retcod == DCL_OK) && (p[0].flag & PRESENT)) {
        (void)EXP_compute(p[0].value,work);
        _STATUS = atoi(work);
        }

    if (popd(cmd[C].stack,&i))
        cmd[C].ln = i;

    return(DCL_OK);
}
