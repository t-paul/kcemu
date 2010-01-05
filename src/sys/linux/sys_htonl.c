#include <arpa/inet.h>

#include "kc/config.h"
#include "sys/sysdep.h"

unsigned long
sys_htonl(unsigned long hostlong)
{
  return htonl(hostlong);
}
