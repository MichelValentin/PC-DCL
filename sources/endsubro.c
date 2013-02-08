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

int dcl_endsubroutine(PARAM_T *p,PARAM_T *q)
{
    UNREFERENCED_PARAM(p);
    UNREFERENCED_PARAM(q);

    if (dcl_subr){
        (void) dcl_exit_subroutine();
    }
    else {
        NEXT_LINE();
        if (!cmd[C].subr) {
            (void) dcl_printf(dcl[D].SYS_OUTPUT,"Illegal use of ENDSUBROUTINE.\n");
            _SEVERITY = 2;
            return(DCL_ERROR);
            }
        cmd[C].subr = 0;
    }
    return(DCL_OK);
}
