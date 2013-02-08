#include <time.h>
#include <string.h>

#include "dcl.h"
#include "platform.h"
#include "termio.h"

int  default_insert_mode;
extern int get_process_info(DWORD dwPid, PROCESSINFO *pi);

void tio_print_status(void)
{
    struct  tm  *time_s;
    time_t  time_l;
    char    value[MAX_TOKEN];
#ifdef _WIN32
    {
    DWORD       dwPid = 0;
    PROCESSINFO pi;
    DWORD       dwIO = 0;
    DWORD       dwMem = 0;
    SYSTEMTIME  SystemTime;
    FILETIME    TotalTime;
    LARGE_INTEGER   Total64;
    LARGE_INTEGER   Work64;
    
    (void)time(&time_l);
    time_s = localtime(&time_l);

    symbol_get("DCL$CTRLT_PID",value);
    if (*value) {
        dwPid = atol(value);
    }
    if (dwPid == 0) {
        dwPid = getpid();
    }
    
    (void) get_process_info(dwPid, &pi);

    Total64.QuadPart = 0;
    memcpy(&Work64, &pi.KernelTime, sizeof(FILETIME));
    Total64.QuadPart = Total64.QuadPart + Work64.QuadPart;
    memcpy(&Work64, &pi.UserTime, sizeof(FILETIME));
    Total64.QuadPart = Total64.QuadPart + Work64.QuadPart;
    memcpy(&TotalTime, &Total64, sizeof(FILETIME));
    (void)FileTimeToSystemTime(&TotalTime, &SystemTime);
    
    dwIO = pi.ioc.OtherOperationCount + pi.ioc.ReadOperationCount + pi.ioc.WriteOperationCount;
    dwMem =  pi.pmc.PagefileUsage + pi.pmc.WorkingSetSize;
    dwMem = dwMem / 1024;
    
    (void)tio_printf("%s::%s  %2.2d:%2.2d:%2.2d", 
                     strupr(pi.szComputerName), strupr(pi.szUserName), 
                     time_s->tm_hour, time_s->tm_min, time_s->tm_sec);
    (void)tio_printf("  %s", strupr(pi.szProcessName));
    (void)tio_printf("  CPU=%2.2d:%2.2d:%2.2d.%3.3d PF=%lu IO=%lu MEM=%lu", 
                     SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond, SystemTime.wMilliseconds,
                     pi.pmc.PageFaultCount, dwIO, dwMem);
    }
#else
    {
    unsigned long elapsed,hour,min,sec;
    (void)tio_printf("PC-DCL V%s  -  ",VERSION);
    (void)time(&time_l);
    time_s = localtime(&time_l);
    (void)tio_printf("%2.2d-%s-%4.4d %2.2d:%2.2d:%2.2d.00  -  (",
            time_s->tm_mday,
            MONTH[time_s->tm_mon+1],
            time_s->tm_year+1900,
            time_s->tm_hour,
            time_s->tm_min,
            time_s->tm_sec);
    elapsed = (unsigned long) difftime(time_l,start_time);
    hour = elapsed / 3600L; elapsed = elapsed % 3600L;
    min  = elapsed / 60L;
    sec  = elapsed % 60L;
    if (hour == 1)
        (void)tio_printf("%lu hour, ",hour);
    if (hour > 1)
        (void)tio_printf("%lu hours, ",hour);
    if (min > 0)
        (void)tio_printf("%lu min, ",min);
    (void)tio_printf("%lu sec.)",sec);
    }
#endif    
    (void)symbol_get("DCL$CTRLT",value);
    if (*value) {
        (void) dcl_printf(dcl[D].SYS_OUTPUT,"\n");
        (void) dcl_printf(dcl[D].SYS_OUTPUT,value);
    }
}

