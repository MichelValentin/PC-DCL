#ifndef SPLITPATH_H_
#define SPLITPATH_H_


// unix emulation code for the win32 _splitpath function
// Warning: don't use this function in your own code, you're asking for secutiry
// problems (buffer overflow)

#ifndef WIN32
#define _mkdir(a)      	mkdir(a, 0777)
#define _rmdir(a)       rmdir(a)
#define _getcwd(a,b)    getcwd(a,b)
#define _sleep(a)		sleep(a)
#define MAX_PATH		PATH_MAX
void _splitpath(char *path, char *drive, char *dir,
                char *fname, char *ext);

void _makepath(char *path, char *drive, char *dir, char *fname, char *ext);

#endif

#endif /*SPLITPATH_H_*/
