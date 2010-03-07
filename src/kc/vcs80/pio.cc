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

#include <iostream>
#include <iomanip>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/z80.h"
#include "kc/vcs80/pio.h"

#include "libdbg/dbg.h"

PIOVCS80::PIOVCS80(void)
{
}

PIOVCS80::~PIOVCS80(void)
{
}

byte_t
PIOVCS80::in(word_t addr)
{
  byte_t val = 0xff;

  switch (addr & 3)
    {
    case 0:
      val = in_B_CTRL();
      break;
    case 1:
      val = in_A_CTRL();
      break;
    case 2:
      set_B_EXT(0x80, (~(addr & 1)) << 7);
      val = in_B_DATA();
      break;
    case 3:
      val = in_A_DATA();
      break;
    }

  DBG(2, form("KCemu/PIO/vcs80/in",
              "PIOVCS80::in(): addr = %04x, val = %02x\n",
              addr, val));

  return val;
}

void
PIOVCS80::out(word_t addr, byte_t val)
{
  DBG(2, form("KCemu/PIO/vcs80/out",
              "PIOVCS80::out(): addr = %04x, val = %02x\n",
              addr, val));

  switch (addr & 3)
    {
    case 0:
      out_B_CTRL(val);
      break;
    case 1:
      out_A_CTRL(val);
      break;
    case 2:
      out_B_DATA(val);
      break;
    case 3:
      out_A_DATA(val);
      break;
    }
}

void
PIOVCS80::change_A(byte_t changed, byte_t val)
{
}

void
PIOVCS80::change_B(byte_t changed, byte_t val)
{
}
