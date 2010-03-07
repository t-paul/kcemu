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
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <winsock2.h>

#include "kc/config.h"
#include "sys/sysdep.h"

int
sys_socket_recvfrom(int socket, unsigned char *buf, int bufsize, unsigned char *ip0, unsigned char *ip1, unsigned char *ip2, unsigned char *ip3, unsigned short *port)
{
  struct timeval tv;
  fd_set fds;
  struct sockaddr_storage peer_addr;
  unsigned int peer_addr_len = sizeof(peer_addr);

  FD_ZERO(&fds);
  FD_SET(socket, &fds);

  tv.tv_sec = 0;
  tv.tv_usec = 0;
  int s = select(1, &fds, NULL, NULL, &tv);
  if (s < 0)
    return -1;
  if (!FD_ISSET(socket, &fds))
    return -1;

  int r = recvfrom(socket, buf, bufsize, 0, (struct sockaddr *)&peer_addr, &peer_addr_len);
  if (r < 0)
    return r;

  struct sockaddr_in *addr;
  addr = (struct sockaddr_in *)&peer_addr;
  if (ip0 != NULL)
    *ip0 = addr->sin_addr.s_addr;
  if (ip1 != NULL)
    *ip1 = addr->sin_addr.s_addr >> 8;
  if (ip2 != NULL)
    *ip2 = addr->sin_addr.s_addr >> 16;
  if (ip3 != NULL)
    *ip3 = addr->sin_addr.s_addr >> 24;
  if (port != NULL)
    *port = ntohs(addr->sin_port);

  return r;
}
