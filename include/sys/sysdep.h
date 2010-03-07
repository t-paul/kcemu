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

#ifndef __sys_sysdep_h
#define __sys_sysdep_h

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  Sleep for the specified number of microseconds.
 */
void sys_usleep(long microseconds);

/*
 *  Return local time.
 *
 *  This works like localtime_r() as it copies the values
 *  into a buffers supplied by the caller instead of returning
 *  a pointer to some statically allocated memory
 */
void sys_localtime(int *year, int *month, int *day, int *hour, int *minute, int *second);

/*
 *  Return local time converted from given time value.
 *
 *  This is the same as sys_localtime, but does not return
 *  the current time but the time converted from the given
 *  value.
 */
void sys_converttime(long time, int *year, int *month, int *day, int *hour, int *minute, int *second);

/*
 *  Return system time.
 *
 *  This works like gettimeofday() but instead of returning the
 *  values in a struct timeval the values are directly stored
 *  in the specified variables.
 */
void sys_gettimeofday(long *tv_sec, long *tv_usec);

/*
 *  Return system time
 *
 *  This works like gettimeofday() but instead of returning the
 *  values in a struct timeval the values are directly stored
 *  in the specified variables.
 */
void sys_gettimeofday(long *tv_sec, long *tv_usec);

/*
 *  Return basename (filename without directory) for a given
 *  full pathname.
 *
 *  This works like basename() but doesn't handle NULL arguments
 *  and special values like "/".
 *  The returned string is allocated with strdup() so it must
 *  be freed with free().
 */
char * sys_basename(const char *path);

/*
 *  Return true if the given path denotes an absolute path
 *  name for the specific system (e.g. if it starts with
 *  a '/' for linux or with a drive letter followed by a ':'
 *  for mingw).
 */
int sys_isabsolutepath(const char *path);

/*
 *  Return the directory of the currently running program.
 *
 *  Returns where the executable is located, not the current
 *  working directory! if it's not possible to retrieve this
 *  information the function returns NULL.
 *
 *  The returned string should be freed with free().
 */
char * sys_getprogrampath(void);

/*
 *  Return the user home directory.
 *
 *  The returned string should be freed with free().
 */
char * sys_gethome(void);

/*
 *  Create new directory, given permissions are ignored on
 *  some platforms (e.g. MinGW).
 */
int sys_mkdir(const char *pathname, int mode);

/*
 *  Return disk information for the given path.
 *
 *  The information is stored in the caller provided buffer. the
 *  data is only valid if the return value of the call is 0, which
 *  means there was no error while retrieving the data from the
 *  operating system.
 *
 *  As the return value is currently 32 bit, the values are clamped
 *  at 4GB.
 */
int sys_getdiskinfo(const char *path, unsigned long *total, unsigned long *available, unsigned short *block_size);

/*
 *  Convert short integer (16-bit) from host to network byte order
 */
unsigned short sys_htons(unsigned short hostshort);

/*
 *  Convert long integer (32-bit) from host to network byte order
 */
unsigned long sys_htonl(unsigned long hostlong);

#define SYS_SOCKET_ERR_INTR (-1)
#define SYS_SOCKET_ERR_INPROGRESS (-2)
#define SYS_SOCKET_ERR_ALREADY (-3)
#define SYS_SOCKET_ERR_ISCONN (-4)

/*
 *  Initialize system socket implementation (e.g. WinSocket)
 *
 *  This will be called once when starting the application, and
 *  should make the socket interface usable. For Unix systems
 *  this is normally a no-op, but it's required for the WinSocket
 *  implementation to call WSAStartup() before using any socket
 *  related functions.
 *
 *  On success sys_socket_init() returns 0.
 */
int sys_socket_init(void);

/*
 *  Create a socket.
 *
 *  If stream is set to non-zero value, the socket is opened in
 *  stream mode, otherwise in datagram mode. A non-zero value
 *  for nonblocking will set stream to nonblocking mode.
 */
int sys_socket_create(int stream, int nonblocking);

/*
 *  Close a socket.
 */
void sys_socket_close(int socket);

/*
 *  Connect socket to given ip address and port.
 */
int sys_socket_connect(int socket, unsigned char ip0, unsigned char ip1, unsigned char ip2, unsigned char ip3, unsigned short port);

/*
 *  Receive a datagram from a socket.
 *
 *  The data is placed into the given buffer and if the
 *  ipX and port parameters are not NULL the ip address
 *  and port of the source will be written into the
 *  given variables.
 */
int sys_socket_recvfrom(int socket, unsigned char *buf, int bufsize, unsigned char *ip0, unsigned char *ip1, unsigned char *ip2, unsigned char *ip3, unsigned short *port);

/*
 *  Send data to a connected socket.
 */
int sys_socket_send(int socket, unsigned char *buf, int bufsize);

/*
 *  Send data to the given ip address and port.
 */
int sys_socket_sendto(int socket, unsigned char *buf, int bufsize, unsigned char ip0, unsigned char ip1, unsigned char ip2, unsigned char ip3, unsigned short port);

#ifdef __cplusplus
}
#endif

#endif /* __sys_sysdep_h */
