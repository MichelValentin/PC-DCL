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
/*lint -e801 use of goto is deprecated*/
/*lint -e818 * could be declared as constant*/
#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "dcl.h"
#include "step.h"

COMMAND_T   command[] = {
    {"ALLOCATE",dcl_allocate,ALLOCATE_PARAM,ALLOCATE_QUAL},
    {"APPEND",dcl_append,APPEND_PARAM,APPEND_QUAL},
    {"ASSIGN",dcl_assign,ASSIGN_PARAM,ASSIGN_QUAL},
    {"CALL",dcl_call,CALL_PARAM,CALL_QUAL},
    {"CANCEL",dcl_cancel,DUMMY_PARAM,DUMMY_QUAL},
    {"CONTINUE",dcl_continue,DUMMY_PARAM,DUMMY_QUAL},
    {"CLOSE",dcl_close,CLOSE_PARAM,CLOSE_QUAL},
    {"CLS",dcl_cls,DUMMY_PARAM,DUMMY_QUAL},
    {"CLEAR",dcl_cls,DUMMY_PARAM,DUMMY_QUAL},
    {"COPY",dcl_copy,COPY_PARAM,COPY_QUAL},
    {"CREATE",dcl_create,CREATE_PARAM,CREATE_QUAL},
    {"DEALLOCATE",dcl_deallocate,DEALLOCATE_PARAM,DEALLOCATE_QUAL},
    {"DEASSIGN",dcl_deallocate,DEALLOCATE_PARAM,DEALLOCATE_QUAL},
    {"DEFINE",dcl_define,DEFINE_PARAM,DEFINE_QUAL},
    {"DELETE",dcl_delete,DELETE_PARAM,DELETE_QUAL},
    {"DIRECTORY",dcl_dir,DIR_PARAM,DIR_QUAL},
    {"ELSE",dcl_else,DUMMY_PARAM,DUMMY_QUAL},
    {"ENDIF",dcl_endif,DUMMY_PARAM,DUMMY_QUAL},
    {"ENDWHILE",dcl_endwhile,DUMMY_PARAM,DUMMY_QUAL},
    {"ENDSUBROUTINE",dcl_endsubroutine,DUMMY_PARAM,DUMMY_QUAL},
    {"EXIT",dcl_exit,RETURN_PARAM,DUMMY_QUAL},
    {"RESUME",dcl_resume,DUMMY_PARAM,DUMMY_QUAL},
    {"GOSUB",dcl_gosub,GOSUB_PARAM,DUMMY_QUAL},
    {"GOTO",dcl_goto,GOSUB_PARAM,DUMMY_QUAL},
    {"HELP",dcl_help,HELP_PARAM,DUMMY_QUAL},
    {"IF",dcl_if,IF_PARAM,DUMMY_QUAL},
    {"INQUIRE",dcl_inquire,INQUIRE_PARAM,INQUIRE_QUAL},
    {"LOGOUT",dcl_logout,DUMMY_PARAM,DUMMY_QUAL},
    {"ON",dcl_on,ON_PARAM,DUMMY_QUAL},
    {"OPEN",dcl_open,OPEN_PARAM,OPEN_QUAL},
    {"PRINT",dcl_print,PRINT_PARAM,PRINT_QUAL},
    {"QUIT",dcl_logout,DUMMY_PARAM,DUMMY_QUAL},
    {"READ",dcl_read,READ_PARAM,READ_QUAL},
    {"RECALL",dcl_recall,RECALL_PARAM,RECALL_QUAL},
    {"RENAME",dcl_rename,RENAME_PARAM,RENAME_QUAL},
    {"REPEAT",dcl_repeat,DUMMY_PARAM,DUMMY_QUAL},
    {"RETURN",dcl_return,RETURN_PARAM,DUMMY_QUAL},
    {"RUN",dcl_run,RUN_PARAM,DUMMY_QUAL},
    {"SET",dcl_set,SET_PARAM,SET_QUAL},
    {"SHOW",dcl_show,SHOW_PARAM,SHOW_QUAL},
    {"STEP",dcl_step,DUMMY_PARAM,DUMMY_QUAL},
    {"SUBROUTINE",dcl_subroutine,DUMMY_PARAM,DUMMY_QUAL},
    {"THEN",dcl_then,DUMMY_PARAM,DUMMY_QUAL},
    {"TYPE",dcl_type,TYPE_PARAM,TYPE_QUAL},
    {"UNTIL",dcl_until,IF_PARAM,DUMMY_QUAL},
    {"WAIT",dcl_wait,WAIT_PARAM,DUMMY_QUAL},
    {"WHILE",dcl_while,IF_PARAM,DUMMY_QUAL},
    {"WRITE",dcl_write,WRITE_PARAM,WRITE_QUAL},
    {"",NULL,NULL,NULL}
    };

