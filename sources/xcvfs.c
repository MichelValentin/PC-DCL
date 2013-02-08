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
/*lint -e661 off by one */

#include <string.h>
#include <unistd.h>

#include "platform.h"
#include "dcl.h"
#include "error.h"

/*static char *cvfs_get_node(char *buf, char *node);*/
static char *cvfs_get_drive(char *buf, char *drive);
static char *cvfs_get_device(char *buf, char *device);
static char *cvfs_get_path(char *buf, char *path, int *recurse);
static char *cvfs_get_file(char *buf, char *file);
static char *cvfs_get_ext(char *buf, char *ext);


/********************************************************/
/*                                                      */
/*  cvfs_dos_to_vms()                                   */
/*                                                      */
/*  Convert file specification from internal (DOS)      */
/*  format to external (VMS or DOS) format.             */
/*                                                      */
/********************************************************/

void cvfs_dos_to_vms(char *internal,char *external)
{
    char    drive[MAX_TOKEN];
    char    path[_MAX_PATH];
    char    file[_MAX_FNAME];
    char    ext[_MAX_EXT];
    char    *v;
    char    *d;
    int     i = 0;
    int     dummy = 0;

    if (internal == NULL) return;
    if (external == NULL) return;

    memset(drive,0,sizeof(drive));
    memset(path,0,sizeof(path));
    memset(file,0,sizeof(file));
    memset(ext,0,sizeof(ext));

    (void) logical_get("INI$FTYPE",INI_VALUE);
    if (INI_VALUE[0] != 'V') {
        strcpy(external,internal);
        return;
    }

    d = cvfs_get_drive(internal,drive);
    d = cvfs_get_path(d,path,&dummy);
    d = cvfs_get_file(d,file);
    d = cvfs_get_ext(d,ext);
    v = external;

    d = drive;
    while (*d) {
        *v++ = *d++;
        }
    d = path;
    while (*d) {
        if (i==0) {
            *v++ = '[';
            if (*d != SLASH_CHR && *d != '.') {
                *v++ = '.';
            }
        }
        switch (*d){
        case SLASH_CHR :  if (i != 0) *v++ = '.';
                           break;
        case '.'        :  if (*(d+1) == '.') {
                               *v++ = '-';
                               d++;
                           }
                           break;
        default         :  *v++ = *d;
        }
        d++;
        i++;
    }
    //if (v != external) {
        if (*(v-1) == '.') *(v-1) = ']';
        if (*(v-1) == '[') *v++ = ']';
    //}
    d = file;
    while (*d) {
        *v++ = *d++;
    }
    d = ext;
    while (*d) {
        *v++ = *d++;
    }

    *v=0;
}

/********************************************************/
/*                                                      */
/*  cvfs_list_to_dos()                                  */
/*                                                      */
/*  Convert file list specification from external       */
/*  (VMS or DOS) format to internal (DOS) format.       */
/*                                                      */
/********************************************************/

void cvfs_lst_to_dos(char *lst_external,char *lst_internal,int *lst_recurse)
{
    char    external[_MAX_PATH];
    char    internal[_MAX_PATH];
    int     recurse = 0;
    int     i   = 0;
    int     j   = 0;

    if (lst_external == NULL) return;
    if (lst_internal == NULL) return;
    if (lst_recurse  == NULL) return;

    *external     = 0;
    *internal     = 0;
    *lst_internal = 0;

    for (i = 0; lst_external[i]; i++) {
        if (lst_external[i] == ',') {
            cvfs_vms_to_dos(external, internal, &recurse);
            strcat(lst_internal, internal);
            strcat(lst_internal, ",");
            if (*lst_recurse == 0) *lst_recurse = recurse;
            *external = 0;
            j = 0;
        }
        else {
            external[j++] = lst_external[i];
            external[j] = 0;
        }
    }
    cvfs_vms_to_dos(external, internal, &recurse);
    strcat(lst_internal, internal);
    if (*lst_recurse == 0) *lst_recurse = recurse;
}

/********************************************************/
/*                                                      */
/*  cvfs_vms_to_dos()                                   */
/*                                                      */
/*  Convert file specification from external            */
/*  (VMS or DOS) format to internal (DOS) format.       */
/*                                                      */
/********************************************************/

