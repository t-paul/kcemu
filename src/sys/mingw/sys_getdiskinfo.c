#include <windows.h>

#include "kc/config.h"
#include "sys/sysdep.h"

int
sys_getdiskinfo(const char *path, unsigned long *total, unsigned long *available, unsigned short *block_size)
{
  ULARGE_INTEGER free, size;
  DWORD bytes_per_sector, d1, d2, d3;

  // Fetch the block size from the old interface as this information
  // is not provided by the new one anymore.
  if (GetDiskFreeSpace(path, &d1, &bytes_per_sector, &d2, &d3))
    {
      *block_size = bytes_per_sector;
    }
  else
    {
      *block_size = 0;
    }

  // The disk size is queried via the new interface that supports
  // drives with more than 2GB (although we currently only support
  // return values of 4GB!
  if (!GetDiskFreeSpaceExA(path, &free, &size, NULL))
    {
      *total = 0;
      *available = 0;
      *block_size = 0;
      return -1;
    }
  
  *available = free.QuadPart > 0xffffffffUL ? 0xffffffffUL : free.QuadPart;
  *total = size.QuadPart > 0xffffffffUL ? 0xffffffffUL : size.QuadPart;
  return 0;
}
