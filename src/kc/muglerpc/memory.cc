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
#include "kc/prefs/types.h"

#include "kc/kc.h"
#include "kc/z80.h"
#include "kc/muglerpc/memory.h"

using namespace std;

MemoryMuglerPC::MemoryMuglerPC(void) : Memory()
{
  load_rom(SystemROM::ROM_KEY_SYSTEM, &_rom);
  load_rom(SystemROM::ROM_KEY_CHARGEN, &_rom_chargen);

  memory_group_t mem[] = {
    { &_m_scr,             "-",               0x0000, 0x10000, 0,                   256, 0, 1 },
    { &_m_rom,             "ROM",             0x0000,  0x2000, &_rom[0],             10, 1, 1 },
    { &_m_ram0_lo,         "RAM Bank 0 (lo)", 0x0000,  0xc000, &_ram_block0[0],      20, 0, 1 },
    { &_m_ram0_hi,         "RAM Bank 0 (hi)", 0xc000,  0x4000, &_ram_block0[0xc000], 20, 0, 1 },
    { &_m_ram1,            "RAM Bank 1",      0x0000, 0x10000, &_ram_block1[0],      30, 0, 0 },
    { &_m_ram2,            "RAM Bank 2",      0x0000, 0x10000, &_ram_block2[0],      30, 0, 0 },
    /*
     *  The irm is not set to active because it is currently not used via the normal memory
     *  switch. Instead it's always written to in memWrite8(). All writes to to both the active
     *  ram block and the irm. Read access to irm is not possible and always goes to the active
     *  ram block.
     */
    { &_m_irm,             "IRM",             0xf800,  0x0800, &_irm[0],             10, 0, 0 },
    { 0, },
  };
  init_memory_groups(mem);

  _m_irm->set_read_through(true);

  reset(true);
  z80->register_ic(this);
}

MemoryMuglerPC::~MemoryMuglerPC(void)
{
  z80->unregister_ic(this);

  delete _m_scr;
  delete _m_rom;
  delete _m_ram0_lo;
  delete _m_ram0_hi;
  delete _m_ram1;
  delete _m_ram2;
  delete _m_irm;
}

byte_t
MemoryMuglerPC::memRead8(word_t addr)
{
  return _memrptr[addr >> MemArea::PAGE_SHIFT][addr & MemArea::PAGE_MASK];
}

void
MemoryMuglerPC::memWrite8(word_t addr, byte_t val)
{
  _memwptr[addr >> MemArea::PAGE_SHIFT][addr & MemArea::PAGE_MASK] = val;
  if (addr >= 0xf800)
    _irm[addr - 0xf800] = val;
}

byte_t *
MemoryMuglerPC::get_irm(void)
{
  return _irm;
}

byte_t *
MemoryMuglerPC::get_char_rom(void)
{
  return _rom_chargen;
}

void
MemoryMuglerPC::reset(bool power_on)
{
  _m_rom->set_active(true);
  _m_ram0_lo->set_active(true);
  _m_ram0_hi->set_active(true);
  _m_ram1->set_active(false);
  _m_ram2->set_active(false);
  _m_irm->set_active(false); // see above
  reload_mem_ptr();

  if (!power_on)
    return;

  scratch_mem(&_irm[0], 0x800);
  scratch_mem(&_ram_block0[0], 0x10000);
  scratch_mem(&_ram_block1[0], 0x10000);
  scratch_mem(&_ram_block2[0], 0x10000);
}

void
MemoryMuglerPC::select_eprom(bool active)
{
  _m_rom->set_active(active);
  reload_mem_ptr();
}

void
MemoryMuglerPC::select_ram_block(int block, bool force_ram0_at_c000h)
{
  _m_ram0_lo->set_active(block == 0);
  _m_ram0_hi->set_active((block == 0) || force_ram0_at_c000h);
  _m_ram1->set_active(block == 1);
  _m_ram2->set_active(block == 2);
  reload_mem_ptr();
}

void
MemoryMuglerPC::dumpCore(void)
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
