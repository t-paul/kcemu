#include <time.h>

#include "kc/config.h"
#include "sys/sysdep.h"

void
sys_localtime(int *year, int *month, int *day, int *hour, int *minute, int *second)
{
  time_t t;
  struct tm *result;

  t = time(NULL);
  result = localtime(&t);

  *year = result->tm_year;
  *month = result->tm_mon;
  *day = result->tm_mday;
  *hour = result->tm_hour;
  *minute = result->tm_min;
  *second = result->tm_sec;
}
