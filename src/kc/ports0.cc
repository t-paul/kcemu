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

#include <unistd.h> // sleep();

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/pio.h"
#include "kc/keyb0.h"
#include "kc/ports0.h"

#include "libdbg/dbg.h"

Ports0::Ports0(void)
{
  for (int a = 0;a < NR_PORTS;a++)
    inout[a] = 0xff;
}

Ports0::~Ports0(void)
{
}

byte_t
Ports0::in(word_t addr)
{
  byte_t a = addr & 0xff;
  byte_t val = inout[a];
  
  switch (a)
    {
    case 0x08:
      break;
    default:
      DBG(0, form("KCemu/internal_error",
		  "Ports0: called in() with undefined address %04xh\n",
		  addr));
      break;
    }

  DBG(2, form("KCemu/Ports/0/in",
	      "Ports0: in() addr = %04x (returning %02x)\n",
	      addr, val));
  
  return val;
}

void
Ports0::out(word_t addr, byte_t val)
{
  byte_t a = addr & 0xff;

  DBG(2, form("KCemu/Ports/0/out",
              "Ports0: out() addr = %04x, val = %02x\n",
              addr, val));
  
  switch (a)
    {
    case 0x08:
      change_0x08(inout[a] ^ val, val);
      break;
    default:
      DBG(0, form("KCemu/internal_error",
		  "Ports0: called out() with undefined address %04xh (val = %02xh)\n",
		  addr, val));
      break;
    }

  inout[a] = val;
}

void
Ports0::change_0x08(byte_t changed, byte_t val)
{
  ((Keyboard0 *)keyboard)->set_value(val & 0x0f);
}
