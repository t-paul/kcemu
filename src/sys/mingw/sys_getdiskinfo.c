/*
 *  KCemu -- The emulator for the KC85 homecomputer series and much more.
 *  Copyright (C) 1997-2010 Torsten Paul
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

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
