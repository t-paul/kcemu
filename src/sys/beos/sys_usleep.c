#include "kc/config.h"
#include "sys/sysdep.h"

void
sys_usleep(long microseconds)
{
    snooze(microseconds);
}
