/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: sysdep.h,v 1.1 2002/06/09 14:24:32 torsten_paul Exp $
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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __sys_sysdep_h
#define __sys_sysdep_h

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  sleep for the specified number of microseconds
 */
void sys_usleep(long microseconds);

/*
 *  return local time
 *
 *  works like localtime_r() as it copies the values into
 *  a buffers supplied by the caller instead of returning
 *  a pointer to some statically allocated memory
 */
void sys_localtime(int *year, int *month, int *day, int *hour, int *minute, int *second);

/*
 *  return system time
 *
 *  works like gettimeofday() but instead of returning the
 *  values in a struct timeval the values are directly stored
 *  in the specified variables.
 */
void sys_gettimeofday(long *tv_sec, long *tv_usec);

/*
 *  return system time
 *
 *  works like gettimeofday() but instead of returning the
 *  values in a struct timeval the values are directly stored
 *  in the specified variables.
 */
void sys_gettimeofday(long *tv_sec, long *tv_usec);

#ifdef __cplusplus
}
#endif

#endif /* __sys_sysdep_h */
