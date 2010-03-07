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

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/pio0.h"
#include "kc/tape.h"

#include "libdbg/dbg.h"

PIO0::PIO0(void)
{
}

PIO0::~PIO0(void)
{
}

byte_t
PIO0::in(word_t addr)
{
  byte_t ret = 0;

  switch (addr & 3)
    {
    case 0:
      ret = in_A_DATA();
      break;
    case 1:
      ret = in_A_CTRL();
      break;
    case 2:
      ret = in_B_DATA();
      break;
    case 3:
      ret = in_B_CTRL();
      break;
    }

  DBG(2, form("KCemu/PIO/0/in",
              "PIO0::in():  addr = %04x, val = %02x\n",
              addr, ret));

  return ret;
}

void
PIO0::out(word_t addr, byte_t val)
{
  DBG(2, form("KCemu/PIO/0/out",
              "PIO0::out(): addr = %04x, val = %02x\n",
              addr, val));

  switch (addr & 3)
    {
    case 0:
      out_A_DATA(val);
      break;
    case 1:
      out_A_CTRL(val);
      break;
    case 2:
      out_B_DATA(val);
      break;
    case 3:
      out_B_CTRL(val);
      break;
    }
}

void
PIO0::change_A(byte_t changed, byte_t val)
{
}

void
PIO0::change_B(byte_t changed, byte_t val)
{
  if (changed & 0x80)
    tape->tape_signal();
}

void
PIO0::tape_callback(byte_t val)
{
  static int x = 0;

  set_B_EXT(0x40, x);
  x = 0x40 - x;
}
