#include "sys/sysdep.h"

void
sys_usleep(long microseconds)
{
    usleep(microseconds);
}
