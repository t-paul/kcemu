#include <stdio.h>

#include "kc/config.h"
#include "sys/sysdep.h"

int
sys_isabsolutepath(const char *path)
{
  int drv;

  if ((path == NULL) || (path[0] == '\0') || (path[1] == '\0'))
    return 0;

  if (path[1] != ':')
    return 0;

  if (path[2] != '\\')
    return 0;

  drv = path[0];
  return (((drv >= 'a') && (drv <= 'z')) || ((drv >= 'A') && (drv <= 'Z')));
}
