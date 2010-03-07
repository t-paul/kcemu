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
#include <string.h>
#include <winsock2.h>

#include "kc/config.h"
#include "sys/sysdep.h"

int
sys_socket_sendto(int socket, unsigned char *buf, int bufsize, unsigned char ip0, unsigned char ip1, unsigned char ip2, unsigned char ip3, unsigned short port)
{
  struct sockaddr_in addr;

  memset(&addr, 0, sizeof (addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = ip0 | ip1 << 8 | ip2 << 16 | ip3 << 24;

  return sendto(socket, buf, bufsize, 0, (struct sockaddr*)&addr, sizeof(addr));
}
