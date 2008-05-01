#include <stdio.h>

#include "kc/config.h"
#include "sys/sysdep.h"

int
sys_isabsolutepath(const char *path)
{
  if (path == NULL)
    return 0;

  return path[0] == '/';
}
