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

#include <stdlib.h>

#include <fstream>

#include "kc/system.h"
#include "kc/prefs/prefs.h"

#include "kc/z80.h"
#include "kc/memory9.h"

using namespace std;

Memory9::Memory9(void) : Memory()
{
  load_rom(SystemROM::ROM_KEY_SYSTEM1, &_rom_slot0[0x0000]);
  load_rom(SystemROM::ROM_KEY_SYSTEM2, &_rom_slot0[0x8000]);
  load_rom(SystemROM::ROM_KEY_SYSTEM3, &_rom_slot1[0x0000]);
  load_rom(SystemROM::ROM_KEY_SYSTEM4, &_rom_slot1[0x2000]);

  memory_group_t mem[] = {
    { &_m_scr,             "-",       0x0000, 0x10000, 0,                   256, 0, 1, -1 },
    { &_m_rom_slot0_page0, "ROM CGG", 0x0000, 0x04000, &_rom_slot0[0x0000],   0, 1, 1, -1 },
    { &_m_rom_slot0_page1, "ROM CGG", 0x4000, 0x04000, &_rom_slot0[0x4000],   0, 1, 1, -1 },
    { &_m_rom_slot0_page2, "ROM CGG", 0x8000, 0x02000, &_rom_slot0[0x8000],   0, 1, 1, -1 },
    { &_m_rom_slot1_page1, "ROM DSE", 0x4000, 0x04000, &_rom_slot1[0x0000],   5, 1, 0, KC_VARIANT_A5105_A5105 },
    { &_m_ram_slot2_page0, "RAM",     0x0000, 0x04000, &_ram_slot2[0x0000],  10, 0, 0, -1 },
    { &_m_ram_slot2_page1, "RAM",     0x4000, 0x04000, &_ram_slot2[0x4000],  10, 0, 0, -1 },
    { &_m_ram_slot2_page2, "RAM",     0x8000, 0x04000, &_ram_slot2[0x8000],  10, 0, 0, -1 },
    { &_m_ram_slot2_page3, "RAM",     0xc000, 0x04000, &_ram_slot2[0xc000],  10, 0, 1, -1 },
    { 0, },
  };
  init_memory_groups(mem);

  reset(true);
  z80->register_ic(this);
}

Memory9::~Memory9(void)
{
  z80->unregister_ic(this);
}

byte_t
Memory9::memRead8(word_t addr)
{
  return _memrptr[addr >> MemArea::PAGE_SHIFT][addr & MemArea::PAGE_MASK];
}

void
Memory9::memWrite8(word_t addr, byte_t val)
{
  _memwptr[addr >> MemArea::PAGE_SHIFT][addr & MemArea::PAGE_MASK] = val;
}

byte_t *
Memory9::get_irm(void)
{
  return (byte_t *)get_page_addr_r(0xec00);
}

byte_t *
Memory9::get_char_rom(void)
{
  return (byte_t *)0;
}

void
Memory9::reset(bool power_on)
{
  set_page(0, 0);
  set_page(1, 0);
  set_page(2, 0);
  set_page(3, 2);

  if (power_on)
    memset(&_ram_slot2[0], 0, 0x10000);
}

void
Memory9::dumpCore(void)
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

void
Memory9::set_page(int page, int slot)
{
  switch (page)
    {
    case 0:
      _m_rom_slot0_page0->set_active(slot == 0);
      _m_ram_slot2_page0->set_active(slot == 2);
      break;
    case 1:
      _m_rom_slot0_page1->set_active(slot == 0);
      if (_m_rom_slot1_page1)
	_m_rom_slot1_page1->set_active(slot == 1);
      _m_ram_slot2_page1->set_active(slot == 2);
      break;
    case 2:
      _m_rom_slot0_page2->set_active(slot == 0);
      _m_ram_slot2_page2->set_active(slot == 2);
      break;
    case 3:
      _m_ram_slot2_page3->set_active(slot == 2);
      break;
    }

  reload_mem_ptr();
}
