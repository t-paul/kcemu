#include <time.h>
#include <stdio.h>

#include "kc/config.h"
#include "sys/sysdep.h"

#if HAVE_LOCALTIME_R

void
sys_localtime(int *year, int *month, int *day, int *hour, int *minute, int *second)
{
  time_t t;
  struct tm result;

  t = time(NULL);
  localtime_r(&t, &result);

  *year = result.tm_year;
  *month = result.tm_mon + 1;
  *day = result.tm_mday;
  *hour = result.tm_hour;
  *minute = result.tm_min;
  *second = result.tm_sec;
}

#else /* HAVE_LOCALTIME_R */
#if HAVE_LOCALTIME

void
sys_localtime(int *year, int *month, int *day, int *hour, int *minute, int *second)
{
  time_t t;
  struct tm *result;

  t = time(NULL);
  result = localtime(&t);

  *year = result->tm_year;
  *month = result->tm_mon + 1;
  *day = result->tm_mday;
  *hour = result->tm_hour;
  *minute = result->tm_min;
  *second = result->tm_sec;
}

#else /* HAVE_LOCALTIME */
#error neither HAVE_LOCALTIME_R nor HAVE_LOCALTIME defined
#endif /* HAVE_LOCALTIME */
#endif /* HAVE_LOCALTIME_R */
