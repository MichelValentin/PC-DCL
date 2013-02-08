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


#define EXP_TOK_NUMBER   0
#define EXP_TOK_STRING   1
#define EXP_TOK_UNKNOWN  2
#define EXP_TOK_LPAREN   3
#define EXP_TOK_RPAREN   4
#define EXP_TOK_PLUS     5
#define EXP_TOK_MINUS    6
#define EXP_TOK_MULT     7
#define EXP_TOK_DIVIDE   8
#define EXP_TOK_ENDT     9
#define EXP_TOK_EQ      10
#define EXP_TOK_GE      11
#define EXP_TOK_GT      12
#define EXP_TOK_LE      13
#define EXP_TOK_LT      14
#define EXP_TOK_NE      15
#define EXP_TOK_AND     16
#define EXP_TOK_OR      17
#define EXP_TOK_NOT     18
#define EXP_TOK_EQS     19
#define EXP_TOK_GES     20
#define EXP_TOK_GTS     21
#define EXP_TOK_LES     22
#define EXP_TOK_LTS     23
#define EXP_TOK_NES     24
#define EXP_TOK_CAT     25

#define EXP_ERR_ZERODIV   0
#define EXP_ERR_RPAREN    1
#define EXP_ERR_EXPECTED  2
#define EXP_ERR_SYNTAX    3
#define EXP_ERR_EOL       4

#define EXP_MAX        1024

extern char  EXP_str[EXP_MAX];
extern int   EXP_pos;
extern int   EXP_token;
extern long  EXP_number_value;
extern char  EXP_string_value[EXP_MAX];

int EXP_expression(char *result);
int EXP_term(char *result);
int EXP_factor(char *result);
int EXP_terminal(char *result);
long EXP_get_number(void);
int EXP_get_string(void);
int EXP_get_symbol(void);
int EXP_get_logical_opr(void);
int EXP_get_token(void);
int EXP_error(int errnum);
int str_reduce(char *s1, char *s2);
long EXP_octal(char *str);
long EXP_hexa(char *str);
long EXP_ipow(int i,int j);

char *EXP_compute(char *str,char *result)
{
    memset(EXP_str,0,EXP_MAX);
    strncpy(EXP_str,str,EXP_MAX-1);
    *result = 0;
    EXP_pos = 0;
    EXP_get_token();
    EXP_expression(result);
    if (EXP_token != EXP_TOK_ENDT) {
        if (EXP_token == EXP_TOK_UNKNOWN)
            EXP_error(EXP_ERR_SYNTAX);
        else
            EXP_error(EXP_ERR_EOL);
    }
    return(&str[EXP_pos]);
}

