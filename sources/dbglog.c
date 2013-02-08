#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "platform.h"
#include "dbglog.h"

#define DEBUGLOG_LEVEL_PARAM    "DEBUGLEVEL"
#define DEBUGLOG_FILE_PARAM     "DEBUGFILE"
#define DEBUGLOG_FILENAME       "DEBUG.LOG"
#define DEBUGLOG_FNAME_LENGTH   255

static void DebugLogInit(void);

static struct __DebugLog__ {
    int     fInitDone;
    int     nDebugLevel;
    char    szFileName[DEBUGLOG_FNAME_LENGTH];
    char    szFileNameRef[DEBUGLOG_FNAME_LENGTH];
} G_DebugLog =
#ifdef NDEBUG
{0, DEBUGLOG_LEVEL_NONE, "", ""};
#else
{0, DEBUGLOG_LEVEL_DEBUG, "", ""};
#endif
static char *achDebugLogLevelChar = "-EWITD*";
#define DEBUGLOG_LEVEL_LAST 6

/**
 * DebugLogInit
 *
 * \li Initializes DEBUGLOG module.
 * \li Reads debug level and debug file name from environment params
 * \li or uses default values.
 *
 * @param   NONE
 * @return  NONE
 */
static void DebugLogInit(void)
{
    char    *szDebugLevel = NULL;
    char    *szFileName   = NULL;

    szDebugLevel = getenv(DEBUGLOG_LEVEL_PARAM);
    if (szDebugLevel != NULL) {
        G_DebugLog.nDebugLevel = atoi(szDebugLevel);
    }
    szFileName = getenv(DEBUGLOG_FILE_PARAM);
    if (szFileName == NULL) {
        if (_getcwd(G_DebugLog.szFileName,DEBUGLOG_FNAME_LENGTH) != NULL) {
            if (G_DebugLog.szFileName[strlen(G_DebugLog.szFileName)-1] != SLASH_CHR) strcat(G_DebugLog.szFileName,SLASH_STR);
        }
        (void)strcat(G_DebugLog.szFileName, DEBUGLOG_FILENAME);
    }
    else {
        (void)memset(G_DebugLog.szFileName, 0, sizeof(G_DebugLog.szFileName));
        (void)strncpy(G_DebugLog.szFileName, szFileName, sizeof(G_DebugLog.szFileName)-1);
    }
    (void)strcpy(G_DebugLog.szFileNameRef, G_DebugLog.szFileName);
    G_DebugLog.fInitDone = 1;
}

/**
 * DebugLog
 *
 * \li Formats and writes text to logging file.
 *
 * @remark  CRLF automagically appended to text.
 *
 * @param   short nSeverity (IN)  - Severity level
 * @param   char  *szText   (IN)  - Output text
 * @return  NONE
 */
void DebugLog(int nSeverity, char const *szText)
{
    FILE        *fp     = NULL;
    time_t      tt      = time(NULL);
    int         i       = 0;
    char        szFileName[DEBUGLOG_FNAME_LENGTH + 1];
    struct  tm  *ttm    = localtime(&tt);

    if (!G_DebugLog.fInitDone) {
        DebugLogInit();
    }

    if (nSeverity <= G_DebugLog.nDebugLevel) {
        char    chLevel = (char)((nSeverity < DEBUGLOG_LEVEL_LAST) ? 
                          achDebugLogLevelChar[nSeverity] : '-');
        strftime(szFileName,DEBUGLOG_FNAME_LENGTH,G_DebugLog.szFileName,ttm);
        fp = fopen(szFileName, "a");
        if (fp != NULL) {
            (void)fprintf(fp,
                          "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d %c ",
                          ttm->tm_year + 1900,
                          ttm->tm_mon + 1,
                          ttm->tm_mday,
                          ttm->tm_hour,
                          ttm->tm_min,
                          ttm->tm_sec,
                          chLevel);
            for(i = 0; i < (int)strlen(szText); putc(szText[i++], fp)) ;
            if (szText[strlen(szText)-1] != '\n')
                (void)fprintf(fp, "\n");
            (void)fclose(fp);
        }
#ifndef NDEBUG
//        (void)printf("%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d %c ",
//                      ttm->tm_year + 1900,
//                      ttm->tm_mon + 1,
//                      ttm->tm_mday,
//                      ttm->tm_hour,
//                      ttm->tm_min,
//                      ttm->tm_sec,
//                      chLevel);
//        for(i = 0; i < (int)strlen(szText); putchar(szText[i++])) ;
//        if (szText[strlen(szText)-1] != '\n')
//            (void)printf("\n");
#endif
    }
}

/**
 * DebugLogV
 *
 * \li Formats and writes text to logging file.
 * \li Takes a variable arguments list
 *
 * @remark  CRLF automagically appended to text.
 *
 * @param   short nSeverity (IN)  - Severity level
 * @param   char  *szFormat (IN)  - Output text in 'printf' format
 * @return  NONE
 */
