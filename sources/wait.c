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
#include "dcltime.h"
#include "termio.h"

// Wait -----------------------------------------------------
RPARAM_T WAIT_PARAM[] = {
    {1,"Time",MANDATORY},
    {0,"",0}
    };

int dcl_wait(PARAM_T *p,PARAM_T *q)
{
    char            delta[MAX_TOKEN];
    time_t          time_u = 0;
    time_t          time_stop = 0;
    time_t          tl;
    int             retcod  = 0;

    NEXT_LINE();
    if (cmd[C].subr) return(DCL_OK);
    if (!dcl[D].cc[dcl[D].ccl]) return(DCL_OK);
    if (p == NULL || q == NULL) return(DCL_ERROR);

    *delta  = 0;
    tl      = 0;

    (void) cmd_parse_line(dcl_line,WAIT_PARAM,DUMMY_QUAL,p,q);
    dcl_string(p[0].value,delta,MAX_TOKEN);
    tm_str_to_delta(delta,&time_u);
    time_stop  = time(NULL) + time_u;
    
    while(time(NULL) < time_stop && CTRL_Y == 0) {
        _sleep(1);
    }

    if (CTRL_Y) tio_print_interrupt();

    return(retcod);
}