void cvfs_vms_to_dos(char *external,char *internal,int *recurse)
{
    char    temp[_MAX_PATH];
    char    work[_MAX_PATH];
    char    device[MAX_TOKEN];
    char    path[_MAX_PATH];
    char    file[_MAX_FNAME];
    char    ext[_MAX_EXT];
    char    *x;
    char    *w1;
    int     i   = 0;

    if (external == NULL) return;
    if (internal == NULL) return;
    if (recurse  == NULL) return;

    memset(temp,0,sizeof(temp));
    memset(work,0,sizeof(work));
    memset(device,0,sizeof(device));
    memset(path,0,sizeof(path));
    memset(file,0,sizeof(file));
    memset(ext,0,sizeof(ext));

    /* Save input buffer (skip node name and version) */
    for (x = external; *x; x++) {
        if (*x== ':' && *(x+1) == ':') {
            x += 2;
            break;
            }
    }
    if (!*x) x = external;      /* no node name */
    for (i = 0; *x && *x != ';' && i < MAX_TOKEN; temp[i++] = *x++) ;

    /* Perform a first attempts to translate logical name */
    (void) cvfs_translate_logical(temp);

    /* Replace <> by [] */
    for (x = temp; *x; x++) {
        if (*x == '<') *x = '[';
        if (*x == '>') *x = ']';
    }

    /* Extract device name and check if logical name */
    x = cvfs_get_device(temp,device);
    if (cvfs_translate_logical(device)) {
        strncpy(work,device,MAX_TOKEN);
        w1 = cvfs_get_device(work,device);
        w1 = cvfs_get_path(w1,path,recurse);
    }
    /* Extract path name */
    x = cvfs_get_path(x,work,recurse);
    if (strlen(path) + strlen(work) > MAX_TOKEN)
        LOG_ERROR(ERR_2LONG);
    strncat(path,work,_MAX_PATH-1);
    /* Extract file name */
    x = cvfs_get_file(x,file);
    /* Extract extension */
    x = cvfs_get_ext(x,ext);

    sprintf(internal,"%s%s%s%s",device,path,file,ext);
}

/********************************************************/
/*                                                      */
/*  cvfs_translate_logical()                            */
/*                                                      */
/*  attempts a logical name translation                 */
/*  returns 1 if logical name found, 0 if unchanged     */
/*                                                      */
/********************************************************/

int cvfs_translate_logical(char *s)
{
    char    name[LOGICAL_MAX_VALUE];
    char    value[LOGICAL_MAX_VALUE];
    char    save[LOGICAL_MAX_VALUE];
    int     i = 0;
    size_t  j;

    if (s == NULL) return(0);

    strncpy(name,s,LOGICAL_MAX_VALUE);
    strncpy(save,s,LOGICAL_MAX_VALUE);
    do {
        (void) logical_get(name,value);
        if (!*value) {
            j = *name ? strlen(name)-1 : 0;
            if (name[j] == ':')
                name[j] = 0;
            (void) logical_get(name,value);
        }
        if (*value) {
            strncpy(save,value,LOGICAL_MAX_VALUE);
            strncpy(name,value,LOGICAL_MAX_VALUE);
        }
        i++;
    }
    while (*value != 0 && i < 10);
    strncpy(s,save,_MAX_PATH);
    return(*s ? 1 : 0);
}


/********************************************************/
/*                                                      */
/*  cvfs_check_device()                                 */
/*                                                      */
/*  Check if filename is a reserved device name         */
/*  returns TRUE or FALSE                               */
/*                                                      */
/********************************************************/

#define DEVNUM 10
const char *dev[DEVNUM] = {"LPT1:","LPT2:","LPT3:",
                           "COM1:","COM2:","COM3:","COM4:",
                           "CON:","AUX:","PRN:"};

int cvfs_check_device(char *name)
{
    int i;

    if (name == NULL) return FALSE;

    for (i = 0; i < DEVNUM; i++) {
        if (strcasecmp(name,dev[i])==0) {
            name[strlen(name)-1] = 0;
            return(TRUE);
            }
        }
    return(FALSE);
}

/********************************************************/
/*                                                      */
/*  cvfs_get_device()                                   */
/*                                                      */
/*  Extract device name from buffer                     */
/*  Returns pointer to character following the          */
/*  device name                                         */
/*                                                      */
/********************************************************/

static char *cvfs_get_device(char *buf,char *device)
{
    int     i   = 0;
    char    *ch = NULL;

    if (buf == NULL) return(NULL);
    if (device == NULL) return(NULL);

    memset(device,0,MAX_TOKEN);
    for (ch = buf; *ch && *ch != ':'; ch++) {
        if (i < MAX_TOKEN - 1) device[i] = *ch;
        i++;
        }
    if (*ch == ':') {
        if (i < MAX_TOKEN - 1) device[i++] = *ch++;
        }
    else {
        ch = buf;
        *device = 0;
        }
    if (i > MAX_TOKEN)
        LOG_ERROR(ERR_2LONG);

    return(ch);
}
/********************************************************/
/*                                                      */
/*  cvfs_get_drive()                                    */
/*                                                      */
/*  Extract drive name from buffer                      */
/*  Returns pointer to character following the          */
/*  drive name                                          */
/*                                                      */
/********************************************************/

static char *cvfs_get_drive(char *buf,char *drive)
{
    int     i   = 0;
    char    *ch = NULL;

    if (buf == NULL) return(NULL);
    if (drive == NULL) return(NULL);

    memset(drive,0,_MAX_DRIVE);
    for (ch = buf; *ch && *ch != ':'; ch++) {
        i++;
    }
    if (*ch == ':') {
        drive[0] = *buf;
        drive[1] = ':';
        ch++;
    }
    else {
        ch = buf;
        i = 0;
    }
    if (i > 1)
        LOG_ERROR(ERR_2LONG);

    return(ch);
}