void DebugLogV(int nSeverity, char const *szFormat, ...)
{
    FILE        *fp     = NULL;
    va_list     argptr;
    time_t      tt      = time(NULL);
    char        szFileName[DEBUGLOG_FNAME_LENGTH + 1];
    struct  tm  *ttm    = localtime(&tt);

    if (!G_DebugLog.fInitDone) {
        DebugLogInit();
    }

    if (nSeverity <= G_DebugLog.nDebugLevel) {
        char    chLevel = (char)((nSeverity < DEBUGLOG_LEVEL_LAST) ? 
                          achDebugLogLevelChar[nSeverity] : '-');
        strftime(szFileName,DEBUGLOG_FNAME_LENGTH,G_DebugLog.szFileName,ttm);
        fp = fopen(szFileName, "a");
        if (fp != NULL) {
            (void)fprintf(fp,
                          "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d %c ",
                          ttm->tm_year + 1900,
                          ttm->tm_mon + 1,
                          ttm->tm_mday,
                          ttm->tm_hour,
                          ttm->tm_min,
                          ttm->tm_sec,
                          chLevel);
            va_start(argptr, szFormat);     /*lint !e826 */
            (void) vfprintf(fp, szFormat, argptr );
            va_end(argptr);
            if (szFormat[strlen(szFormat)-1] != '\n')
                (void)fprintf(fp, "\n");
            (void)fclose(fp);
        }
#ifndef NDEBUG
//        (void)printf("%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d %c ",
//                      ttm->tm_year + 1900,
//                      ttm->tm_mon + 1,
//                      ttm->tm_mday,
//                      ttm->tm_hour,
//                      ttm->tm_min,
//                      ttm->tm_sec,
//                      chLevel);
//        va_start(argptr, szFormat);     /*lint !e826 */
//        (void) vprintf(szFormat, argptr );
//        va_end(argptr);
//        if (szFormat[strlen(szFormat)-1] != '\n')
//            (void)printf("\n");
#endif
    }
}

/**
 * DebugLogDump
 *
 * \li Formats and writes data dump to logging file.
 *
 *
 * @param   short  nSeverity (IN)  - Severity level
 * @param   void   *lpData   (IN)  - Data to be dumped
 * @param   size_t size      (IN)  - Data length
 * @param   char   *szTag    (In)  - Additional text to be printed
 * @return  NONE
 */
void DebugLogDump(int nSeverity, void const *lpData, size_t size, char const *szTag)
{
    FILE            *fp     = NULL;
    time_t          tt      = time(NULL);
    struct  tm  *ttm    = localtime(&tt);
    char            achBuffer[128];
    unsigned long   i, j, k = 0;
    char    ch;
    char    chWork[5];
    char    *pchData;
    char    szFileName[DEBUGLOG_FNAME_LENGTH + 1];

    if (!G_DebugLog.fInitDone) {
        DebugLogInit();
    }

    if (nSeverity <= G_DebugLog.nDebugLevel) {
        char    chLevel = (char)((nSeverity < DEBUGLOG_LEVEL_LAST) ? 
                           achDebugLogLevelChar[nSeverity] : '-');
        strftime(szFileName,DEBUGLOG_FNAME_LENGTH,G_DebugLog.szFileName,ttm);
        fp = fopen(szFileName, "a");
        if (fp != NULL) {
            (void)fprintf(fp,
                          "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d %c ",
                          ttm->tm_year + 1900,
                          ttm->tm_mon + 1,
                          ttm->tm_mday,
                          ttm->tm_hour,
                          ttm->tm_min,
                          ttm->tm_sec,
                          chLevel);
#ifndef NDEBUG
//            (void)printf("%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d %c ",
//                         ttm->tm_year + 1900,
//                         ttm->tm_mon + 1,
//                         ttm->tm_mday,
//                         ttm->tm_hour,
//                         ttm->tm_min,
//                         ttm->tm_sec,
//                         chLevel);
#endif
            if (szTag != NULL && *szTag) {
                (void)fprintf(fp, "%s Length: %d\n",szTag, (int)size);
#ifndef NDEBUG
 //               (void)printf("%s Length: %d\n",szTag, (int)size);
#endif
            }
            pchData = (char *) lpData;
            (void)memset(achBuffer, ' ', 80);
            j = 0;
            k = 50;
            for (i = 0; i < size; i++) {
                if (j == 0) {
                    (void)sprintf(chWork,"%4.4X", (int)i);
                    (void)memcpy(achBuffer, chWork, 4);
                    j += 6;
                    k += 6;
                }
                ch = pchData[i];
                (void)sprintf(chWork, "%2.2X ", ch & 0x00FF);
                (void)memcpy(&achBuffer[j], chWork, 3);
                if (isprint(ch & 0x00FF))
                    achBuffer[k] = ch;
                else
                    achBuffer[k] = '.';
                j += 3;
                k += 1;
                if (j == 30) {
                    j += 1;
                }
                if (j >= 54) {
                    achBuffer[k] = '\n';
                    achBuffer[k+1] = 0;
                    (void)fprintf(fp, "%s",achBuffer);
#ifndef NDEBUG
//                    (void)printf("%s",achBuffer);
#endif
                    (void)memset(achBuffer, ' ', 80);
                    j = 0;
                    k = 50;
                }
            }
            /* print end of line */
            if (j) {
                achBuffer[k] = '\n';
                achBuffer[k+1] = 0;
                (void)fprintf(fp, "%s",achBuffer);
#ifndef NDEBUG
//                (void)printf("%s",achBuffer);
#endif
            }
            (void)fclose(fp);
        }
    }
}

