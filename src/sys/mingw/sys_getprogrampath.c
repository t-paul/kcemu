#include <windows.h>

#include "kc/config.h"
#include "sys/sysdep.h"

char *
sys_getprogrampath(void)
{
  int len;
  TCHAR cmd[MAX_PATH];

  int ret = GetModuleFileName(NULL, cmd, MAX_PATH);
  if ((ret == 0) || (ret == MAX_PATH))
    return NULL;

  // safeguard against path without drive letter
  if ((cmd[1] != ':') || (cmd[2] != '\\'))
    return NULL;

  // remove filename, loop end checked above
  len = strlen(cmd);
  while (cmd[len - 1] != '\\') len--;
  
  cmd[len] = '\0';

  return strdup(cmd);
}
