#include <windows.h>
#include <mmsystem.h>

#include "kc/config.h"
#include "sys/sysdep.h"

void
sys_gettimeofday(long *tv_sec, long *tv_usec)
{
  DWORD t = timeGetTime();
  *tv_sec = t / 1000;
  *tv_usec = 1000 * (t % 1000); // microseconds!
}
