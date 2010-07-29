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
#include "kc/fdc0s.h"

#include "libdbg/dbg.h"

FDC0S::FDC0S(void)
{
}

FDC0S::~FDC0S(void)
{
}

long long
FDC0S::get_counter()
{
  return z80->getCounter();
}

void
FDC0S::add_callback(unsigned long long offset, Callback *cb, void *data)
{
  z80->addCallback(offset, cb, data);
}

byte_t
FDC0S::in(word_t addr)
{
  byte_t val;

  switch (addr & 0xff)
    {
    case 0xf0: // CFDC -- fdc control
      val = get_msr();
      break;
    case 0xf1: // DFDC -- fdc data
      val = in_data(addr);
      break;
    case 0xf2: // MOAUS -- drive motor off
      val = 0xff;
      break;
    case 0xf4: // MOAUS -- drive motor on (device 1)
      val = 0xff;
      break;
    case 0xf6: // MOTON -- drive motor on (device 0)
      val = 0xff;
      break;
    case 0xf8: // TC -- terminal count
      val = 0xff;
      break;
    case 0xfa: // FDCRES -- fdc reset
      val = 0xff;
      break;
    default:
      DBG(2, form("KCemu/FDC/in_unhandled",
                  "FDC::in(): addr = %04x\n",
                  addr));
      val = 0xff;
      break;
    }

  return val;
}

void
FDC0S::out(word_t addr, byte_t val)
{
  switch (addr & 0xff)
    {
    case 0xf1:
      out_data(addr, val);
      break;
    case 0xf8: // TC -- terminal count
      set_terminal_count(1);
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
