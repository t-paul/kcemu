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

#include "kc/system.h"

#include "kc/kc.h"

#include "kc/prefs/prefs.h"

#include "kc/kcnet/udp.h"

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
  _socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (_socket < 0)
    {
      close();
      return false;
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
  printf("UDP::close()\n");
  if (_socket > 0)
    {
      shutdown(_socket, SHUT_RDWR);
      ::close(_socket);
    }
  _socket = 0;
}

void
UDP::poll(void)
{
  char buf[4096];

  if (_send_data != NULL)
    return;
  
  struct sockaddr_storage peer_addr;
  socklen_t peer_addr_len;
  int r = recvfrom(_socket, buf, sizeof (buf), MSG_DONTWAIT, (struct sockaddr *) &peer_addr, &peer_addr_len);
  if (r < 0)
    return;

  struct sockaddr_in *addr;
  addr = (sockaddr_in *)&peer_addr;
   word_t port = ntohs(addr->sin_port);
  printf("UDP::poll(): recvfrom() returned %d - %s:%d\n", r, inet_ntoa(addr->sin_addr), port);

  _send_data = new SocketData(r + 8);

  _send_data->put_byte(addr->sin_addr.s_addr);
  _send_data->put_byte(addr->sin_addr.s_addr >> 8);
  _send_data->put_byte(addr->sin_addr.s_addr >> 16);
  _send_data->put_byte(addr->sin_addr.s_addr >> 24);
  _send_data->put_word(port);
  _send_data->put_word(r);

  for (int a = 0;a < r;a++)
    _send_data->put_byte(buf[a]);
}

void
UDP::send(SocketData *data)
{
  char buf[4096];
  snprintf(buf, sizeof(buf), "%d.%d.%d.%d", _ip0, _ip1, _ip2, _ip3);

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof (addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(_port);
  inet_pton(AF_INET, buf, &addr.sin_addr);

  printf("UDP::send(): len = %d - %s:%d\n", data->length(), inet_ntoa(addr.sin_addr), _port);
  printf("UDP::send(): data = ");
  for (int a = 0;a < data->length();a++)
    printf("%02x ", data->get(a));
  printf("\n");
  int n = sendto(_socket, data->get(), data->length(), 0, (sockaddr*)&addr, sizeof(addr));
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