/********************************************************/
/*                                                      */
/*  cvfs_get_path()                                     */
/*                                                      */
/*  Extract path name from buffer                       */
/*  Returns pointer to character following the          */
/*  path name                                           */
/*                                                      */
/********************************************************/

static char *cvfs_get_path(char *buf,char *path,int *recurse)
{
    int     loop    = TRUE;
    int        i       = 0;
    int     point   = 0;
    char    *ch     = NULL;
    char    root_flag = 0;

    if (buf == NULL) return(NULL);
    if (path == NULL) return(NULL);
    if (recurse == NULL) return(NULL);

    ch = buf;
    *recurse = 0;

    if (*ch == '[') {        /* VMS style **********************************/
        while (*ch && loop) {
            if (i < _MAX_PATH) {
                switch (*ch) {
                case '[':
                    if (i == 0) {
                        if (ch[1] != ']' && ch[1] != '.') {
                            path[i++] = SLASH_CHR;
                        }
                    }
                    else
                        path[i++] = *ch;
                    ch++;
                    break;
                case ']' :
                    if (i && path[i-1] != SLASH_CHR)
                        path[i++] = SLASH_CHR;
                    ch++;
                    loop = 0;
                    break;
                case '.' :
                    if (point == 0)
                        if (strncmp(ch,"...",3)){
                            if (i == 1 && root_flag == 0) {
                                i = 0;
                                path[i] = 0;
                                }
                            else
                                if (i && path[i-1] != SLASH_CHR)
                                    path[i++] = SLASH_CHR;
                            }
                    point++;
                    ch++;
                    break;
                case '-' :
                    if (i == 1) {
                        i = 0;
                        path[i++] = '.';
                        path[i++] = '.';
                    }
                    else {
                        path[i++] = *ch;
                    }
                    ch++;
                    break;
                case '0' :
                    if (strncmp(ch,"000000",6)==0 && i==1) {
                        if (path[0] != SLASH_CHR) path[i++] = SLASH_CHR;
                        ch += 6;
                        root_flag = 1;
                        }
                    else
                        path[i++] = *ch++;
                    break;
                default  :
                    path[i++] = *ch++;
                    point = 0;
                }
            }
            else {
                loop = 0;
            }
        } /* end while */
        if (point == 3) *recurse = 1;
    }
    else {                  /* DOS style ***********************************/
        for (; *ch; ch++)
            if (*ch == SLASH_CHR) point++ ;    /* count (back)slashes */
        for (ch = buf; *ch && point && i < _MAX_PATH; ch++) {
            path[i++] = *ch;
            if (*ch == SLASH_CHR)
                point--;
        }
    }
    path[i] = 0;

    if (i == _MAX_PATH)
        LOG_ERROR(ERR_2LONG);

    return(ch);
}

/********************************************************/
/*                                                      */
/*  cvfs_get_file()                                     */
/*                                                      */
/*  Extract file name from buffer                       */
/*  Returns pointer to character following the          */
/*  file name                                           */
/*                                                      */
/********************************************************/

static char *cvfs_get_file(char *buf,char *file)
{
    int  i   = 0;
    char *ch = NULL;

    if (buf == NULL) return NULL;
    if (file == NULL) return NULL;

    for (ch = buf; *ch && *ch != '.'; ch++) {
        if (i < _MAX_FNAME - 1) file[i] = *ch;
        i++;
    }
    file[i] = 0;

    return(ch);
}

/********************************************************/
/*                                                      */
/*  cvfs_get_ext()                                      */
/*                                                      */
/*  Extract ext name from buffer                        */
/*  Returns pointer to character following the          */
/*  ext name                                            */
/*                                                      */
/********************************************************/

static char *cvfs_get_ext(char *buf,char *ext)
{
    int  i   = 0;
    char *ch = NULL;

    if (buf == NULL) return NULL;
    if (ext == NULL) return NULL;

    for (ch = buf; *ch; ch++) {
        if (i < _MAX_EXT - 1) ext[i] = *ch;
        i++;
    }
    ext[i] = 0;

    return(ch);
}

int cvfs_cmp_file_name(char *pat, char *str)
{
    char    pat_file[_MAX_FNAME];
    char    pat_ext[_MAX_EXT];
    char    str_file[_MAX_FNAME];
    char    str_ext[_MAX_EXT];

    _splitpath(pat,NULL, NULL, pat_file, pat_ext);
    _splitpath(str,NULL, NULL, str_file, str_ext);

    if (!*pat_ext) strcpy(pat_ext, ".");
    if (!*str_ext) strcpy(str_ext, ".");

    return (xstricmp(pat_file, str_file) && xstricmp(pat_ext, str_ext));
}
