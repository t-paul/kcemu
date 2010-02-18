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

#include <stdio.h>
#include <iostream>
#include <iomanip>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/ports4.h"
#include "kc/memory4.h"

#include "libdbg/dbg.h"

#define memory ((Memory4 *)memory)

Ports4::Ports4(void)
{
  for (int a = 0;a < NR_PORTS;a++)
    inout[a] = -1;
}

Ports4::~Ports4(void)
{
}

byte_t
Ports4::in(word_t addr)
{
  byte_t a = addr & 0xff;
  byte_t val = inout[a];

  switch (a)
    {
    case 0x84:
    case 0x86:
      break;
    default:
      DBG(0, form("KCemu/internal_error",
		  "Ports4: called in() with undefined address %04xh\n",
		  addr));
      break;
    }

  DBG(2, form("KCemu/Ports/4/in",
	      "Ports4: in() addr = %04x (returning %02x)\n",
	      addr, val));

  return val;
}

void
Ports4::out(word_t addr, byte_t val)
{
  byte_t a = addr & 0xff;

  DBG(2, form("KCemu/Ports/4/out",
              "Ports4: out() addr = %04x, val = %02x\n",
              addr, val));

  switch (a)
    {
    case 0x84:
      if (inout[0x84] < 0)
	inout[0x84] = ~val;
      change_0x84(inout[0x84] ^ val, val);
      break;
    case 0x86:
      if (inout[0x86] < 0)
	inout[0x86] = ~val;
      change_0x86(inout[0x86] ^ val, val);
      break;
    default:
      DBG(0, form("KCemu/internal_error",
		  "Ports4: called out() with undefined address %04xh (val = %02xh)\n",
		  addr, val));
      break;
    }

  inout[addr & 0xff] = val;
}

void
Ports4::change_0x84(byte_t changed, byte_t val)
{
  if (changed & 0x01)
    {
      DBG(2, form("KCemu/Ports/4/change/84",
                  "Ports 0x84: display screen %d\n",
                  val & 1));
      memory->displaySCREEN_1(val & 0x01);
    }
  if (changed & 0x02)
    {
      DBG(2, form("KCemu/Ports/4/change/84",
                  "Ports 0x84: pixel/ color %d\n",
                  (val >> 1) & 1));
      memory->enableCOLOR(val & 0x02);
    }
  if (changed & 0x04)
    {
      DBG(2, form("KCemu/Ports/4/change/84",
                  "Ports 0x84: access screen %d\n",
                  (val >> 2) & 1));
      memory->enableSCREEN_1(val & 0x04);
    }
  if (changed & 0x08)
    {
      DBG(2, form("KCemu/Ports/4/change/84",
                  "Ports 0x84: high resolution %d\n",
                  (val >> 3) & 1));
    }
  if (changed & 0xf0)
    {
      DBG(2, form("KCemu/Ports/4/change/84",
                  "Ports 0x84: access RAM8 segment %d\n",
                  (val >> 4) & 0x0f));
      memory->selectRAM_8((val >> 4) & 0x0f);
    }
}

void
Ports4::change_0x86(byte_t changed, byte_t val)
{
  if (changed & 0x01)
    {
      DBG(2, form("KCemu/Ports/4/change/86",
                  "Ports 0x86: RAM 4 %d\n",
                  val & 1));
      memory->enableRAM_4(val & 0x01);
    }
  if (changed & 0x02)
    {
      DBG(2, form("KCemu/Ports/4/change/86",
                  "Ports 0x86: write protect RAM 4 %d\n",
                  (val >> 1) & 1));
      memory->protectRAM_4(val & 0x02);
    }
  if (changed & 0x04)
    {
      DBG(2, form("KCemu/Ports/4/change/86",
                  "Ports 0x86: unused 0x04 %d\n",
                  (val >> 2) & 1));
    }
  if (changed & 0x08)
    {
      DBG(2, form("KCemu/Ports/4/change/86",
                  "Ports 0x86: unused 0x08 %d\n",
                  (val >> 3) & 1));
    }
  if (changed & 0x10)
    {
      DBG(2, form("KCemu/Ports/4/change/86",
                  "Ports 0x86: unused 0x10 %d\n",
                  (val >> 4) & 1));
    }
  if (changed & 0x60)
    {
      DBG(2, form("KCemu/Ports/4/change/86",
                  "Ports 0x86: BASIC/USER ROM bank %d\n",
                  (val >> 5) & 3));
      memory->selectROM_C((val >> 5) & 3);
    }
  if (changed & 0x80)
    {
      DBG(2, form("KCemu/Ports/4/change/86",
                  "Ports 0x86: CAOS C %d\n",
                  (val >> 7) & 1));
      memory->enableCAOS_C(val & 0x80);
    }
}
