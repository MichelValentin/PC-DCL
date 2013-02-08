#ifndef _WIN32
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fnmatch.h>
#include "dcl.h"
#include "platform.h"
#include "findfile.h"
#include "dbglog.h"

#ifndef _SEPARATOR
#ifdef _WIN32
#define _SEPARATOR	'\\'
#else
#define _SEPARATOR '/'
#endif
#endif

#ifndef MAXNAMLEN
#define MAXNAMLEN 255
#endif
#ifndef S_IFDIR
#define S_IFDIR __S_IFDIR
#endif
#ifndef S_IWRITE
#define S_IWRITE __S_IWRITE
#endif

typedef struct _ff_ {
	intptr_t	hDir;
	char		szDir[MAXNAMLEN];
	char		szPattern[MAXNAMLEN];
} T_FF, *PT_FF;

intptr_t _findfirst(const char *szFileSpec, struct _finddata_t *fileinfo)
{
//	intptr_t	hDir	= -1;
//	char		szPathName[MAXNAMLEN+1];
//	int			i		= 0;
//	int			l		= 0;
	intptr_t	hTmp;
	int 		nRetCod = 0;
	char		szDir[_MAX_DIR];
	char		szFile[_MAX_FNAME];
	char		szExt[_MAX_EXT];
	PT_FF		hFF		= (PT_FF)-1;


	if (szFileSpec != NULL && fileinfo != NULL) {
		if (strlen(szFileSpec) < MAXNAMLEN) {
			/* Get pathname only */
			_splitpath((char*)szFileSpec, NULL, szDir, szFile, szExt);
			if (*szDir == 0) {
				strcpy(szDir, ".");
			}
			hFF = malloc(sizeof(T_FF));
			if (hFF != NULL) {
				hTmp = (intptr_t)opendir(szDir);
				if (hTmp) {
					DebugLogV(DEBUGLOG_LEVEL_DEBUG, "<_findfirst> Opendir %s OK.", szDir);
					hFF->hDir = hTmp;
					strcpy(hFF->szDir, szDir);
					strcpy(hFF->szPattern, szFile);
					strcat(hFF->szPattern, szExt);
					nRetCod = _findnext((intptr_t)hFF, fileinfo);
					if (nRetCod != 0) {
						_findclose((intptr_t)hFF);
						hFF = (PT_FF)-1;
					}
				}
				else {
					_STATUS = errno;
					DebugLogV(DEBUGLOG_LEVEL_DEBUG, "<_findfirst> Opendir %s error %d.", szDir, _STATUS);
					hFF = (PT_FF)-1;
				}
			}
		}
	}
	return((intptr_t)hFF);
}

int _findnext(intptr_t handle, struct _finddata_t *fileinfo)
{
	int				nRetCod = -1;
	int				loop	= 1;
	struct dirent	*dd;
	struct stat		buf;
	PT_FF			hFF;
	char			szPath[_MAX_FNAME];


	if (handle != (intptr_t)NULL && fileinfo != NULL) {
		hFF = (PT_FF)handle;
		while (loop) {
			dd = readdir((DIR*)hFF->hDir);
			if (dd == NULL) {
				DebugLogV(DEBUGLOG_LEVEL_DEBUG, "<_findnext> readdir error %d", errno);
				if (errno == 0) { /* normal end of file */
					errno = ENOENT;
				}
				loop = 0;
			}
			else {
				DebugLogV(DEBUGLOG_LEVEL_DEBUG,
				          "<_findnext> readdir OK pattern=%s name=%s",
				          hFF->szPattern, dd->d_name);
				if (fnmatch(hFF->szPattern, dd->d_name, 0)==0){
					int	rc = 0;
					loop = 0;
					_makepath(szPath, NULL, hFF->szDir, dd->d_name, NULL);
					memset(&buf, 0, sizeof(buf));
					rc = stat(szPath, &buf);
					if (rc == 0) {
						DebugLogV(DEBUGLOG_LEVEL_DEBUG,
				          		"<_findnext> stat %s ok attr=0x%X",
				          		szPath, buf.st_mode);
						fileinfo->attrib = _A_NORMAL;
						if (buf.st_mode & S_IFDIR) {
							fileinfo->attrib |= _A_SUBDIR;
						}
						if (!(buf.st_mode & S_IWRITE)) {
							fileinfo->attrib |= _A_RDONLY;
						}
						fileinfo->size = (_fsize_t)buf.st_size;
						fileinfo->time_create = buf.st_ctime;
						fileinfo->time_access = buf.st_atime;
						fileinfo->time_write = buf.st_mtime;
					}
					else {
//						DebugLogV(DEBUGLOG_LEVEL_DEBUG, "<_findnext> stat %s error %d.", szPath, errno);
						nRetCod = errno;
					}
					(void)memset(fileinfo->name,0,sizeof(fileinfo->name));
					(void)strncpy(fileinfo->name, dd->d_name, sizeof(fileinfo->name) - 1);
					nRetCod = 0; /* OK */
				}
			}
		}
	}

	return(nRetCod);
}


int _findclose(intptr_t handle)
{
	int		nRetCod = -1;
	PT_FF	hFF		= NULL;

	if (handle != (intptr_t)NULL && handle != -1) {
		hFF = (PT_FF)handle;
		if (hFF->hDir != (intptr_t)NULL) {
			nRetCod = closedir((DIR*)hFF->hDir);
//			DebugLogV(DEBUGLOG_LEVEL_DEBUG,
//					  "<_findclose> closedir (rc = %d).", errno);
		}
		free(hFF);
	}

	return(nRetCod);
}

#endif
