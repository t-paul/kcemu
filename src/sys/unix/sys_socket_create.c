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

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "kc/config.h"
#include "sys/sysdep.h"

int
sys_socket_create(int stream, int nonblocking)
{
  int type = stream ? SOCK_STREAM : SOCK_DGRAM;

  int s = socket(AF_INET, type, 0);
  if (s < 0)
    return -1;

  if (type == SOCK_DGRAM) {
    /* Set socket to allow broadcast */
    int broadcastPermission = 1;
    if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, (void *) &broadcastPermission,
          sizeof(broadcastPermission)) < 0)
        perror("setsockopt() failed");
  }

  if (!nonblocking)
    return s;

  if (fcntl(s, F_SETFL, O_NONBLOCK) == 0)
    return s;

  close(s);
  return -1;
}
