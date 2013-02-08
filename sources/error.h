#define LOG_ERROR(a) fprintf(stderr,"%s\t%5.5d\t %s\n",__FILE__,__LINE__,ERR_TEXT[a]);

#define ERR_NOERR  0
#define ERR_MALLOC 1
#define ERR_2LONG  2

const char *ERR_TEXT[] = {
      "",
      "Memory allocation error.",
      "Token too long."
      };
