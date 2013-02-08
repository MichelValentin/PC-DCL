#ifndef __DEBUGLOG_H__
#define __DEBUGLOG_H__

#define DEBUGLOG_LEVEL_NONE       0
#define DEBUGLOG_LEVEL_ERROR      1
#define DEBUGLOG_LEVEL_WARNING    2
#define DEBUGLOG_LEVEL_INFO       3
#define DEBUGLOG_LEVEL_TRACE      4
#define DEBUGLOG_LEVEL_DEBUG      5

#ifdef __cplusplus
extern "C"
{
#endif

void DebugLogSetLevel(int nLevel);
int  DebugLogGetLevel(void);
void DebugLogSetFilename(char const *szFilename);
void DebugLogSetDayFile(int fClear);
void DebugLogClearFile(void);
void DebugLogV(int nSeverity, char const *szFormat, ...);
void DebugLog(int nSeverity, char const *szText);
void DebugLogDump(int nSeverity, void const *lpData, size_t size, char const *sztag);

#ifdef __cplusplus
}
#endif

#define DEBUGLOG_TIMER_START(str) /*lint -save -e578*/{\
    clock_t     __tt_clock__ = clock(); \
    char const *__tt_msg__   = (char const *)(str);\
    {/*lint -restore */

#define DEBUGLOG_TIMER_STOP() }\
    DebugLog(DEBUGLOG_LEVEL_TRACE, "%s - elapsed time : %.2f sec(s).\n", __tt_msg__,\
             (double)(clock() - __tt_clock__) / CLOCKS_PER_SEC);\
    }

#define DEBUGLOG(s,t) DebugLogV(s,"%s %s %d %s\n",__FILE__,__func__,__LINE__,t)
#define DEBUGLOGV(s,t,...) DebugLogV(s,"%s %s %d %s\n",__FILE__,__func__,__LINE__,t,...)
#endif