int EXP_expression(char *result)
{
    long    n = 0;
    int     oper;
    char    *s1 = (char *) malloc(EXP_MAX);
    char    *s2 = (char *) malloc(EXP_MAX);

    if (s1 == NULL || s2 == NULL) {
        free(s1); free(s2);
        fprintf(stderr,"FATAL ERROR - EX0004\n");
        return(-1);
        }
    memset(s1, 0, sizeof(s1));
    memset(s2, 0, sizeof(s2));

    EXP_term(s1);
    while (EXP_token == EXP_TOK_PLUS ||
           EXP_token == EXP_TOK_CAT  ||
           EXP_token == EXP_TOK_MINUS ||
           EXP_token == EXP_TOK_EQ ||
           EXP_token == EXP_TOK_GE ||
           EXP_token == EXP_TOK_GT ||
           EXP_token == EXP_TOK_LE ||
           EXP_token == EXP_TOK_LT ||
           EXP_token == EXP_TOK_NE ||
           EXP_token == EXP_TOK_OR ||
           EXP_token == EXP_TOK_EQS ||
           EXP_token == EXP_TOK_GES ||
           EXP_token == EXP_TOK_GTS ||
           EXP_token == EXP_TOK_LES ||
           EXP_token == EXP_TOK_LTS ||
           EXP_token == EXP_TOK_NES ||
           EXP_token == EXP_TOK_AND) {
    oper = EXP_token;
    EXP_get_token();
    switch (oper) {
        case EXP_TOK_PLUS :  EXP_term(s2);
                            if (EXP_isnumber(s1) && EXP_isnumber(s2)) {
                                n = atol(s1) + atol(s2);
                                sprintf(s1, "%ld", n);
                                }
                            else {
                                strcat(s1,s2);
                                }
                            break;
        case EXP_TOK_CAT :  EXP_term(s2);
                            strcat(s1,s2);
                            break;
        case EXP_TOK_MINUS:  EXP_term(s2);
                            if (EXP_isnumber(s1) && EXP_isnumber(s2)) {
                                n = atol(s1) - atol(s2);
                                sprintf(s1, "%ld", n);
                                }
                            else {
                                str_reduce(s1,s2);
                                }
                            break;
        case EXP_TOK_EQ   :  EXP_term(s2);
                            n = (atol(s1) == atol(s2));
                            sprintf(s1, "%ld", n);
                            break;
        case EXP_TOK_GE   :  EXP_term(s2);
                            n = (atol(s1) >= atol(s2));
                            sprintf(s1, "%ld", n);
                            break;
        case EXP_TOK_GT   :  EXP_term(s2);
                            n = (atol(s1) > atol(s2));
                            sprintf(s1, "%ld", n);
                            break;
        case EXP_TOK_LE   :  EXP_term(s2);
                            n = (atol(s1) <= atol(s2));
                            sprintf(s1, "%ld", n);
                            break;
        case EXP_TOK_LT   :  EXP_term(s2);
                            n = (atol(s1) < atol(s2));
                            sprintf(s1, "%ld", n);
                            break;
        case EXP_TOK_NE   :  EXP_term(s2);
                            n = (atol(s1) != atol(s2));
                            sprintf(s1, "%ld", n);
                            break;
        case EXP_TOK_OR   :  EXP_term(s2);
                            n = atol(s1) | atol(s2);
                            sprintf(s1, "%ld", n);
                            break;
        case EXP_TOK_AND  :  EXP_term(s2);
                            n = atol(s1) & atol(s2);
                            sprintf(s1, "%ld", n);
                            break;
        case EXP_TOK_EQS :   EXP_term(s2);
                            n = (strcasecmp(s1,s2) == 0) ? 1 : 0;
                            sprintf(s1, "%ld", n);
                            break;
        case EXP_TOK_GES :   EXP_term(s2);
                            n = (strcasecmp(s1,s2) < 0) ? 0 : 1;
                            sprintf(s1, "%ld", n);
                            break;
        case EXP_TOK_GTS :   EXP_term(s2);
                            n = (strcasecmp(s1,s2) > 0) ? 1 : 0;
                            sprintf(s1, "%ld", n);
                            break;
        case EXP_TOK_LES :   EXP_term(s2);
                            n = (strcasecmp(s1,s2) > 0) ? 0 : 1;
                            sprintf(s1, "%ld", n);
                            break;
        case EXP_TOK_LTS :   EXP_term(s2);
                            n = (strcasecmp(s1,s2) < 0) ? 1 : 0;
                            sprintf(s1, "%ld", n);
                            break;
        case EXP_TOK_NES :   EXP_term(s2);
                            n = (strcasecmp(s1,s2) == 0) ? 0 : 1;
                            sprintf(s1, "%ld", n);
                            break;
        }
    }
    strcpy(result,s1);
    free(s1); free(s2);
    return(0);
}

int EXP_term(char *result)
{
    char    *s1 = (char *) malloc(EXP_MAX);
    char    *s2 = (char *) malloc(EXP_MAX);
    long    accumulator;
    long    oper;
    long    div_temp;

    if (s1 == NULL || s2 == NULL) {
        free(s1); free(s2);
        fprintf(stderr,"FATAL ERROR - EX0003\n");
        return(-1);
        }
    *s1 = 0; *s2 = 0;

    EXP_factor(s1);
    while (EXP_token == EXP_TOK_MULT || EXP_token == EXP_TOK_DIVIDE) {
        oper = EXP_token;
        EXP_get_token();
        if (oper == EXP_TOK_MULT) {
            EXP_factor(s2);
            accumulator = atol(s1) * atol(s2);
            sprintf(s1, "%ld", accumulator);
            }
        else {
            EXP_factor(s2);
            div_temp = atol(s2);
            if (div_temp != 0) {
                accumulator = atol(s1) / div_temp;
            sprintf(s1, "%ld", accumulator);
                }
            else
                EXP_error(EXP_ERR_ZERODIV);
            }
        }
    strcpy(result,s1);
    free(s1); free(s2);
    return (0);
}

