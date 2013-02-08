

/*
Copyright (C) 2003 Matthias Braun <matze@braunis.de>
                                                                                
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
                                                                                
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
                                                                                
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef _WIN32

#include <stdio.h>
#include <string.h>
#include "splitpath.h"

/** small helper function, copies only a part of a string into another
 * (we append a 0 in contrary to strncpy)
 */
static inline void strcpypart(char *dest, char *src, size_t size)
{
    memcpy(dest, src, size);
    dest[size] = 0;
}

/** Unix version of the _splitpath functions which breaks a path into it's
 * components. (Warning: Don't use in your own code, you're asking for security
 * problems (buffer overflow)
 */
void _splitpath(char *path, char *drive, char *dir,
                char *fname, char *ext)
{
	char *p 		= 0;
    char *lastslash = 0;
    char *lastdot 	= 0;
    char *begin 	= 0;
	
    // Step1: Fill in drive ("" on unix systems since we don't have drives)
    if(drive)
        strcpy(drive, "");

    // Step1: find the last slash in the path
    for(p = path; *p != '\0'; p++) {
        if(*p == '/')
            lastslash = p;
    }
    // now fill in directory
    if(dir) {
        if(lastslash == 0)
            strcpy(dir, "");
        else
            strcpypart(dir, path, lastslash - path + 1);
    }

    // Step2: Get the (last) dot in the filename
    begin = (lastslash != 0) ? lastslash+1 : path;
    for(p = begin; *p != '\0'; p++) {
        if(*p == '.')
            lastdot = p;
    }
    // now we can fill in filename and extension
    if(lastdot==0) {
        if(fname)
            strcpy(fname, begin);
        if(ext)
            strcpy(ext, "");
    } else {
        if(fname)
            strcpypart(fname, begin, lastdot - begin);
        if(ext)
            strcpy(ext, lastdot);
    }

    //printf ("Splitted Path : %s => %s - %s - %s - %s.\n", path,
    //		drive, dir, fname, ext);
}

void _makepath(char *path, char *drive, char *dir, char *fname, char *ext)
{
	size_t	len = 0;
	
	*path = 0;
	
	if (drive && *drive) {
		strcat(path, drive);
		len = strlen(path);
		if (len > 0 && path[strlen(path)-1] != ':') {
			strcat(path, ":");
		}
	}
	
	if (dir) {
//		if (*dir != '/') {
//			strcat(path, "/");
//		}
		strcat(path, dir);
		len = strlen(path);
		if (len > 0 && path[strlen(path)-1] != '/') {
			strcat(path, "/");
		}
	}

	if (fname) {
		strcat(path, fname);
	}

	if (ext) {
		if (*ext && *ext != '.') {
			strcat(path, ".");
		}
		strcat(path, ext);
	}
	
}


#endif

