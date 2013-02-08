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

#include "platform.h"
#include "dcl.h"

char    STEP_line[1024];
int     STEP = 0;
int     STEP_loop = 0;

int dcl_step(PARAM_T *p, PARAM_T *q)
{
    UNREFERENCED_PARAM(p);
    UNREFERENCED_PARAM(q);

    STEP_loop = 0;
    return(DCL_OK);
}

int dcl_resume(PARAM_T *p, PARAM_T *q)
{
    UNREFERENCED_PARAM(p);
    UNREFERENCED_PARAM(q);

    STEP_loop = 0;
    STEP = 0;
    return(DCL_OK);
}

int dcl_cancel(PARAM_T *p, PARAM_T *q)
{
    STEP_loop = 0;
    STEP = 0;
    return(dcl_exit(p,q));
}
