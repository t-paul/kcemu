/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2009 Torsten Paul
 *
 *  $Id$
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

#include <string>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "kc/system.h"

#include "kc/kc.h"

#include "kc/prefs/prefs.h"

#include "kc/kcnet/tcp.h"

#include "libdbg/dbg.h"

TCP::TCP(void)
{
  _socket = 0;
  _is_open = false;
  _send_data = NULL;
}

TCP::~TCP(void)
{
}

void
TCP::set_ip_address(byte_t a0, byte_t a1, byte_t a2, byte_t a3)
{
  printf("TCP::set_ip_address(): %d.%d.%d.%d\n", a0, a1, a2, a3);
  _ip0 = a0;
  _ip1 = a1;
  _ip2 = a2;
  _ip3 = a3;
}

void
TCP::set_port(word_t port)
{
  printf("TCP::set_port(): %d\n", port);
  _port = port;
}

bool
TCP::open(void)
{
  printf("TCP::open()\n");
  _is_open = false;
  _socket = socket(AF_INET, SOCK_STREAM, 0);
  if (_socket < 0)
    {
      close();
      return false;
    }

  if (fcntl(_socket, F_SETFL, O_NONBLOCK) < 0)
    {
      close();
      return false;
    }

  return true;
}

bool
TCP::is_open(void)
{
  if (_socket == 0)
    return false;

  if (_is_open)
    return true;

  char buf[4096];
  snprintf(buf, sizeof(buf), "%d.%d.%d.%d", _ip0, _ip1, _ip2, _ip3);

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof (addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(_port);
  inet_pton(AF_INET, buf, &addr.sin_addr);

  int ret = connect(_socket, (struct sockaddr *)&addr, sizeof(struct sockaddr));
  if (ret < 0)
    {
      switch (errno)
        {
        case EINTR:
          printf("TCP::is_open(): connect() failed with EINTR (%s:%d)\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
          return false;
        case EINPROGRESS:
          printf("TCP::is_open(): connect() failed with EINPROGRESS (%s:%d)\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
          return false;
        case EALREADY:
          printf("TCP::is_open(): connect() failed with EALREADY (%s:%d)\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
          return false;
        case EISCONN:
          return true;
        default:
          printf("TCP::is_open(): connect() failed with errno %d (%s:%d)\n", errno, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
          close();
          return false;
        }
    }
  return true;
}

void
TCP::close(void)
{
  printf("TCP::close()\n");
  if (_socket > 0)
    {
      shutdown(_socket, SHUT_RDWR);
      ::close(_socket);
    }
  _socket = 0;
  _is_open = false;
}

void
TCP::poll(void)
{
  char buf[512];

  if (_socket == 0)
    return;
  
  if (_send_data != NULL)
    return;

  int r = recv(_socket, buf, sizeof(buf), MSG_DONTWAIT);

  if (r < 0)
    {
      switch (errno)
        {
        case EAGAIN:
          //printf("TCP::poll(): recv() returned %d / EAGAIN\n", r);
          break;
        default:
          printf("TCP::poll(): recv() returned %d / errno = %d\n", r, errno);
          break;
        }
      return;
    }

  if (r == 0)
    {
      close();
      return;
    }
  printf("TCP::poll(): recv() returned %d\n", r);

  _send_data = new SocketData(r);
  for (int a = 0;a < r;a++)
    _send_data->put_byte(buf[a]);
}

void
TCP::send(SocketData *data)
{
  printf("TCP::send(): len = %d\n", data->length());
  printf("TCP::send(): data = ");
  for (int a = 0;a < data->length();a++)
    printf("%02x ", data->get(a));
  printf("\n");
  int n = ::send(_socket, data->get(), data->length(), 0);
  printf("TCP::send(): send() returned %d\n", n);
}

SocketData *
TCP::receive(void)
{
  if (_send_data == NULL)
    return NULL;

  SocketData *ret = _send_data;
  printf("TCP::receive(): len = %d\n", ret->length());
  printf("TCP::receive(): data = ");
  for (int a = 0;a < ret->length();a++)
    printf("%02x ", ret->get(a));
  printf("\n");
  _send_data = NULL;
  return ret;
}