int EXP_factor(char *result)
{
    char    *s = (char *) malloc(EXP_MAX);
    long    n = 0;

    if (s == NULL) {
        fprintf(stderr,"FATAL ERROR - EX0002\n");
        return(-1);
        }
    *s = 0;

    switch(EXP_token) {
    case EXP_TOK_MINUS   :   EXP_get_token();
                            EXP_terminal(s);
                            n = -atol(s);
            				sprintf(s, "%ld", n);
                            break;
    case EXP_TOK_NOT     :   EXP_get_token();
                            EXP_terminal(s);
                            n = ~atol(s);
            				sprintf(s, "%ld", n);
                            break;
    default             :   EXP_terminal(s);
    }
    strcpy(result,s);
    free(s);
    return(0);
}

int EXP_terminal(char *result)
{
    char    *s = (char *) malloc(EXP_MAX);

    if (s == NULL) {
        fprintf(stderr,"FATAL ERROR - EX0001\n");
        return(-1);
        }
    *s = 0;

    switch (EXP_token){
        case EXP_TOK_LPAREN :   EXP_get_token();
                                EXP_expression(s);
                                if (EXP_token != EXP_TOK_RPAREN)
                                EXP_error(EXP_ERR_RPAREN);
                                break;
        case EXP_TOK_STRING :
        case EXP_TOK_NUMBER :   strcpy(s,EXP_string_value);
                                break;
        default            :  EXP_error(EXP_ERR_EXPECTED);
        }
    EXP_get_token();
    strcpy(result,s);
    free(s);
    return (0);
}

long EXP_get_number(void)
{
    long   l = 0;
    int    i = 0;

    memset(EXP_string_value,0,19);
    if (EXP_str[EXP_pos] == '%') {
        EXP_string_value[i++] = EXP_str[EXP_pos++];
        switch (EXP_str[EXP_pos]){
            case 'd':
            case 'D':   EXP_pos++;
                        i = 0;
                        while (EXP_str[EXP_pos] >= '0' && EXP_str[EXP_pos] <= '9'
                                && i <= 18) {
                            EXP_string_value[i++] = EXP_str[EXP_pos++];
                            }
                        return(atol(EXP_string_value));
            case 'x':
            case 'X':   EXP_pos++;
                        i = 0;
                        while (isxdigit((int)
                            EXP_str[EXP_pos]) && i <= 18) {
                            EXP_string_value[i++] = EXP_str[EXP_pos++];
                            }
                        l = EXP_hexa(EXP_string_value);
          				sprintf(EXP_string_value, "%ld", l);
                        return(l);
            case 'o':
            case 'O':   EXP_pos++;
                        i = 0;
                        while (EXP_str[EXP_pos] >= '0' && EXP_str[EXP_pos] <= '7'
                                && i <= 18) {
                            EXP_string_value[i++] = EXP_str[EXP_pos++];
                            }
                        l = EXP_octal(EXP_string_value);
          				sprintf(EXP_string_value, "%ld", l);
                        return(l);
            default:    return(0);
            }
        }
    while (EXP_str[EXP_pos] >= '0' && EXP_str[EXP_pos] <= '9'
            && i <= 18) {
        EXP_string_value[i++] = EXP_str[EXP_pos++];
        }
    return(atol(EXP_string_value));
}

int EXP_get_string(void)
{
    int   i = 0;
    int   loop = 1;

    memset(EXP_string_value,0,EXP_MAX);
    EXP_pos++;
    while (loop && EXP_str[EXP_pos]) {
        if (EXP_str[EXP_pos] == '"') {
            EXP_pos++;
            if (EXP_str[EXP_pos] == '"') {
                EXP_string_value[i++] = EXP_str[EXP_pos++];
                }
            else {
                loop = 0;
                }
            }
        else {
            EXP_string_value[i++] = EXP_str[EXP_pos++];
            }
        }
    return(0);
}

