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

#include "kc/system.h"

#include "kc/kc.h"

#include "kc/kcnet/socket.h"

#include "sys/sysdep.h"

#include "libdbg/dbg.h"

SocketData::SocketData(int len)
{
  _len = len;
  _idx = 0;
  _buf = new byte_t[len];
}

SocketData::~SocketData(void)
{
  delete[] _buf;
}

int
SocketData::length(void)
{
  return _len;
}

void
SocketData::put_byte(byte_t val)
{
  if (_idx < _len)
    {
      _buf[_idx++] = val;
    }
}

void
SocketData::put_word(word_t val)
{
  word_t nval = sys_htons(val);
  put_byte(nval);
  put_byte(nval >> 8);
}

void
SocketData::put_long(dword_t val)
{
  word_t nval = sys_htonl(val);
  put_byte(nval);
  put_byte(nval >> 8);
  put_byte(nval >> 16);
  put_byte(nval >> 24);
}

void
SocketData::put_text(const char *text)
{
  while (*text != 0)
    put_byte(*text++);
}

void
SocketData::put(int idx, byte_t val)
{
  if (idx < _len)
    _buf[idx] = val;
}

byte_t *
SocketData::get(void)
{
  return _buf;
}

byte_t
SocketData::get(int idx)
{
  if (idx < _len)
    return _buf[idx];

  return 0xff;
}
