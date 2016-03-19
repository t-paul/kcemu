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
#include <sys/socket.h>
#include <arpa/inet.h>

#include "kc/system.h"

#include "kc/kc.h"

#include "kc/prefs/prefs.h"

#include "kc/kcnet/udp.h"

#include "sys/sysdep.h"

#include "libdbg/dbg.h"

UDP::UDP(void)
{
  _socket = 0;
  _send_data = NULL;
}

UDP::~UDP(void)
{
}

void
UDP::set_ip_address(byte_t a0, byte_t a1, byte_t a2, byte_t a3)
{
  printf("UDP::set_ip_address(): %d.%d.%d.%d\n", a0, a1, a2, a3);
  _ip0 = a0;
  _ip1 = a1;
  _ip2 = a2;
  _ip3 = a3;
}

void
UDP::set_port(word_t port)
{
  printf("UDP::set_port(): %d\n", port);
  _port = port;
}

bool
UDP::open(void)
{
  printf("UDP::open()\n");
  _socket = sys_socket_create(0, 0);
  if (_socket < 0)
    {
      close();
      return false;
    }

  int enable = 1;
  setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (const char *) &enable, sizeof(enable));
#ifdef __APPLE__
  setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (const char *) &enable, sizeof(enable));
#endif

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(_port);

  if (bind(_socket, (struct sockaddr *) &addr, sizeof(addr)) != 0) {
    printf("UDP::bind error\n");
  }

  return true;
}

bool
UDP::is_open(void)
{
  return true;
}

void
UDP::close(void)
{
  if (_socket > 0)
    {
      sys_socket_close(_socket);
    }
  _socket = 0;
}

void
UDP::poll(void)
{
  unsigned short port;
  unsigned char ip0, ip1, ip2, ip3;
  unsigned char buf[4096];

  if (_send_data != NULL)
    return;

//  printf("poll: socket = %d\n", _socket);
  int r = sys_socket_recvfrom(_socket, buf, sizeof(buf), &ip0, &ip1, &ip2, &ip3, &port);
  if (r < 0)
    return;

  printf("UDP::poll(): recvfrom() returned %d - %d.%d.%d.%d:%d\n", r, ip0, ip1, ip2, ip3, port);

  _send_data = new SocketData(r + 8);

  _send_data->put_byte(ip0);
  _send_data->put_byte(ip1);
  _send_data->put_byte(ip2);
  _send_data->put_byte(ip3);
  _send_data->put_word(port);
  _send_data->put_word(r);

  for (int a = 0;a < r;a++)
    _send_data->put_byte(buf[a]);
}

void
UDP::send(SocketData *data)
{
  printf("UDP::send(): len = %d - %d.%d.%d.%d:%d\n", data->length(), _ip0, _ip1, _ip2, _ip3, _port);
  printf("UDP::send(): data = ");
  for (int a = 0;a < data->length();a++)
    printf("%02x ", data->get(a));
  printf("\n");
  int n = sys_socket_sendto(_socket, data->get(), data->length(), _ip0, _ip1, _ip2, _ip3, _port);
  printf("UDP::send(): sendto() returned %d\n", n);
}

SocketData *
UDP::receive(void)
{
  if (_send_data == NULL)
    return NULL;

  SocketData *ret = _send_data;
  printf("UDP::receive(): len = %d\n", ret->length());
  printf("UDP::receive(): data = ");
  for (int a = 0;a < ret->length();a++)
    printf("%02x ", ret->get(a));
  printf("\n");
  _send_data = NULL;
  return ret;
}