int EXP_get_symbol(void)
{
    char    *s = (char *) calloc(1,EXP_MAX);
    int     i = 0;

    if (s == NULL) {
        fprintf(stderr,"FATAL ERROR - EX0101\n");
        return(-1);
        }
    *s = 0;
    memset(EXP_string_value,0,EXP_MAX);
    while(isalnum((int)EXP_str[EXP_pos]) || EXP_str[EXP_pos] == '_' || EXP_str[EXP_pos] == '$') {
        s[i++] = EXP_str[EXP_pos++];
        }
    symbol_get(s,EXP_string_value);
    EXP_number_value = atol(EXP_string_value);
    free(s);
    return(0);
}


int EXP_get_logical_opr(void)
{
    int   token = EXP_TOK_UNKNOWN;

    if (strncasecmp(&EXP_str[EXP_pos],".EQ.",4) == 0) {
        token = EXP_TOK_EQ;
        EXP_pos += 4;
        }
    if (strncasecmp(&EXP_str[EXP_pos],".GE.",4) == 0) {
        token = EXP_TOK_GE;
        EXP_pos += 4;
        }
    if (strncasecmp(&EXP_str[EXP_pos],".GT.",4) == 0) {
        token = EXP_TOK_GT;
        EXP_pos += 4;
        }
    if (strncasecmp(&EXP_str[EXP_pos],".LE.",4) == 0) {
        token = EXP_TOK_LE;
        EXP_pos += 4;
        }
    if (strncasecmp(&EXP_str[EXP_pos],".LT.",4) == 0) {
        token = EXP_TOK_LT;
        EXP_pos += 4;
        }
    if (strncasecmp(&EXP_str[EXP_pos],".NE.",4) == 0) {
        token = EXP_TOK_NE;
        EXP_pos += 4;
        }
    if (strncasecmp(&EXP_str[EXP_pos],".OR.",4) == 0) {
        token = EXP_TOK_OR;
        EXP_pos += 4;
        }
    if (strncasecmp(&EXP_str[EXP_pos],".AND.",5) == 0) {
        token = EXP_TOK_AND;
        EXP_pos += 5;
        }
    if (strncasecmp(&EXP_str[EXP_pos],".NOT.",5) == 0) {
        token = EXP_TOK_NOT;
        EXP_pos += 5;
        }
    if (strncasecmp(&EXP_str[EXP_pos],".EQS.",5) == 0) {
        token = EXP_TOK_EQS;
        EXP_pos += 5;
        }
    if (strncasecmp(&EXP_str[EXP_pos],".GES.",5) == 0) {
        token = EXP_TOK_GES;
        EXP_pos += 5;
        }
    if (strncasecmp(&EXP_str[EXP_pos],".GTS.",5) == 0) {
        token = EXP_TOK_GTS;
        EXP_pos += 5;
        }
    if (strncasecmp(&EXP_str[EXP_pos],".LES.",5) == 0) {
        token = EXP_TOK_LES;
        EXP_pos += 5;
        }
    if (strncasecmp(&EXP_str[EXP_pos],".LTS.",5) == 0) {
        token = EXP_TOK_LTS;
        EXP_pos += 5;
        }
    if (strncasecmp(&EXP_str[EXP_pos],".NES.",5) == 0) {
        token = EXP_TOK_NES;
        EXP_pos += 5;
        }
    if (token == EXP_TOK_UNKNOWN) EXP_pos += 1;
    return(token);
}

