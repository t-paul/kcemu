#include <windows.h>

#include "kc/config.h"
#include "sys/sysdep.h"

void
sys_usleep(long microseconds)
{
  Sleep(microseconds / 1000);
}