/**
 * DebugLogSetLevel
 *
 * \li Overrides debug log level.
 *
 *
 * @param   short nLevel (IN)  - Severity level
 * @return  NONE
 */
void DebugLogSetLevel(int nLevel)
{
    G_DebugLog.nDebugLevel = nLevel;
}

/**
 * DebugLogGetLevel
 *
 * \li Returns current debug log level.
 *
 *
 * @param   NONE
 * @return  int - Severity level
 */
int DebugLogGetLevel(void)
{
    return(G_DebugLog.nDebugLevel);
}

/**
 * DebugLogSetFilename
 *
 * \li Change Debuglog file name
 *
 * @param   char*    Filename (IN)  - File name
 *
 * @return  NONE
 */
void DebugLogSetFilename(char const *szFilename)
{
    if (!G_DebugLog.fInitDone) {
        DebugLogInit();
    }

    if (szFilename != NULL && *szFilename != 0) {
        (void)memset(G_DebugLog.szFileName, 0, sizeof(G_DebugLog.szFileName));
        (void)strncpy(G_DebugLog.szFileName, szFilename, sizeof(G_DebugLog.szFileName)-1);
        (void)strcpy(G_DebugLog.szFileNameRef, G_DebugLog.szFileName);
    }
    else {
        DebugLog(DEBUGLOG_LEVEL_ERROR, "DebugLogSetFilename - Invalid file name.");
    }
}

/**
 * DebugLogClearFile
 *
 * \li Clears DebugLog file
 *
 * @param   NONE
 *
 * @return  NONE
 */
void DebugLogClearFile(void)
{
    char        szFileName[DEBUGLOG_FNAME_LENGTH + 1];
    FILE        *fp     = NULL;
    time_t      tt      = time(NULL);
    struct  tm  *ttm    = localtime(&tt);

    if (!G_DebugLog.fInitDone) {
        DebugLogInit();
    }

    strftime(szFileName,DEBUGLOG_FNAME_LENGTH,G_DebugLog.szFileName,ttm);
    fp = fopen(szFileName, "w");
    if (fp != NULL) {
        (void)fclose(fp);
    }

    DebugLogV(DEBUGLOG_LEVEL_INFO, "DEBUGLOG file set to %s.", szFileName);
}

/**
 * DebugLogSetDayFile
 *
 * \li Change Debuglog file name to end with the number of the day
 *     of the week
 *
 * @param   int   fClear (IN)  - Flag - Clear file in TRUE
 *
 * @return  NONE
 */
void DebugLogSetDayFile(int fClear)
{
    time_t      tt      = time(NULL);
    struct  tm  *ttm    = localtime(&tt);
    char        szDay[4]= {0};
    char        szFileName[DEBUGLOG_FNAME_LENGTH + 1];

    if (!G_DebugLog.fInitDone) {
        DebugLogInit();
    }

    (void)sprintf(szDay, "%d", ttm->tm_wday);
    szDay[1] = 0;

    {
        char    *ptr = NULL;
        (void)strcpy(G_DebugLog.szFileName, G_DebugLog.szFileNameRef);
        ptr = strrchr(G_DebugLog.szFileName, '.');
        if (ptr == NULL) {
            (void)strcat(G_DebugLog.szFileName, szDay);
        }
        else {
            char szSave[DEBUGLOG_FNAME_LENGTH];
            (void)strcpy(szSave, ptr);
            *ptr = szDay[0];
            (void)strcpy(&ptr[1], szSave);
        }
    }
    /* Clear file */
    strftime(szFileName,DEBUGLOG_FNAME_LENGTH,G_DebugLog.szFileName,ttm);
    if (fClear) {
        FILE *fp = fopen(szFileName, "w");
        if (fp != NULL)
            (void)fclose(fp);
    }

    DebugLogV(DEBUGLOG_LEVEL_INFO, "DEBUGLOG file set to %s.", G_DebugLog.szFileName);
}
