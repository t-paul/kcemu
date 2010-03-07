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

#include <time.h>
#include <stdio.h>

#include "kc/config.h"
#include "sys/sysdep.h"

#if HAVE_LOCALTIME_R

void
sys_localtime(int *year, int *month, int *day, int *hour, int *minute, int *second)
{
  time_t t;
  struct tm result;

  t = time(NULL);
  localtime_r(&t, &result);

  *year = result.tm_year;
  *month = result.tm_mon + 1;
  *day = result.tm_mday;
  *hour = result.tm_hour;
  *minute = result.tm_min;
  *second = result.tm_sec;
}

#else /* HAVE_LOCALTIME_R */
#if HAVE_LOCALTIME

void
sys_localtime(int *year, int *month, int *day, int *hour, int *minute, int *second)
{
  time_t t;
  struct tm *result;

  t = time(NULL);
  result = localtime(&t);

  *year = result->tm_year;
  *month = result->tm_mon + 1;
  *day = result->tm_mday;
  *hour = result->tm_hour;
  *minute = result->tm_min;
  *second = result->tm_sec;
}

#else /* HAVE_LOCALTIME */
#error neither HAVE_LOCALTIME_R nor HAVE_LOCALTIME defined
#endif /* HAVE_LOCALTIME */
#endif /* HAVE_LOCALTIME_R */
