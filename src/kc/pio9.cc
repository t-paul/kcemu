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

#include <assert.h>
#include <iostream>
#include <iomanip>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/z80.h"
#include "kc/pio9.h"

#include "libdbg/dbg.h"

PIO9::PIO9(void)
{
}

PIO9::~PIO9(void)
{
}

byte_t
PIO9::in(word_t addr)
{
  byte_t val;

  switch (addr & 3)
    {
    case 0:
      val = in_A_DATA();
      break;
    case 1:
      val = in_B_DATA();
      break;
    case 2:
      val = in_A_CTRL();
      break;
    case 3:
      val = in_B_CTRL();
      break;
    default:
      assert(0);
    }

  DBG(2, form("KCemu/PIO/9/in",
              "PIO9::in():  %04xh: addr = %04x, val = %02x\n",
              z80->getPC(), addr, val));

  return val;
}

void
PIO9::out(word_t addr, byte_t val)
{
  DBG(3, form("KCemu/PIO/9/out",
              "PIO9::out(): %04xh: addr = %04x, val = %02x\n",
              z80->getPC(), addr, val));

  switch (addr & 3)
    {
    case 0:
      out_A_DATA(val);
      break;
    case 1:
      out_B_DATA(val);
      break;
    case 2:
      out_A_CTRL(val);
      break;
    case 3:
      out_B_CTRL(val);
      break;
    }
}

void
PIO9::change_A(byte_t changed, byte_t val)
{
}

void
PIO9::change_B(byte_t changed, byte_t val)
{
}

void
PIO9::tape_callback(byte_t val)
{
  static int x = 0;
  //cout << "PIO9::tape_callback(): " << hex << (int)val << endl;

  set_B_EXT(0x80, x);
  x = 0x80 - x;
}
