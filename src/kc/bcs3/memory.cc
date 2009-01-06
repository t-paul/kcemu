/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2009 Torsten Paul
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

#include <stdlib.h>
#include <fstream>

#include "kc/system.h"
#include "kc/prefs/types.h"

#include "kc/kc.h"
#include "kc/z80.h"
#include "kc/bcs3/memory.h"
#include "kc/bcs3/keyboard.h"

using namespace std;

MemoryBCS3::MemoryBCS3(void) : Memory()
{
  load_rom(SystemROM::ROM_KEY_SYSTEM, &_rom);
  load_rom(SystemROM::ROM_KEY_CHARGEN, &_chargen);

  memory_group_t mem[] = {
    { &_m_scr, "-",            0x0000, 0x10000, 0,        256, 0, 1, -1 },
    { &_m_rom, "Monitor",      0x0000,  0x1000, &_rom[0],   1, 1, 1, -1 },
    { &_m_ram, "RAM",          0x3c00,  0x0400, &_ram[0],   1, 0, 1, -1 },
    { 0, },
  };
  init_memory_groups(mem);

  reset(true);
  z80->register_ic(this);
}

MemoryBCS3::~MemoryBCS3(void)
{
  z80->unregister_ic(this);

  delete _m_scr;
  delete _m_rom;
  delete _m_ram;
}

byte_t
MemoryBCS3::memRead8(word_t addr)
{
  if ((addr & 0xf800) == 0x1000)
    return ((KeyboardBCS3 *)keyboard)->memory_read(addr);
  
  return _memrptr[addr >> MemArea::PAGE_SHIFT][addr & MemArea::PAGE_MASK];
}

void
MemoryBCS3::memWrite8(word_t addr, byte_t val)
{
  _memwptr[addr >> MemArea::PAGE_SHIFT][addr & MemArea::PAGE_MASK] = val;
}

byte_t *
MemoryBCS3::get_irm(void)
{
  return 0;
}

byte_t *
MemoryBCS3::get_char_rom(void)
{
  return _chargen;
}

void
MemoryBCS3::reset(bool power_on)
{
  if (!power_on)
    return;

  scratch_mem(&_ram[0], 0x0400);
}

void
MemoryBCS3::dumpCore(void)
{
  ofstream os;

  os.open("core.z80");

  cerr << "Memory: dumping core..." << endl;
  if (!os)
    {
      cerr << "Memory: can't write 'core.z80'" << endl;
      return;
    }

  for (int a = 0;a < 0x10000;a++)
    os.put(memRead8(a));

  os.close();
  cerr << "Memory: done." << endl;
}
