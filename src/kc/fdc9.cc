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

#include <ctype.h>

#include "kc/system.h"

#include "kc/z80.h"
#include "kc/fdc9.h"

#include "libdbg/dbg.h"

FDC9::FDC9(void)
{
}

FDC9::~FDC9(void)
{
}

long long
FDC9::get_counter()
{
  return z80->getCounter();
}

void
FDC9::add_callback(unsigned long long offset, Callback *cb, void *data)
{
  z80->addCallback(offset, cb, data);
}

byte_t
FDC9::in(word_t addr)
{
  byte_t val = 0;

  switch (addr & 0xff)
    {
    case 0x40:
      val = get_msr();
      DBG(2, form("KCemu/FDC/in_F0",
                  "FDC::in(): addr = %04x, val = %02x [%c]\n",
                  addr, val, isprint(val) ? val : '.'));
      break;
    case 0x41:
      val = in_data(addr);
      DBG(2, form("KCemu/FDC/in_F1",
                  "FDC::in(): addr = %04x, val = %02x [%c]\n",
                  addr, val, isprint(val) ? val : '.'));
      break;
    case 0x42:
      val = read_byte();
      DBG(2, form("KCemu/FDC/in_F2",
                  "FDC::in(): addr = %04x, val = %02x [%c]\n",
                  addr, val, isprint(val) ? val : '.'));
      break;
    case 0x44:
      val = get_input_gate();
      DBG(2, form("KCemu/FDC/in_F4",
                  "FDC::in(): addr = %04x, val = %02x [%c]\n",
                  addr, val, isprint(val) ? val : '.'));
      break;
    case 0x46:
      val = 0x00;
      DBG(2, form("KCemu/FDC/in_F6",
                  "FDC::in(): addr = %04x, val = %02x [%c]\n",
                  addr, val, isprint(val) ? val : '.'));
      break;
    case 0x48:
      val = 0x00;
      DBG(2, form("KCemu/FDC/in_F8",
                  "FDC::in(): addr = %04x, val = %02x [%c]\n",
                  addr, val, isprint(val) ? val : '.'));
      break;
    }

  DBG(2, form("KCemu/FDC/in",
              "FDC::in(): addr = %04x, val = %02x\n",
              addr, val));

  return val;
}

void
FDC9::out(word_t addr, byte_t val)
{
  switch (addr & 0xff)
    {
    case 0x41:
      DBG(2, form("KCemu/FDC/out_F1",
                  "FDC::out(): addr = %04x, val = %02x [%c]\n",
                  addr, val, isprint(val) ? val : '.'));
      out_data(addr, val);
      break;
    case 0x42:
      DBG(2, form("KCemu/FDC/out_F2",
                  "FDC::out(): addr = %04x, val = %02x [%c]\n",
                  addr, val, isprint(val) ? val : '.'));
      write_byte(val);
      break;
    case 0x46:
      DBG(2, form("KCemu/FDC/out_F6",
                  "FDC::out(): addr = %04x, val = %02x [%c]\n",
                  addr, val, isprint(val) ? val : '.'));
      break;
    case 0x48:
      DBG(2, form("KCemu/FDC/out_F8",
                  "FDC::out(): TC %02x\n",
                  val));
      if (val == 0x00)
	set_state(FDC_STATE_IDLE);
      if (val == 0x11)
	set_state(FDC_STATE_RESULT);

      set_input_gate(0x40, 0x00);
      break;
    default:
      DBG(2, form("KCemu/FDC/out_unhandled",
                  "FDC::out(): addr = %04x, val = %02x [%c]\n",
                  addr, val, isprint(val) ? val : '.'));
      break;
    }
}
