/****************************************************************************/
/*                                                                          */
/*   PC-DCL -  DCL command line interpreter for Windows.                    */
/*   Copyright (C) 1991-2007 by Michel Valentin                             */
/*                                                                          */
/*   This program is free software; you can redistribute it and/or modify   */
/*   it under the terms of the enclosed license.                            */
/*                                                                          */
/*   This program is distributed in the hope that it will be useful,        */
/*   but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                   */
/*                                                                          */
/****************************************************************************/
/*lint -e801 use of goto is deprecated*/
/*lint -e818 * could be declared as constant*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "platform.h"
#include "dcltime.h"

#ifndef MAX_TOKEN
#define MAX_TOKEN 260
#endif

char    *MONTH[13] = {"   ","JAN","FEB","MAR","APR","MAY","JUN","JUL",
                      "AUG","SEP","OCT","NOV","DEC"};

static int DM[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

#define DAY_DURATION (86400L)
#define HOUR_DURATION (3600L)
#define MIN_DURATION    (60L)
#define SEC_DURATION     (1L)

/********************************************************/
/*                                                      */
/*  tm_str_to_long()                                    */
/*                                                      */
/*  Convert combination time string to internal form    */
/*                                                      */
/********************************************************/

void tm_str_to_long(char *str, time_t *ttime)
{
    char    work[MAX_TOKEN];
    char    temp[MAX_TOKEN];
    char    *s;
    char    *w;
    int     i = 0;
    int     sign;
    time_t  tt;
    struct tm  *st;

//    FILETIME        ft;
    time_t  dt;

    if (str == NULL) return;
    if (ttime == NULL) return;

    time(&tt);
    st = localtime(&tt);

    *work = 0;
    strncpy(temp,str,MAX_TOKEN);
    s = temp;
    if (!*s) {
        strcpy(temp,"NOW");
        }

    while (*s && *s == ' ') s++;

/***   ABSOLUTE TIME   ***/
    if (*s != '-' && *s != '+') {
        if (strncasecmp(s,"NOW",3)==0){
            s += 3;
            goto delta_label;
        }

        if (strncasecmp(s,"TODAY",5)==0){
            st->tm_sec      = 0;
            st->tm_min      = 0;
            st->tm_hour     = 0;
            s += 5;
            goto delta_label;
        }

        if (strncasecmp(s,"YESTERDAY",9)==0){
            if (tm_leapyear(st->tm_year+1900)) DM[1]++;
            st->tm_mday--;
            if (st->tm_mday == 0){
                st->tm_mon--;
                if (st->tm_mon == -1) {
                    st->tm_mon = 11;
                    st->tm_year--;
                }
                st->tm_mday = (int)DM[st->tm_mon];
            }
            st->tm_sec      = 0;//59;
            st->tm_min      = 0;//59;
            st->tm_hour     = 0;//23;
            s += 9;
            goto delta_label;
        }

        if (strncasecmp(s,"TOMORROW",8)==0){
            if (tm_leapyear(st->tm_year+1900)) DM[1]++;
            st->tm_mday++;
            if (st->tm_mday > DM[st->tm_mon]) {
                st->tm_mday = 1;
                st->tm_mon++;
                if (st->tm_mon > 11) {
                    st->tm_mon = 0;
                    st->tm_year++;
                }
            }
            st->tm_sec      = 0;
            st->tm_min      = 0;
            st->tm_hour     = 0;
            s += 8;
            goto delta_label;
        }
/***   DAY   ***/
        w = work;
        while (*s && *s != ':' && *s != '-' && *s != '+') {
            if (*s != ' ') *w++ = *s;
            s++;
        }
        *w = 0;
        if (*s == '-')
            s++;
        else {
            *work = 0;
            s = temp;
            goto time_label;
        }
        if (*work) st->tm_mday = (WORD)atoi(work);
/***   MONTH   ***/
        w = work;
        while (*s && *s != '-' && *s != '+') {
            if (*s != ' ') *w++ = *s;
            s++;
        }
        *w = 0;
        for (i = 1; i < 13; i++ ) {
            if (strcasecmp(MONTH[i],work)==0) {
                st->tm_mon = (WORD)i - 1;
                break;
            }
        }
        if (*s && *s != '+') s++;
/***   YEAR   ***/
        w = work;
        while (*s && *s != ':' && *s != '+' && *s != '-' && *s != ' ')
            *w++ = *s++;
        *w = 0;
        if (*work) st->tm_year = atoi(work) - 1900;
        if (*s && *s == ':') s++;
time_label:
/***   HOURS   ***/
        w = work;
        while (*s && *s != ':' && *s != '+' && *s != '-') {
            if (*s != ' ') *w++ = *s;
            s++;
        }
        *w = 0;
        if (*work) st->tm_hour = atoi(work);
        if (*s && *s == ':') s++;
/***   MINUTES   ***/
        w = work;
        while (*s && *s != ':' && *s != '+' && *s != '-') {
            if (*s != ' ') *w++ = *s;
            s++;
        }
        *w = 0;
        if (*work) st->tm_min = atoi(work);
        if (*s && *s == ':') s++;
/***   SECONDS   ***/
        w = work;
        while (*s && *s != '.' && *s != '+' && *s != '-') {
            if (*s != ' ') *w++ = *s;
            s++;
        }
        *w = 0;
        if (*work) st->tm_sec = atoi(work);
/***   HUNDRETH   ***/
        w = work;
        while (*s && *s != '+' && *s != '-') s++;
    }

delta_label:

    *ttime = mktime(st);

    while (*s && *s == ' ') s++;
    if (*s) {
        if (*s == '-')
            sign = -1;
        else
            sign = 1;
        s++;
        tm_str_to_delta(s,&dt);
        dt *= (unsigned long) sign;
        *ttime += dt;
    }

}

