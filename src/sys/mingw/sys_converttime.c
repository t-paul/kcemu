#include <time.h>
#include <stdio.h>

#include "kc/config.h"
#include "sys/sysdep.h"

void
sys_converttime(long time, int *year, int *month, int *day, int *hour, int *minute, int *second)
{
  time_t t = time;
  struct tm *result;

  result = localtime(&t);

  *year = result->tm_year;
  *month = result->tm_mon + 1;
  *day = result->tm_mday;
  *hour = result->tm_hour;
  *minute = result->tm_min;
  *second = result->tm_sec;
}
