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

#include <stdio.h>
#include <iostream>
#include <iomanip>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/basickc/ports.h"
#include "kc/basickc/memory.h"

#include "libdbg/dbg.h"

PortsBasicKC::PortsBasicKC(void)
{
  _rom_enabled = false;
  _video_base = 0xc000;
}

PortsBasicKC::~PortsBasicKC(void)
{
}

byte_t
PortsBasicKC::in(word_t addr)
{
  byte_t val = 0xff;
  DBG(2, form("KCemu/ports/basickc/in",
              "PortsBasicKC::in():  addr = %04x <= val = %02x\n",
              addr, val));
  
  return val;
}

void
PortsBasicKC::out(word_t addr, byte_t val)
{
  switch (addr & 0xff)
    {
    case 0x00:
    case 0x01:
    case 0x02:
    case 0x03:
      _rom_enabled = false;
      break;
    case 0x04:
    case 0x05:
    case 0x06:
    case 0x07:
      _rom_enabled = true;
      break;
    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
      _video_base = val & 0xe0;
      break;
    }

  ((MemoryBasicKC *)memory)->set_rom_enabled(_rom_enabled);

  DBG(2, form("KCemu/ports/basickc/out",
              "PortsBasicKC::out(): addr = %04x <= val = %02x\n",
              addr, val));
}

bool
PortsBasicKC::is_rom_enabled(void)
{
  return _rom_enabled;
}

byte_t
PortsBasicKC::get_video_base(void)
{
  return _video_base;
}
