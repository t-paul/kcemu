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

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "kc/system.h"

#include "kc/kc.h"

#include "kc/prefs/prefs.h"

#include "kc/kcnet/ipraw.h"

#include "libdbg/dbg.h"

IPRAW::IPRAW(void)
{
  _socket = 0;
  _data = NULL;
  _send_data = NULL;
}

IPRAW::~IPRAW(void)
{
}

void
IPRAW::set_ip_address(byte_t a0, byte_t a1, byte_t a2, byte_t a3)
{
  printf("IPRAW::set_ip_address(): %d.%d.%d.%d\n", a0, a1, a2, a3);
  _ip0 = a0;
  _ip1 = a1;
  _ip2 = a2;
  _ip3 = a3;
}

void
IPRAW::set_port(word_t port)
{
  printf("IPRAW::set_port(): %d\n", port);
  _port = port;
}

bool
IPRAW::open(void)
{
  printf("IPRAW::open()\n");
  return true;
}

bool
IPRAW::is_open(void)
{
  return true;
}

void
IPRAW::close(void)
{
  printf("IPRAW::close()\n");
  close_socket();
}

void
IPRAW::close_socket(void)
{
  shutdown(_socket, SHUT_RDWR);
  ::close(_socket);
  _socket = 0;
}

void
IPRAW::poll(void)
{
  if (_socket == 0)
    return;
  
  int ret = connect(_socket, (struct sockaddr *)&_send_addr, sizeof(_send_addr));
  if (ret < 0)
    {
      switch (errno)
        {
        case EINTR:
          printf("IPRAW::send(): connect() failed with EINTR\n");
          break;
        case EINPROGRESS:
          printf("IPRAW::send(): connect() failed with EINPROGRESS\n");
          break;
        case EALREADY:
          printf("IPRAW::send(): connect() failed with EALREADY\n");
          break;
        case EISCONN:
          _send_data = _data;
          _data = NULL;
          close_socket();
          break;
        default:
          printf("IPRAW::send(): connect() failed with errno %d\n", errno);
          if (_data)
            delete _data;
          _data = NULL;
          _send_data = NULL;
          close_socket();
          break;
        }
    }
}

void
IPRAW::send(SocketData *data)
{
  if (data->get(0) != 8) // ECHO REQUEST
    {
      printf("IPRAW::send(): not an echo request: data = ");
      for (int a = 0;a < data->length();a++)
        printf("%02x ", data->get(a));
      printf("\n");
      return;
    }

  char buf[4096];
  snprintf(buf, sizeof(buf), "%d.%d.%d.%d", _ip0, _ip1, _ip2, _ip3);

  memset(&_send_addr, 0, sizeof (_send_addr));
  _send_addr.sin_family = AF_INET;
  _send_addr.sin_port = htons(80);
  inet_pton(AF_INET, buf, &_send_addr.sin_addr);

  printf("IPRAW::send(): len = %d - %s:%d\n", data->length(), inet_ntoa(_send_addr.sin_addr), _port);
  printf("IPRAW::send(): data = ");
  for (int a = 0;a < data->length();a++)
    printf("%02x ", data->get(a));
  printf("\n");

  int s = socket(AF_INET, SOCK_STREAM, 0);
  if (fcntl(s, F_SETFL, O_NONBLOCK) < 0)
    return;

  _data = new SocketData(data->length() + 6);
  _data->put_byte(_send_addr.sin_addr.s_addr);
  _data->put_byte(_send_addr.sin_addr.s_addr >> 8);
  _data->put_byte(_send_addr.sin_addr.s_addr >> 16);
  _data->put_byte(_send_addr.sin_addr.s_addr >> 24);
  _data->put_word(data->length());
  _data->put_byte(0); // ECHO REPLY
  _data->put_byte(0); // code
  _data->put_word(0); // dummy checksum
  for (int a = 4;a < data->length();a++)
    _data->put_byte(data->get(a));

  word_t sum = checksum(_data, 6);
  _data->put(8, sum);
  _data->put(9, sum >> 8);
  printf("IPRAW::send(): new checksum: %04x\n", sum);

  _socket = s;
  poll();
}

/**
 * Based on example code in RFC1071.
 */
word_t
IPRAW::checksum(SocketData *data, int offset)
{
  int idx = offset;
  int count = data->length() - idx;

  dword_t sum = 0;

  while (count > 1)
    {
      sum += data->get(idx) | data->get(idx + 1) << 8;
      idx += 2;
      count -= 2;
    }

  /*  Add left-over byte, if any */
  if (count > 0)
    sum += data->get(idx);

  /*  Fold 32-bit sum to 16 bits */
  while (sum >> 16)
    sum = (sum & 0xffff) + (sum >> 16);

  return ~sum;
}

SocketData *
IPRAW::receive(void)
{
  if (_send_data == NULL)
    return NULL;

  SocketData *ret = _send_data;
  printf("IPRAW::receive(): len = %d\n", ret->length());
  printf("IPRAW::receive(): data = ");
  for (int a = 0;a < ret->length();a++)
    printf("%02x ", ret->get(a));
  printf("\n");
  _send_data = NULL;
  return ret;
}