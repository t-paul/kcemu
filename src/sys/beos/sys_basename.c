#include <string.h>

#include "kc/config.h"
#include "sys/sysdep.h"

char *
sys_basename(const char *path)
{
  const char *basename = strrchr(path, '/');
              
  if (basename)
    basename++;
  else
    basename = path;
  
  return strdup(basename);
}
