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

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "platform.h"
#include "dcl.h"
#include "dclfindfile.h"
#include "termio.h"


int kbdread(char *buffer,int maxlen,Flist stack,long timeout)
{
    int  cc = 0;
    char wrk[256];

    CTRL_Y = 0;
    CTRL_Z = 0;

    if (buffer == NULL) return(0);

    (void)mkcdptr(stack,0);
    
    cc = tio_get_one_line(buffer, maxlen, timeout, stack);
    *wrk = 0;
    if (*buffer){
        (void)rcld(stack,wrk,1);
        if (strcmp(buffer,wrk)){
            (void)mkcdptr(stack,0);
            (void)insdptr(stack,buffer);
            while (nempty(stack) > MAX_KBD_STACK + 1){
                (void)mkcdptr(stack,nempty(stack));
                (void)deln(stack);
            }
        }
    }

    return(cc);
}

