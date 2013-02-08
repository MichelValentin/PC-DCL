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

//struct _finddata_t
//{
//    unsigned    attrib;     /* Attributes, see constants above. */
//    time_t      time_create;
//    time_t      time_access;    /* always midnight local time */
//    time_t      time_write;
//    _fsize_t    size;
//    char        name[FILENAME_MAX]; /* may include spaces. */
//};

//#include <sys/io.h>
#include <string.h>
#include "platform.h"
#include "dcl.h"
#include "dbglog.h"

int Dcl_FindFirstFile(char *lpFileName, LPDCL_FIND_DATA lpFindFileData)
{
    int    hFindFile;
    struct _finddata_t ff;

    hFindFile = _findfirst(lpFileName, &ff);
    if (hFindFile != (int)INVALID_HANDLE_VALUE) {
        memset(lpFindFileData, 0, sizeof(DCL_FIND_DATA));
        lpFindFileData->dwFileAttributes = ff.attrib;
        lpFindFileData->nFileSize = ff.size;
        lpFindFileData->CreationTime = ff.time_create;
        lpFindFileData->AccessTime = ff.time_access;
        lpFindFileData->WriteTime = ff.time_write;
        memcpy(lpFindFileData->cFileName, ff.name, sizeof(lpFindFileData->cFileName));
    }
    return(hFindFile);
}

BOOL Dcl_FindNextFile(int hFindFile, LPDCL_FIND_DATA lpFindFileData)
{
    int    rc;
    struct _finddata_t ff;

    rc = _findnext(hFindFile, &ff);
    if (rc == 0) {
        memset(lpFindFileData, 0, sizeof(DCL_FIND_DATA));
        lpFindFileData->dwFileAttributes = ff.attrib;
        lpFindFileData->nFileSize = ff.size;
        lpFindFileData->CreationTime = ff.time_create;
        lpFindFileData->AccessTime = ff.time_access;
        lpFindFileData->WriteTime = ff.time_write;
        memcpy(lpFindFileData->cFileName, ff.name, sizeof(lpFindFileData->cFileName));
    }
    return(rc == 0 ? 1 : 0);
}

BOOL Dcl_FindClose(int hFindFile) 
{
    return _findclose(hFindFile);
}

 
