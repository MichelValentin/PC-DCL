#ifndef _DCLTIME_H_
#define _DCLTIME_H_

#include <time.h>

#ifdef __cplusplus
extern "C"
{
#endif
void tm_str_to_long(char *str, time_t *tt);
void tm_tm_to_long(struct tm *tms, time_t *tt);
void tm_long_to_tm(time_t *tt, struct tm *tms);
void tm_tm_to_str(struct tm  *tms, char *str);
void tm_long_to_str(time_t *tt, char *str);
void tm_str_to_delta(char *str,time_t *delta);
int  tm_leapyear(int year);
//void tm_dir_to_tm(unsigned f_date,unsigned f_time,struct tm *tm_time);

#ifdef __cplusplus
}
#endif

#endif
