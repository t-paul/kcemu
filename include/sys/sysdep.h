/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2005 Torsten Paul
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

/*
 *  return basename (filename without directory) for a given
 *  full pathname
 *
 *  works like basename() but doesn't handle NULL arguments
 *  and special values like "/".
 *  the returned string is allocated with strdup() so it must
 *  be freed with free().
 */
char * sys_basename(const char *path);

/*
 *  return true if the given path denotes an absolute path
 *  name for the specific system (e.g. if it starts with
 *  a '/' for linux or with a drive letter followed by a ':'
 *  for mingw).
 */
int sys_isabsolutepath(const char *path);

/*
 *  return the directory of the currently running program
 *
 *  returns where the executable is located, not the current
 *  working directory! if it's not possible to retrieve this
 *  information the function returns NULL.
 *
 *  the returned string should be freed with free().
 */
char * sys_getprogrampath(void);

/*
 *  return the user home directory
 *
 *  the returned string should be freed with free().
 */
char * sys_gethome(void);

/*
 *  create new directory, given permissions are ignored on
 *  some platforms (e.g. MinGW)
 */
int sys_mkdir(const char *pathname, int mode);

#ifdef __cplusplus
}
#endif

#endif /* __sys_sysdep_h */