// Dummy table -----------------------------------------------------
RPARAM_T DUMMY_PARAM[] = {
    {0,"",0}
    };
RPARAM_T DUMMY_QUAL[] = {
    {0,"",0}
    };

char  *cmd_find_token(char *str);
char  *cmd_get_token(char *str,char *token);
int   cmd_search_qual(RPARAM_T *qual,char *key);
void  cmd_prompt_param(PARAM_T *param);

// Cmd preprocessor
// ****************
//
// setup PARAM_T tables from RPARAM_T

int cmd_prepro(RPARAM_T *rp,RPARAM_T *rq,PARAM_T **p,PARAM_T **q)
{
    size_t  i,pmax,qmax;
    PARAM_T *wp,*wq;

    // count rp elements and allocate space
    for (pmax = 0; rp[pmax].tag; pmax++) ;
    pmax++;
    *p = (PARAM_T *) malloc(pmax * sizeof(PARAM_T));

    // count rq elements and allocate space
    for (qmax = 0; rq[qmax].tag; qmax++) ;
    qmax++;
    *q = (PARAM_T *) malloc(qmax * sizeof(PARAM_T));

    if (*p == NULL || *q == NULL) {
        (void)dcl_printf(dcl[D].SYS_OUTPUT,"FATAL ERROR - MA0078\n");
        _SEVERITY = _SEVERE_ERROR;
        return(DCL_ERROR);
        }

    wp = *p;
    wq = *q;

    //setup p
    for (i = 0; i < pmax; i++) {
        memset((PARAM_T *) &wp[i],0,sizeof(PARAM_T));
        }

    //setup q
    for (i = 0; i < qmax; i++) {
        memset((PARAM_T *) &wq[i],0,sizeof(PARAM_T));
        }
    return(DCL_OK);
}

// Parse command line
// ******************
//
// check and get parameters and qualifiers
// prompt for missing parameters

int cmd_parse_line(char *cmdline,RPARAM_T *rp,RPARAM_T *rq,PARAM_T *param,PARAM_T *qual)
{
   char *c = cmdline;
   char token[MAX_TOKEN];
   char command_name[MAX_TOKEN];
   int  i;
   int  max_param;
   int  p;                   // index to param list
   int  q;                   // index to qual  list
   int  retcod = DCL_OK;
   int  expression = 0;

   p = 0;
   q = 0;
   memset(token, 0, sizeof(token));
   memset(command_name, 0, sizeof(command_name));
   for (max_param = 0; rp[max_param].tag; max_param++) ;

   c = cmd_get_token(c,command_name);  // Get command name

   while (*(c = cmd_find_token(c)) && !expression) {
      if (*c == SWITCHAR) {
         c = cmd_get_token(c,token);
         i = cmd_search_qual(rq,token);
         switch (i) {
            case NOT_FOUND:
                (void)dcl_printf(dcl[D].SYS_OUTPUT,"ERROR - Invalid command qualifier.\n");
                _SEVERITY = 2;
                _STATUS = 19;
                return(DCL_ERROR);
            case AMBIGUOUS:
                (void)dcl_printf(dcl[D].SYS_OUTPUT,"ERROR - Ambiguous command qualifier.\n");
                _SEVERITY = 2;
                _STATUS = 19;
                return(DCL_ERROR);
            default:
                for (q = 0; qual[q].tag && qual[q].tag != rq[i].tag; q++) ;
                qual[q].tag = rq[i].tag;
                strncpy(qual[q].name,rq[i].name,32);
                qual[q].flag = rq[i].flag | PRESENT;
               if (qual[q].flag & VALUE) {
                  c = cmd_find_token(c);
                  if (*c == '=') {
                     c = &c[1];
                     c = cmd_get_token(c,qual[q].value);
                     }
                  }
            }
         }
      else {
//         if (!expression) {
            if (p < max_param) {
                param[p].tag = rp[p].tag;
                strncpy(param[p].name,rp[p].name,32);
                param[p].flag = rp[p].flag | PRESENT;
                param[p].expr = c;
                c = cmd_get_token(c,param[p].value);
                expression = param[p].flag & EXPR;
                p++;
                }
            else {
                (void)dcl_printf(dcl[D].SYS_OUTPUT,"WARNING - Extraneous parameter ignored.\n");
                _SEVERITY = 1;
                _STATUS = 19;
                break;
                }
//            }
//            else
//                c = cmd_get_token(c,token);
         }
      }

   // check if all mandatory Params are found
   // if not, prompt for them if in interactive mode, else abort

   for (i = 0; i < max_param && retcod == DCL_OK; i++) {
      if (rp[i].flag & MANDATORY && *param[i].value == 0) {
         if (mode == MODE_INTERACTIVE && D <= 1) {
            param[p].tag = rp[p].tag;
            strncpy(param[p].name,rp[p].name,32);
            cmd_prompt_param(&param[i]);
            if (CTRL_Y || CTRL_Z) retcod = DCL_ERROR;
            }
         else {
            (void)dcl_printf(dcl[D].SYS_OUTPUT,"ERROR - Missing command parameter.\n");
            _SEVERITY = 2;
            _STATUS = 19;
            retcod = DCL_ERROR;
            }
         }
      }

   return(retcod);
}

