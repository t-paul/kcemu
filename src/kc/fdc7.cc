/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
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

#include <ctype.h>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/z80.h"
#include "kc/fdc7.h"

#include "libdbg/dbg.h"

FDC7::FDC7(void)
{
}

FDC7::~FDC7(void)
{
}

long long
FDC7::get_counter()
{
  return z80->getCounter();
}

void
FDC7::add_callback(unsigned long long offset, Callback *cb, void *data)
{
  z80->addCallback(offset, cb, data);
}

byte_t
FDC7::in(word_t addr)
{
  byte_t val = 0;

  switch (addr & 0xff)
    {
    case 0x98: // CPM-Z9 module (Status Register)
    case 0x10: // CPM-Z9 module (Status Register) ???
      val = get_msr();
      break;
    case 0x99: // CPM-Z9 module (Data Register)
    case 0x11: // CPM-Z9 module (Data Register) ??
      val = in_data(addr);
      break;
    case 0xa0: // CPM-Z9 module
      break;
    default:
      DBG(2, form("KCemu/FDC/in_unhandled",
                  "FDC::in(): addr = %04x\n",
                  addr));
      break;
    }

  return val;
}

void
FDC7::out(word_t addr, byte_t val)
{
  switch (addr & 0xff)
    {
    case 0x99: // CPM-Z9 module (Status Register)
      out_data(addr, val);
      break;
    case 0x98: // CPM-Z9 module (Data Register)
      write_byte(val);
      break;
    case 0xa0: // CPM-Z9 module
      set_terminal_count((val & 0x10) == 0x10);

      if ((val & 0x20) == 0x20)
	; // RESET

      break;
    default:
      DBG(2, form("KCemu/FDC/out_unhandled",
                  "FDC::out(): addr = %04x, val = %02x [%c]\n",
                  addr, val, isprint(val) ? val : '.'));
      break;
    }
}
