/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2004 Torsten Paul
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
#include "kc/prefs/prefs.h"

#include "kc/kc.h"
#include "kc/z80.h"
#include "kc/basickc/memory.h"

using namespace std;

MemoryBasicKC::MemoryBasicKC(void) : Memory()
{
  load_rom(SystemROM::ROM_KEY_SYSTEM, &_rom_monitor[0]);

  memset(&_rom_basic[0], 0xff, 0x4000);
  if (Preferences::instance()->get_kc_variant() == KC_VARIANT_BASICKC_F)
    load_rom(SystemROM::ROM_KEY_BASIC, &_rom_basic[0]);

  memory_group_t mem[] = {
    { &_m_scr,         "-",            0x0000, 0x10000, 0,                256, 0, 1, -1 },
    { &_m_ram,         "RAM",          0x0000, 0x10000, &_ram[0],          10, 0, 1, -1 },
    { &_m_rom_monitor, "Monitor",      0x0000,  0x1000, &_rom_monitor[0],   1, 1, 1, -1 },
    { &_m_rom_basic,   "BASIC",        0x4000,  0x4000, &_rom_basic[0],     1, 1, 1, KC_VARIANT_BASICKC_F },
    { 0, },
  };
  init_memory_groups(mem);

  reset(true);
  z80->register_ic(this);
}

MemoryBasicKC::~MemoryBasicKC(void)
{
  z80->unregister_ic(this);

  delete _m_scr;
  delete _m_rom_monitor;
  delete _m_rom_basic;
  delete _m_ram;
}

byte_t
MemoryBasicKC::memRead8(word_t addr)
{
  return _memrptr[addr >> MemArea::PAGE_SHIFT][addr & MemArea::PAGE_MASK];
}

void
MemoryBasicKC::memWrite8(word_t addr, byte_t val)
{
  _memwptr[addr >> MemArea::PAGE_SHIFT][addr & MemArea::PAGE_MASK] = val;
}

byte_t *
MemoryBasicKC::get_irm(void)
{
  return _ram;
}

byte_t *
MemoryBasicKC::get_char_rom(void)
{
  return NULL;
}

void
MemoryBasicKC::reset(bool power_on)
{
  if (_m_rom_basic)
    _m_rom_basic->set_active(true);
  
  _m_rom_monitor->set_active(true);
  reload_mem_ptr();

  if (power_on)
    {
      scratch_mem(&_ram[0], 0x10000);
    }
}

bool
MemoryBasicKC::is_rom_enabled(void)
{
  return _m_rom_monitor->is_active();
}

void
MemoryBasicKC::set_rom_enabled(bool val)
{
  if (val != _m_rom_monitor->is_active())
    {
      if (_m_rom_basic)
        _m_rom_basic->set_active(val);
      _m_rom_monitor->set_active(val);
      reload_mem_ptr();
    }
}

void
MemoryBasicKC::dumpCore(void)
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