/************************************/
/*  Search command table by name    */
/*  Key = command name or abbrev.   */
/*  Returns index in table          */
/*          NOT_FOUND               */
/*          AMBIGUOUS               */
/************************************/

int cmd_search_table(char *key)
{
    unsigned int    l       = strlen(key);
    int             i       = 0;
    int             elem    = 0;
    int             count   = 0;

    while (command[i].name[0]) {
        if (strncasecmp(command[i].name,key,l) == 0) {
            elem = i;
            count++;
            }
        i++;
        }
    if (count > 1)  return(AMBIGUOUS);
    if (count == 0) return(NOT_FOUND);
    return(elem);
}

// find_token
// **********
//
// Returns a pointer to the next token in string
// (IE the next non blank character)
//

char *cmd_find_token(char *str)
{
   int i;

   for (i = 0; str[i] && (str[i] == ' ' || str[i] == '\t'); i++) ;

   return(&str[i]);
}

// get_token
// *********
//
// finds next token in string STR and copies it to string TOKEN
// returns a pointer to the character following the token in STR
//

char *cmd_get_token(char *str,char *token)
{
   char     *ch;
   char     c;
   int      i,j;
   char     quoting = 0;

   ch = cmd_find_token(str);

   if (!*ch) return(ch);

   i = j = 0;
   c = *ch;

   do {
      if (c == '"') {
         if (quoting) {
            if (ch[i+1] == '"')
               token[j++] = c;
            quoting = 0;
            }
         else
            quoting = 1;
         }
      else
         token[j++] = c;
      token[j] = 0;
      i++;
      c = ch[i];
      }
   while ((c && c != ' ' && c != SWITCHAR && c != '!' && c != '\t' && c != '=' && j < MAX_TOKEN)
         ||  (c && quoting && j < MAX_TOKEN));

   return(&ch[i]);
}

// Search_qual
// ***********
//
// search for a qualifier in qualifier table
// return index of qualifier in table or
// -1 if not found
// -2 if ambiguous key
//

int cmd_search_qual(RPARAM_T *qual,char *key)
{
    unsigned int    l       = strlen(key) - 1;
    int             i       = 0;
    int             elem    = 0;
    int             count   = 0;

    while (qual[i].tag) {
        if (strncasecmp(&qual[i].name[1],&key[1],l) == 0) {
            elem = i;
            count++;
            }
        i++;
        }
    if (count > 1)  elem = AMBIGUOUS;
    if (count == 0) elem = NOT_FOUND;

    return(elem);
}

// Prompt_param
// ************
//
// Prompt user for parameter
// Loop until input entered or CTRL-C pressed
//

void  cmd_prompt_param(PARAM_T *param)
{
    do {
       printf("_%s: ",param->name);
       (void)kbdread(param->value,MAX_TOKEN,data_stack,INFINITE_TIMEOUT);
    } while (!*param->value && !CTRL_Y && !CTRL_Z);
}
