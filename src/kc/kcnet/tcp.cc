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
#include <errno.h>

#include "kc/system.h"

#include "kc/kc.h"

#include "kc/prefs/prefs.h"

#include "kc/kcnet/tcp.h"

#include "sys/sysdep.h"

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
  _socket = sys_socket_create(1, 1);
  if (_socket < 0)
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

  int ret = sys_socket_connect(_socket, _ip0, _ip1, _ip2, _ip3, _port);
  if (ret == 0)
    return true;

  switch (ret)
    {
    case SYS_SOCKET_ERR_INTR:
      printf("TCP::is_open(): connect() failed with EINTR (%d.%d.%d.%d:%d)\n", _ip0, _ip1, _ip2, _ip3, _port);
      return false;
    case SYS_SOCKET_ERR_INPROGRESS:
      printf("TCP::is_open(): connect() failed with EINPROGRESS (%d.%d.%d.%d:%d)\n", _ip0, _ip1, _ip2, _ip3, _port);
      return false;
    case SYS_SOCKET_ERR_ALREADY:
      printf("TCP::is_open(): connect() failed with EALREADY (%d.%d.%d.%d:%d)\n", _ip0, _ip1, _ip2, _ip3, _port);
      return false;
    case SYS_SOCKET_ERR_ISCONN:
      return true;
    default:
      printf("TCP::is_open(): connect() failed with errno %d (%d.%d.%d.%d:%d)\n", ret, _ip0, _ip1, _ip2, _ip3, _port);
      close();
      return false;
    }
}

void
TCP::close(void)
{
  printf("TCP::close()\n");
  if (_socket > 0)
    {
      sys_socket_close(_socket);
    }
  _socket = 0;
  _is_open = false;
}

void
TCP::poll(void)
{
  unsigned char buf[512];

  if (_socket == 0)
    return;
  
  if (_send_data != NULL)
    return;

  int r = sys_socket_recvfrom(_socket, buf, sizeof(buf), NULL, NULL, NULL, NULL, NULL);

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
  int n = sys_socket_send(_socket, data->get(), data->length());
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
