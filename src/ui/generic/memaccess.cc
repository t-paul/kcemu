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

#include <string.h>
#include <stdlib.h> // DEBUG

#include "kc/system.h"
#include "kc/prefs/prefs.h"

#include "kc/kc.h"
#include "kc/z80.h"

#include "ui/generic/memaccess.h"

#include "libdbg/dbg.h"

MemAccess::MemAccess(void)
{
  reset();
}

MemAccess::~MemAccess(void)
{
}

void
MemAccess::update(void)
{
  _base_counter = z80->getCounter();
}

void
MemAccess::set_value(int x, int y, byte_t value)
{
  _buf[40 * y + x] = value;
}

byte_t
MemAccess::get_value(int x, int y)
{
  int idx = 40 * y + x;
  
  byte_t val = _buf[idx];
  if (val > 0)
    _buf[idx]--;

  return val;
}

void
MemAccess::reset()
{
  _base_counter = 0;
  memset(_buf, 0, BUFFER_SIZE);
}

void
MemAccess::memory_read(word_t addr)
{
  DBG(3, form("KCemu/MemAccess/read",
	      "MemAccess::memory_read():  addr = %04x\n",
	      addr));
}

void
MemAccess::memory_write(word_t addr)
{
  static double line_val = 35000.0 / 288.0;
  static int val = Preferences::instance()->get_int_value("display_mem_access", 0) ? 2 : 0;

  DBG(3, form("KCemu/MemAccess/write",
	      "MemAccess::memory_write(): addr = %04x\n",
	      addr));

  if (_base_counter == 0)
    return;

  double diff = z80->getCounter() - _base_counter;
  if (diff >= 35000)
    return;

  int y = (int)(diff / line_val);
  if (y >= 256)
    return;

  diff -= (y * line_val);
  int x = (int)((diff * 384.0) / line_val);
  if (x >= 320)
    return;

  x /= 8;

  _buf[40 * y + x] = val;
}
