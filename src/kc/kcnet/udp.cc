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

#include "kc/system.h"

#include "kc/kc.h"

#include "kc/prefs/prefs.h"

#include "kc/kcnet/udp.h"

#include "sys/sysdep.h"

#include "libdbg/dbg.h"

#include "sys/types.h"
#include "sys/socket.h"

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
  
  int broadcast = 1;

  if ((setsockopt(_socket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast)) == -1)
    {
      printf("UDP::open(): can't set broadcast option\n");
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

  printf("poll: socket = %d\n", _socket);
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
  if ((_ip0 == 255) && (_ip1 == 255) && (_ip2 == 255) && (_ip3 == 255) && (_port == 67))
    {
      printf("UDP::send(): BOOTP - size = %d\n", data->length());
      byte_t *buf = new byte_t[data->length()];
      memcpy(buf, data->get(), data->length());
      _send_data = new SocketData(data->length() + 8);
      _send_data->put_byte(_ip0);
      _send_data->put_byte(_ip1);
      _send_data->put_byte(_ip2);
      _send_data->put_byte(_ip3);
      _send_data->put_word(_port);
      _send_data->put_word(data->length());
      buf[0] = 2; //BOOTREPLY
      buf[16] = 192; //YIADDR
      buf[17] = 168;
      buf[18] = 46;
      buf[19] = 30;
      buf[20] = 192; //SIADDR
      buf[21] = 168;
      buf[22] = 46;
      buf[23] = 104;
      strcpy((char *)buf + 44, "dali");
      strcpy((char *)buf + 108, "/tftpboot/kc85.kcc");
      for (int a = 0;a < data->length();a++)
        _send_data->put_byte(buf[a]);
      return;
    }

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
