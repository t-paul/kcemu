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

#include <string>

#include <stdio.h>
#include <unistd.h>

#include "kc/system.h"

#include "kc/kc.h"

#include "kc/prefs/prefs.h"

#include "kc/kcnet/ipraw.h"

#include "sys/sysdep.h"

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
  DBG(2, form("KCemu/KCNET/ipraw",
              "IPRAW::set_ip_address(): %d.%d.%d.%d\n",
              a0, a1, a2, a3));
  _ip0 = a0;
  _ip1 = a1;
  _ip2 = a2;
  _ip3 = a3;
}

void
IPRAW::set_port(word_t port)
{
  DBG(2, form("KCemu/KCNET/ipraw",
              "IPRAW::set_port(): %d\n",
              port));
  _port = port;
}

bool
IPRAW::open(void)
{
  DBG(2, form("KCemu/KCNET/ipraw",
              "IPRAW::open()\n"));
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
  DBG(2, form("KCemu/KCNET/ipraw",
              "IPRAW::close()\n"));
  close_socket();
}

void
IPRAW::close_socket(void)
{
  sys_socket_close(_socket);
  _socket = 0;
}

void
IPRAW::poll(void)
{
  if (_socket == 0)
    return;

  // Use hard coded port 80, this is currently only intended to
  // simulate ping. Because we can't simply send raw ip packets
  // from user space program, we try to connect to web server
  // port 80 and interpet a successful connect as ping reply.
  int ret = sys_socket_connect(_socket, _ip0, _ip1, _ip2, _ip3, 80);

  switch (ret)
    {
    case 0:
    case SYS_SOCKET_ERR_ISCONN:
      _send_data = _data;
      _data = NULL;
      close_socket();
      break;
    case SYS_SOCKET_ERR_INTR:
      DBG(2, form("KCemu/KCNET/ipraw",
                  "IPRAW::send(): connect() failed with EINTR\n"));
      break;
    case SYS_SOCKET_ERR_INPROGRESS:
      DBG(2, form("KCemu/KCNET/ipraw",
                  "IPRAW::send(): connect() failed with EINPROGRESS\n"));
      break;
    case SYS_SOCKET_ERR_ALREADY:
      DBG(2, form("KCemu/KCNET/ipraw",
                  "IPRAW::send(): connect() failed with EALREADY\n"));
      break;
    default:
      DBG(2, form("KCemu/KCNET/ipraw",
                  "IPRAW::send(): connect() failed with error code %d\n",
                  ret));
      if (_data)
        delete _data;
      _data = NULL;
      _send_data = NULL;
      close_socket();
      break;
    }
}

void
IPRAW::send(SocketData *data)
{
  DBG(2, form("KCemu/KCNET/ipraw",
              "IPRAW::send(): len = %d - %d.%d.%d.%d:%d\n",
              data->length(), _ip0, _ip1, _ip2, _ip3, _port));

  string text;
  char buf[10];
  for (int a = 0;a < data->length();a++)
    {
      snprintf(buf, sizeof (buf), " %02x", data->get(a));
      text += buf;
    }
  DBG(2, form("KCemu/KCNET/ipraw",
              "IPRAW::send(): data =%s\n",
              text.c_str()));

  if (data->get(0) != 8) // ECHO REQUEST
    {
      DBG(2, form("KCemu/KCNET/ipraw",
                  "IPRAW::send(): not an echo request!\n"));
      return;
    }

  int s = sys_socket_create(1, 1);
  if (s < 0)
    return;

  _data = new SocketData(data->length() + 6);
  _data->put_byte(_ip0);
  _data->put_byte(_ip1);
  _data->put_byte(_ip2);
  _data->put_byte(_ip3);
  _data->put_word(data->length());
  _data->put_byte(0); // ECHO REPLY
  _data->put_byte(0); // code
  _data->put_word(0); // dummy checksum
  for (int a = 4;a < data->length();a++)
    _data->put_byte(data->get(a));

  word_t sum = checksum(_data, 6);
  _data->put(8, sum);
  _data->put(9, sum >> 8);
  DBG(2, form("KCemu/KCNET/ipraw",
              "IPRAW::send(): new checksum: %04x\n",
              sum));

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

  string text;
  char buf[10];
  for (int a = 0;a < ret->length();a++)
    {
      snprintf(buf, sizeof(buf), " %02x", ret->get(a));
      text += buf;
    }
  DBG(2, form("KCemu/KCNET/ipraw",
              "IPRAW::receive(): len = %d, data =%s\n",
              ret->length(), text.c_str()));

  _send_data = NULL;
  return ret;
}
