#include <sys/stat.h>
#include <sys/types.h>

#include "kc/config.h"
#include "sys/sysdep.h"

int
sys_mkdir(const char *pathname, int mode)
{
  return mkdir(pathname);
}
