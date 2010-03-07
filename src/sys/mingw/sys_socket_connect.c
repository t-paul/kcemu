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
sys_socket_connect(int socket, unsigned char ip0, unsigned char ip1, unsigned char ip2, unsigned char ip3, unsigned short port)
{
  int err;
  struct sockaddr_in addr;

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = ip0 | ip1 << 8 | ip2 << 16 | ip3 << 24;

  err = connect(socket, (struct sockaddr *)&addr, sizeof(addr));
  if (err >= 0)
    return 0;

  err = WSAGetLastError();

  switch (err)
    {
      case WSAEINTR:
        return SYS_SOCKET_ERR_INTR;
      case WSAEINPROGRESS:
      case WSAEWOULDBLOCK:
      case WSAEINVAL:
	return SYS_SOCKET_ERR_INPROGRESS;
      case WSAEALREADY:
	return SYS_SOCKET_ERR_ALREADY;
      case WSAEISCONN:
	return SYS_SOCKET_ERR_ISCONN;
      default:
        return err;
    }
}
