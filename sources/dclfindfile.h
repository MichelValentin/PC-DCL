#ifndef _DCL_FIND_DATA_H_
#define _DCL_FIND_DATA_H_



/* File attribute constants for _findfirst() */

//#define _A_NORMAL       0x00    /* Normal file - No read/write restrictions */
//#define _A_RDONLY       0x01    /* Read only file */
//#define _A_HIDDEN       0x02    /* Hidden file */
//#define _A_SYSTEM       0x04    /* System file */
//#define _A_SUBDIR       0x10    /* Subdirectory */
//#define _A_ARCH         0x20    /* Archive file */

#ifdef __cplusplus
extern "C"
{
#endif
int     Dcl_FindFirstFile(char *lpFileName, LPDCL_FIND_DATA lpFindFileData);
BOOL    Dcl_FindNextFile(int hFindFile, LPDCL_FIND_DATA lpFindFileData);
BOOL    Dcl_FindClose(int hFindFile);
#ifdef __cplusplus
}
#endif

#endif