int EXP_get_token(void)
{
    while (EXP_str[EXP_pos] == ' ')   EXP_pos++;
    if (isdigit((int)EXP_str[EXP_pos]) || EXP_str[EXP_pos] == '%') {
        EXP_number_value = EXP_get_number();
        EXP_token = EXP_TOK_NUMBER;
        }
    else  {
        if (EXP_str[EXP_pos] == '"') {
            EXP_get_string();
            EXP_token = EXP_TOK_STRING;
            }
        else  {
            if (isalnum((int)EXP_str[EXP_pos]) || EXP_str[EXP_pos] == '_' ||
            EXP_str[EXP_pos] == '$' || EXP_str[EXP_pos] == '"') {
                EXP_get_symbol();
                if (EXP_isnumber(EXP_string_value))
                    EXP_token = EXP_TOK_NUMBER;
                else
                    EXP_token = EXP_TOK_STRING;
                }
            else  {
                if (EXP_str[EXP_pos] == '.')
                    EXP_token = EXP_get_logical_opr();
                else  {
                    switch (EXP_str[EXP_pos]) {
                        case '(' :  EXP_token = EXP_TOK_LPAREN;
                                    break;
                        case ')' :  EXP_token = EXP_TOK_RPAREN;
                                    break;
                        case '+' :  EXP_token = EXP_TOK_PLUS;
                                    break;
                        case ',' :  EXP_token = EXP_TOK_CAT;
                                    break;
                        case '-' :  EXP_token = EXP_TOK_MINUS;
                                    break;
                        case '*' :  EXP_token = EXP_TOK_MULT;
                                    break;
                        case '/' :  EXP_token = EXP_TOK_DIVIDE;
                                    break;
                        case 0   :  EXP_token = EXP_TOK_ENDT;
                                    break;
                        default  :  EXP_token = EXP_TOK_UNKNOWN;
                                    break;
                        }
                    EXP_pos++;
                    }
                }
         }
    }
    return(0);
}

int EXP_error(int errnum)
{
   switch (errnum) {
   case EXP_ERR_ZERODIV :   fprintf(dcl[D].SYS_OUTPUT,"Division by zero\n");
                            _SEVERITY = 2;
                            break;
   case EXP_ERR_RPAREN  :   fprintf(dcl[D].SYS_OUTPUT,") expected\n");
                            _SEVERITY = 2;
                            break;
   case EXP_ERR_EXPECTED:   /*fprintf(dcl[D].SYS_OUTPUT,"Number or expression expected\n\r");*/
                            break;
   case EXP_ERR_SYNTAX  :   fprintf(dcl[D].SYS_OUTPUT,"Invalid syntax\n");
                            _SEVERITY = 2;
                            break;
   case EXP_ERR_EOL     :   fprintf(dcl[D].SYS_OUTPUT,"End of line expected\n");
                            _SEVERITY = 2;
                            break;
   default             :    fprintf(dcl[D].SYS_OUTPUT,"Unknown error\n");
                            _SEVERITY = 2;
                            break;
   }
    return(0);
}

int EXP_isnumber(char *s)
{
    char *p = s;

    if (!*p) return(0);

    while (*p) {
        if (*p == '-') {
            if (p != s)
                return(0);
            else
                p++;
            }
        else if (!isdigit((int)*p++)) return(0);
        }
    return(1);
}

int str_reduce(char *s1, char *s2)
{
    char    *c1 = s1;
    char    *c2;
    int     l;

    l = strlen(s2);
    while(*c1 && strncasecmp(c1,s2,l)) c1++;
    if (*c1) {
        c2 = c1;
        while(*c2 && l) {c2++; l--;}
        while(*c2) *c1++ = *c2++;
        *c1 = 0;
        }
    return(0);
}

long EXP_octal(char *str)
{
    long result     = 0;
    int i           = 0;
    int j           = 0;

    for (i = strlen(str)-1; i >= 0; i--) {
        result += (str[i]-48) * EXP_ipow(8,j++);
        }
    return(result);
}

long EXP_hexa(char *str)
{
    long result     = 0;
    int i           = 0;
    int j           = 0;

    for (i = strlen(str)-1; i >= 0; i--) {
        str[i] = toupper(str[i]);
        if (str[i] >= 'A' && str[i] <= 'F') {
            result += (str[i]-55) * EXP_ipow(16,j++);
            }
        else
            result += (str[i]-48) * EXP_ipow(16,j++);
        }
    return(result);
}

long EXP_ipow(int i,int j)
{
    long result = 1;

    while (j--){
        result *= i;
        }
    return(result);
}

