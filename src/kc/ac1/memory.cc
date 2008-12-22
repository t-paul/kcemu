/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2008 Torsten Paul
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
#include "kc/ac1/memory.h"

using namespace std;

MemoryAC1::MemoryAC1(void) : Memory()
{
  load_rom(SystemROM::ROM_KEY_SYSTEM, &_rom_monitor);
  load_rom(SystemROM::ROM_KEY_CHARGEN, &_chargen);

  if (Preferences::instance()->get_kc_variant() != KC_VARIANT_AC1_SCCH)
    load_rom(SystemROM::ROM_KEY_BASIC, &_rom_basic);

  memory_group_t mem[] = {
    { &_m_scr,         "-",            0x0000, 0x10000, 0,                256, 0, 1, -1 },
    { &_m_rom_monitor, "Monitor",      0x0000,  0x0800, &_rom_monitor[0],   1, 1, 1, KC_VARIANT_AC1_16 },
    { &_m_rom_basic,   "BASIC",        0x0800,  0x0800, &_rom_basic[0],     1, 1, 1, KC_VARIANT_AC1_16 },
    { &_m_irm,         "IRM",          0x1000,  0x0400, &_irm[0],           1, 0, 1, KC_VARIANT_AC1_16 },
    { &_m_ram,         "RAM",          0x1800,  0x0400, &_ram[0],           1, 0, 1, KC_VARIANT_AC1_16 },
    { &_m_rom_monitor, "Monitor",      0x0000,  0x0800, &_rom_monitor[0],   1, 1, 1, KC_VARIANT_AC1_32 },
    { &_m_rom_basic,   "BASIC",        0x0800,  0x0800, &_rom_basic[0],     1, 1, 1, KC_VARIANT_AC1_32 },
    { &_m_irm,         "IRM",          0x1000,  0x0800, &_irm[0],           1, 0, 1, KC_VARIANT_AC1_32 },
    { &_m_ram,         "RAM",          0x1800,  0x0800, &_ram[0],           1, 0, 1, KC_VARIANT_AC1_32 },
    { &_m_rom_monitor, "Monitor",      0x0000,  0x1000, &_rom_monitor[0],   1, 1, 1, KC_VARIANT_AC1_SCCH },
    { &_m_irm,         "IRM",          0x1000,  0x0800, &_irm[0],           1, 0, 1, KC_VARIANT_AC1_SCCH },
    { &_m_ram,         "SRAM",         0x1800,  0x0800, &_ram[0],           1, 0, 1, KC_VARIANT_AC1_SCCH },
    { &_m_ram64,       "DRAM",         0x0000, 0x10000, &_ram64[0],        10, 0, 1, KC_VARIANT_AC1_SCCH },
    { 0, },
  };
  init_memory_groups(mem);

  reset(true);
  z80->register_ic(this);
}

MemoryAC1::~MemoryAC1(void)
{
  z80->unregister_ic(this);

  delete _m_scr;
  delete _m_rom_monitor;
  delete _m_rom_basic;
  delete _m_irm;
  delete _m_ram;
}

byte_t
MemoryAC1::memRead8(word_t addr)
{
  return _memrptr[addr >> MemArea::PAGE_SHIFT][addr & MemArea::PAGE_MASK];
}

void
MemoryAC1::memWrite8(word_t addr, byte_t val)
{
  _memwptr[addr >> MemArea::PAGE_SHIFT][addr & MemArea::PAGE_MASK] = val;
}

byte_t *
MemoryAC1::get_irm(void)
{
  return _irm;
}

byte_t *
MemoryAC1::get_char_rom(void)
{
  return _chargen;
}

void
MemoryAC1::reset(bool power_on)
{
  if (!power_on)
    return;

  scratch_mem(&_irm[0], 0x0800);
  scratch_mem(&_ram[0], 0x0800);
  scratch_mem(&_ram64[0], 0x10000);
}

void
MemoryAC1::dumpCore(void)
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
