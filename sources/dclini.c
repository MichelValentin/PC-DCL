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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "platform.h"
#include "dcl.h"

int  dclini_line(char *buffer);
void dclini_username(char *ch);
void dclini_defcmdfileext(char *ch);
void dclini_filespecs(char *ch);
void dclini_allowdoscmd(char *ch);
void dclini_strictchecking(char *ch);
void dclini_switchar(char *ch);

int dclini(int argc,char **argv)
{
    FILE    *fp;
    char    path[_MAX_PATH];
    char    drive[_MAX_DRIVE];
    char    dir[_MAX_DIR];
//    char    file[_MAX_FNAME];
//    char    ext[_MAX_EXT];
    char    buffer[MAX_TOKEN];
    DWORD   size;
    char    *ptr;

    UNREFERENCED_PARAM(argc);

    size = sizeof(buffer);
    strcpy(INI_NAME,"INI$USERNAME");
#ifdef _WIN32
	ptr = ptr;
    GetUserName(buffer, &size);
#else
	ptr = getlogin();
	if (ptr) {
		strcpy(buffer, ptr);
	}
	else {
		strcpy(buffer, "unknown");
	}
#endif
	if (*buffer)
        strcpy(INI_VALUE, buffer);
    else
        strcpy(INI_VALUE,"PC-DCL");
    (void)logical_put(INI_NAME,INI_VALUE,LOG_SYSTEM);

    strcpy(INI_NAME,"INI$EXT");
    strcpy(INI_VALUE,"dcl");
    (void)logical_put(INI_NAME,INI_VALUE,LOG_SYSTEM);

    strcpy(INI_NAME,"INI$FTYPE");
    strcpy(INI_VALUE,"VMS");
    (void)logical_put(INI_NAME,INI_VALUE,LOG_SYSTEM);

    strcpy(INI_NAME,"INI$DOS");
    strcpy(INI_VALUE,"YES");
    (void)logical_put(INI_NAME,INI_VALUE,LOG_SYSTEM);

    strcpy(INI_NAME,"INI$STRICT");
    strcpy(INI_VALUE,"NO");
    (void)logical_put(INI_NAME,INI_VALUE,LOG_SYSTEM);

    strcpy(INI_NAME,"INI$SWITCHAR");
    strcpy(INI_VALUE,"/");
    (void)logical_put(INI_NAME,INI_VALUE,LOG_SYSTEM);

    *buffer = 0;
#ifdef _WIN32
    ptr = getenv("HOMEDRIVE");
    if (ptr != NULL) {
        strncat(buffer, ptr, sizeof(buffer)-1);
    }
    ptr = getenv("HOMEPATH");
    if (ptr != NULL) {
        strncat(buffer, ptr, sizeof(buffer)-1);
    }
#else
    ptr = getenv("HOME");
    if (ptr != NULL) {
        strncat(buffer, ptr, sizeof(buffer)-1);
    }
#endif
    if (*buffer == 0) {
        ptr = getcwd(buffer,MAX_TOKEN-1);
    }
    if (buffer[strlen(buffer)-1] != SLASH_CHR)
        strcat(buffer,SLASH_STR);
    strcpy(INI_HOME,buffer);

#ifdef _WIN32
    strcpy(buffer, argv[0]);
#else
    ptr = realpath(argv[0],buffer);
#endif
    _splitpath(buffer,drive,dir,NULL, NULL);
    strcpy(buffer, drive);
    strcat(buffer, dir);
    if (*buffer == 0) {
        ptr = getcwd(buffer,MAX_TOKEN-1);
    }
    if (buffer[strlen(buffer)-1] != SLASH_CHR)
        strcat(buffer,SLASH_STR);
    strcpy(INI_DCL,buffer);

    strcpy(path, INI_DCL);
    strcat(path,"dcl.ini");
    if ((fp = fopen(path,"rt")) != NULL) {
        while (fgets(buffer,256,fp)) {
            (void)dclini_line(buffer);
            }
        fclose(fp);
    }

    strcpy(path, INI_HOME);
    strcat(path,"dcl.ini");
    if ((fp = fopen(path,"rt")) != NULL) {
        while (fgets(buffer,256,fp)) {
            (void)dclini_line(buffer);
            }
        fclose(fp);
    }

    return(DCL_OK);
}


