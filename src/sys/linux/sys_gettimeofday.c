#include <sys/time.h>

#include "kc/config.h"
#include "sys/sysdep.h"

void
sys_gettimeofday(long *tv_sec, long *tv_usec)
{
  struct timeval tv;

  gettimeofday(&tv, 0);
  *tv_sec = tv.tv_sec;
  *tv_usec = tv.tv_usec;
}
