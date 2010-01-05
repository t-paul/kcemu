#include <arpa/inet.h>

#include "kc/config.h"
#include "sys/sysdep.h"

unsigned short
sys_htons(unsigned short hostshort)
{
  return htons(hostshort);
}
