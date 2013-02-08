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

#include <string.h>
#include <ctype.h>
#include "platform.h"
#include "dcl.h"

int dcl_string_assign(char *sym,char *oper)
{
    int     symlvl;
    char    work[1024];
    char    *ch = dcl_line;
    char    *w1;
    int     quoting = 0;
    int     sp = 0;

    NEXT_LINE();
    if (cmd[C].subr) return(0);
    if (!dcl[D].cc[dcl[D].ccl]) return(0);
    if (sym == NULL || oper == NULL) return(DCL_ERROR);

    *work = 0;
    w1 = work;

    if (strncmp(oper,"==",2)==0 || strncmp(oper,":==",2)==0)
        symlvl = 0;
    else
        symlvl = D;
    while (*ch && *ch != '=') ch++;
    while (*ch && *ch == '=') ch++;
    //while (*ch && *ch != ' ' && *ch != '\t') ch++;
    while (*ch && (*ch ==  ' ' || *ch == '\t')) ch++;
    sp = (int)strlen(ch) - 1;
    while (sp && (ch[sp] == ' ' || ch[sp] == '\t')) ch[sp--] = 0;

    while (*ch) {
        switch (*ch){
            case '"' :  if (quoting) {
                            if (*(ch+1)== '"')
                                *w1++ = *ch;
                            quoting = 0;
                            }
                        else {
                            quoting = 1;
                            }
                        sp = 0;
                        break;
            case '\t':  if (quoting) {
                            *w1++ = *ch;
                            sp = 0;
                            break;
                            }
                        else
                            *ch = ' ';
							/*lint -fallthrough */
            case ' ' :  if (quoting) { 
                            *w1++ = *ch;
                            sp = 0;
                            break;
                            }
                        else {
                            sp++;
                            if (sp == 1) *w1++ = *ch;
                            break;
                            }
            default  :  sp = 0;
                        if (quoting)
                            *w1++ = *ch;
                        else
                            *w1++ = (char) toupper(*ch);
                        break;
            }
        *w1 = 0;
        ch++;
        }

    (void) symbol_put(sym,work,symlvl);
    return(0);
}

int dcl_sym_assign(char *sym,char *oper)
{
    int     symlvl;
    char    work[1024];
    char    *ch = dcl_line;

    NEXT_LINE();
    if (cmd[C].subr) return(0);
    if (!dcl[D].cc[dcl[D].ccl]) return(0);
    if (sym == NULL || oper == NULL) return(DCL_ERROR);

    *work = 0;
    if (strcmp(oper,"==")==0 || strcmp(oper,":==")==0)
        symlvl = 0;
    else
        symlvl = D;
    while (*ch && *ch != '=') ch++;
    while (*ch && *ch != ' ') ch++;
    if (*ch) {
        (void) EXP_compute(ch,work);
        if (*work) 
            (void) symbol_put(sym,work,symlvl);
        else
            symbol_del_lvl(sym, symlvl);
    }
    return(0);
}
