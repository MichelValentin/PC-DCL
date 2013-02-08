#ifndef _WIN32

#include <unistd.h>
#include <ctype.h>
#include <string.h>

unsigned long filelength(int fh)
{
  unsigned long nLen, nCur;

  nCur = lseek(fh, 0, SEEK_CUR);
  nLen = lseek(fh, 0, SEEK_END);
  lseek(fh, nCur, SEEK_SET);

  return nLen;
}

char *strupr(char *s)
{
	int	i;
	
	for(i = 0; i < (int)strlen(s); i++) {
		s[i] = (char)toupper((int)s[i]);
	}
	return(s);
}
#endif