/********************************************************/
/*                                                      */
/*  tm_tm_to_long()                                     */
/*                                                      */
/*  Convert SYSTEMTIME into internal form               */
/*                                                      */
/********************************************************/

void tm_tm_to_long(struct tm *tms, time_t *tt)
{
    *tt = mktime(tms);
}

/********************************************************/
/*                                                      */
/*  tm_long_to_tm()                                     */
/*                                                      */
/*  Convert internal form into SYSTEMTIME               */
/*                                                      */
/********************************************************/

void tm_long_to_tm(time_t *tt, struct tm *tms)
{
    struct tm *tm;
    tm = localtime(tt);
    memcpy(tms, tm, sizeof(struct tm));
}

/********************************************************/
/*                                                      */
/*  tm_tm_to_str()                                      */
/*                                                      */
/*  Convert SYSTEMTIME into character string            */
/*                                                      */
/********************************************************/

void tm_tm_to_str(struct tm  *tm, char *str)
{
    if (str != NULL && tm != NULL) {
        sprintf(str,
                "%2.2d-%s-%4.4d %2.2d:%2.2d:%2.2d.00",
                tm->tm_mday,
                MONTH[tm->tm_mon+1],
                tm->tm_year + 1900,
                tm->tm_hour,
                tm->tm_min,
                tm->tm_sec);
    }
}

/********************************************************/
/*                                                      */
/*  tm_long_to_str()                                    */
/*                                                      */
/*  Convert internal form into character string         */
/*                                                      */
/********************************************************/

void tm_long_to_str(time_t *tt, char *str)
{
    struct tm tm;

    tm_long_to_tm(tt, &tm);
    tm_tm_to_str(&tm, str);
}

/********************************************************/
/*                                                      */
/*  tm_str_to_delta()                                   */
/*                                                      */
/*  Convert delta time string into internal format      */
/*                                                      */
/********************************************************/

void tm_str_to_delta(char *str,time_t *delta)
{
    char            work[MAX_TOKEN];
    char            *s;
    char            *w;
    time_t          tt;

    if (str == NULL) return;
    if (delta == NULL) return;

    *work = 0;
    s = str;
    while (*s && *s == ' ') s++;
    tt = 0;
    *delta = 0;

    /***   DAYS   ***/
    w = work;
    if (strchr(s,'-') != NULL) {
        while (*s && *s != '-') *w++ = *s++;
        *w = 0;
        tt = (unsigned) atoi(work);
        *delta += tt * DAY_DURATION;
        if (*s) s++;
    }

    /***   HOURS   ***/
    w = work;
    while (*s && *s != ':') *w++ = *s++;
    *w = 0;
    tt = (unsigned) atoi(work);
    *delta += tt * HOUR_DURATION;

    /***   MINUTES   ***/
    if (*s) s++;
    w = work;
    while (*s && *s != ':') *w++ = *s++;
    *w = 0;
    tt = (unsigned) atoi(work);
    *delta += tt * MIN_DURATION;

    /***   SECONDS   ***/
    if (*s) s++;
    w = work;
    while (*s && *s != '.') *w++ = *s++;
    *w = 0;
    tt = (unsigned) atoi(work);
    *delta += tt * SEC_DURATION;
}

/********************************************************/
/*                                                      */
/*  tm_leapyear()                                       */
/*                                                      */
/*  Returns TRUE if a given year is a leapyear          */
/*                                                      */
/********************************************************/

int tm_leapyear(int year)
{
    return((year % 4 == 0 && year % 100 != 0) || year % 400 == 0);
}

/********************************************************/
/*                                                      */
/*  tm_dir_to_tm()                                      */
/*                                                      */
/*  Convert DOS directory date & time into internal     */
/*  format                                              */
/*                                                      */
/********************************************************/
/*
void tm_dir_to_tm(unsigned f_date,unsigned f_time,struct tm *tm_time)
{
    tm_time->tm_year = ((f_date >> 9) & 0x7f) + 80;
    tm_time->tm_mon  = ((f_date >> 5) & 0x0f) - 1;
    tm_time->tm_mday = (f_date & 0x1f);
    tm_time->tm_hour = ((f_time >> 11) & 0x1f);
    tm_time->tm_min  = (f_time >> 5) & 0x3f;
    tm_time->tm_sec  = (f_time & 0x1f) * 2;
    tm_time->tm_isdst = 0;
}
*/
