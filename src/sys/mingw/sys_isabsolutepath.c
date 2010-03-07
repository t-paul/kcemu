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
