#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <unistd.h>

static const int COPYBUFFSZ = 4096;

int filecopy(const char *fn_src, const char *fn_dst, int bFailIfExists)
{
	int   bytesr;
	int   retval  = 0;
	FILE  *fh_src = NULL;
	FILE  *fh_dst = NULL;
	char  *buff   = NULL;
	struct stat stat_s;
	struct stat stat_d;

	buff = (char *) malloc(COPYBUFFSZ);
	if(buff == NULL)
		return errno;

	if(stat(fn_src, &stat_s) != 0)
		retval = errno;

	/* if we can stat the dest file then make sure it is not the same as src */
	if(stat(fn_dst, &stat_d) == 0)	{
		if (bFailIfExists || (strcmp(fn_src, fn_dst) == 0)) {
			retval = EEXIST;
		}
	}

	if(!retval) {
		fh_src = fopen(fn_src, "rb");
		if(fh_src == NULL)
			retval = errno;
	}

	if(!retval)	{
	 	fh_dst = fopen(fn_dst, "wb+");

	 	if(fh_dst == NULL || stat(fn_dst, &stat_d) != 0)
			retval = errno;
		else {
			while(!retval) {
				bytesr = fread(buff, 1, COPYBUFFSZ, fh_src);
				if(bytesr < 1) {
					if(feof(fh_src))
						break;
					else
						retval = ferror(fh_src);
				}
				else {
					if(fwrite(buff, 1, bytesr, fh_dst) != bytesr) {
						retval = ferror(fh_dst);
						break;
					}
				}
			}
		} /* if fh_dst */

		if(fh_dst) {
			fclose(fh_dst);
		}

	} /* if !retval */

	if(fh_src) {
		fclose(fh_src);
	}
	if(buff) {
		free(buff);
	}

	return retval;
}
