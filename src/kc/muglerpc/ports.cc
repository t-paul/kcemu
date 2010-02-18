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
#include "kc/muglerpc/ports.h"
#include "kc/muglerpc/memory.h"

#include "libdbg/dbg.h"

#define memory ((MemoryMuglerPC *)memory)

PortsMuglerPC::PortsMuglerPC(void)
{
  _val_0x94 = 0;
}

PortsMuglerPC::~PortsMuglerPC(void)
{
}

byte_t
PortsMuglerPC::in(word_t addr)
{
  byte_t val;
  byte_t a = addr & 0xff;
  
  switch (a)
    {
    case 0x94:
    case 0x95:
    case 0x96:
    case 0x97:
      val = _val_0x94;
      break;
    default:
      DBG(0, form("KCemu/internal_error",
		  "PortsMuglerPC: called in() with undefined address %04xh\n",
		  addr));
      break;
    }

  DBG(2, form("KCemu/Ports/muglerpc/in",
	      "PortsMuglerPC: in() addr = %04x (returning %02x)\n",
	      addr, val));
  
  return val;
}

void
PortsMuglerPC::out(word_t addr, byte_t val)
{
  byte_t a = addr & 0xff;

  DBG(2, form("KCemu/Ports/muglerpc/out",
              "PortsMuglerPC: out() addr = %04x, val = %02x\n",
              addr, val));
  
  switch (a)
    {
    case 0x94:
    case 0x95:
    case 0x96:
    case 0x97:
      change_0x94(_val_0x94 ^ val, val);
      _val_0x94 = val;
      break;
    default:
      DBG(0, form("KCemu/internal_error",
		  "PortsMuglerPC: called out() with undefined address %04xh (val = %02xh)\n",
		  addr, val));
      break;
    }
}

void
PortsMuglerPC::change_0x94(byte_t changed, byte_t val)
{
  /*
   *  memory switch:
   *
   *  bit 0:  \
   *  bit 1:   >  ram block select (block 0 to 2 on base system)
   *  bit 2:  /
   *  bit 3:  SEL 0 (on system connector X3)
   *  bit 4:  SEL 1 (on system connector X3)
   *  bit 5:  SEL 2 (on system connector X3)
   *  bit 6:  force block 0 at c000h - ffffh
   *  bit 7:  enable/disable EPROM at 0000h (0 = on, 1 = off)
   *
   */
  if (changed & 0x47)
    {
      DBG(2, form("KCemu/Ports/muglerpc/change/94",
		  "PortsMuglerPC: change_0x94() select ram block %d, %sforce block 0 at c000h\n",
		  val & 7,
		  (val & 0x40) ? "" : "don't "));
      memory->select_ram_block(val & 7, (val & 0x40) == 0x40);
    }
  if (changed & 0x80)
    {
      DBG(2, form("KCemu/Ports/muglerpc/change/94",
		  "PortsMuglerPC: change_0x94() set eprom %s\n",
		  (val & 0x80) ? "inactive" : "active"));
      memory->select_eprom((val & 0x80) == 0);
    }
}
