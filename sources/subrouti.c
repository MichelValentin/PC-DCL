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

int dcl_subroutine(PARAM_T *p, PARAM_T *q)
{
    UNREFERENCED_PARAM(p);
    UNREFERENCED_PARAM(q);

    NEXT_LINE();
    if (cmd[C].subr) {
        (void) dcl_printf(dcl[D].SYS_OUTPUT,"Illegal SUBROUTINE nesting.\n");
        _SEVERITY = 2;
        return(-1);
        }
    if (!dcl_subr) cmd[C].subr = 1;
    return(0);
}
