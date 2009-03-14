#include <sys/statvfs.h>

#include "kc/config.h"
#include "sys/sysdep.h"

int
sys_getdiskinfo(const char *path, unsigned long *total, unsigned long *available, unsigned short *block_size)
{
  struct statvfs buf;

  if (statvfs(path, &buf) == 0)
    {
      unsigned long long free = (unsigned long long) buf.f_bavail * buf.f_bsize;
      unsigned long long size = (unsigned long long) buf.f_blocks * buf.f_frsize;
      *available = free > 0xffffffffUL ? 0xffffffffUL : free;
      *total = size > 0xffffffffUL ? 0xffffffffUL : total;
      *block_size = buf.f_bsize;
      return 0;
    }
  else
    {
      *total = 0;
      *available = 0;
      *block_size = 0;
      return -1;
    }
}
