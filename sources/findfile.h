#ifndef WIN32
#ifndef _FSIZE_T_DEFINED
typedef unsigned long _fsize_t;
#define _FSIZE_T_DEFINED
#endif  /* _FSIZE_T_DEFINED */

#ifndef _INTPTR_T_DEFINED
typedef int intptr_t;
#define _INTPTR_T_DEFINED
#endif

#ifndef _FINDDATA_T_DEFINED

struct _finddata_t {
        unsigned    attrib;
        time_t      time_create;    /* -1 for FAT file systems */
        time_t      time_access;    /* -1 for FAT file systems */
        time_t      time_write;
        _fsize_t    size;
        char        name[260];
		char		dirname[260];	/* unix cheating */
};

#define _FINDDATA_T_DEFINED
#endif  /* _FINDDATA_T_DEFINED */

/* File attribute constants for _findfirst() */

#define _A_NORMAL       0x00    /* Normal file - No read/write restrictions */
#define _A_RDONLY       0x01    /* Read only file */
#define _A_HIDDEN       0x02    /* Hidden file */
#define _A_SYSTEM       0x04    /* System file */
#define _A_SUBDIR       0x10    /* Subdirectory */
#define _A_ARCH         0x20    /* Archive file */

#ifdef  __cplusplus
extern "C" {
#endif
intptr_t  _findfirst(const char *, struct _finddata_t *);
int   _findnext(intptr_t, struct _finddata_t *);
int   _findclose(intptr_t);
#ifdef  __cplusplus
}
#endif
#endif