int dclini_line(char *buffer)
{
    char *ch = buffer;
    if (buffer[strlen(buffer)-1]=='\n')
        buffer[strlen(buffer)-1] = 0;
    if (*buffer == '!') return(0);
    while (*ch && *ch == ' ') ch++;
    if (!*ch) return(0);
    if (strncasecmp(ch,"USERNAME",8) == 0)
        dclini_username(ch);
    else if (strncasecmp(ch,"DEFCMDFILEEXT",13) == 0)
        dclini_defcmdfileext(ch);
    else if (strncasecmp(ch,"FILESPECS",9) == 0)
        dclini_filespecs(ch);
    else if (strncasecmp(ch,"ALLOWDOSCMD",11) == 0)
        dclini_allowdoscmd(ch);
    else if (strncasecmp(ch,"STRICTCHECKING",14) == 0)
        dclini_strictchecking(ch);
    else if (strncasecmp(ch,"SWITCHAR",8) == 0)
        dclini_switchar(ch);
    else
        fprintf(stderr,"Invalid command in DCL.INI (%s)\n",ch);
    return(DCL_OK);
}

void dclini_username(char *ch)
{
    while (*ch && *ch != ' ' && *ch != '=') ch++;
    while (*ch && (*ch == ' ' || *ch == '=')) ch++;
    strcpy(INI_NAME,"INI$USERNAME");
    strcpy(INI_VALUE,ch);
    (void)logical_put(INI_NAME,INI_VALUE,LOG_SYSTEM);
}

void dclini_defcmdfileext(char *ch)
{
    while (*ch && *ch != ' ' && *ch != '=') ch++;
    while (*ch && (*ch == ' ' || *ch == '=')) ch++;
    strcpy(INI_NAME,"INI$EXT");
    strncpy(INI_VALUE,ch,3);
    INI_VALUE[3] = 0;
    (void)logical_put(INI_NAME,INI_VALUE,LOG_SYSTEM);
}

void dclini_filespecs(char *ch)
{
    while (*ch && *ch != ' ' && *ch != '=') ch++;
    while (*ch && (*ch == ' ' || *ch == '=')) ch++;
    strcpy(INI_NAME,"INI$FTYPE");
    if (strncasecmp(ch,"DOS",3)==0)
        strcpy(INI_VALUE,"DOS");
    else
        strcpy(INI_VALUE,"VMS");
    (void)logical_put(INI_NAME,INI_VALUE,LOG_SYSTEM);
}

void dclini_allowdoscmd(char *ch)
{
    while (*ch && *ch != ' ' && *ch != '=') ch++;
    while (*ch && (*ch == ' ' || *ch == '=')) ch++;
    strcpy(INI_NAME,"INI$DOS");
    if (strncasecmp(ch,"YES",3)==0)
        strcpy(INI_VALUE,"YES");
    else
        strcpy(INI_VALUE,"NO");
    (void)logical_put(INI_NAME,INI_VALUE,LOG_SYSTEM);
}

void dclini_strictchecking(char *ch)
{
    while (*ch && *ch != ' ' && *ch != '=') ch++;
    while (*ch && (*ch == ' ' || *ch == '=')) ch++;
    strcpy(INI_NAME,"INI$STRICT");
    if (strncasecmp(ch,"YES",3)==0)
        strcpy(INI_VALUE,"YES");
    else
        strcpy(INI_VALUE,"NO");
    (void)logical_put(INI_NAME,INI_VALUE,LOG_SYSTEM);
}

void dclini_switchar(char *ch)
{
    while (*ch && *ch != ' ' && *ch != '=') ch++;
    while (*ch && (*ch == ' ' || *ch == '=')) ch++;
    strcpy(INI_NAME,"INI$SWITCHAR");
    INI_VALUE[0] = *ch;
    INI_VALUE[1] = 0;
    (void)logical_put(INI_NAME,INI_VALUE,LOG_SYSTEM);
    SWITCHAR = *ch;
}

