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
