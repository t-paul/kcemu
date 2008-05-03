/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: memory3.cc,v 1.12 2002/06/09 14:24:33 torsten_paul Exp $
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
#include "kc/memory3.h"

#include "ui/ui.h"

using namespace std;

Memory3::Memory3(void) : Memory()
{
  _access_color = false;

  load_rom(SystemROM::ROM_KEY_CAOSE, &_rom_caos);
  load_rom(SystemROM::ROM_KEY_BASIC, &_rom_basic);

  memory_group_t mem[] = {
    { &_m_scr,   "-",     0x0000, 0x10000, 0,            256, 0, 1, -1 },
    { &_m_ram,   "RAM 0", 0x0000,  0x4000, &_ram[0],       0, 0, 1, -1 },
    { &_m_irm,   "IRM",   0x8000,  0x4000, &_irm[0],       1, 0, 1, -1 },
    { &_m_caos,  "CAOS",  0xe000,  0x2000, &_rom_caos[0],  2, 1, 1, -1 },
    { &_m_basic, "BASIC", 0xc000,  0x2000, &_rom_basic[0], 2, 1, 1, -1 },
    { 0, },
  };
  init_memory_groups(mem);

  reset(true);
  z80->register_ic(this);
}

Memory3::~Memory3(void)
{
  z80->unregister_ic(this);
}

byte_t
Memory3::memRead8(word_t addr)
{
  if (_m_irm->is_active() && (addr >= 0x8000) && (addr <= 0xc000))
    ui->memory_read(addr);

  return _memrptr[addr >> MemArea::PAGE_SHIFT][addr & MemArea::PAGE_MASK];
}

void
Memory3::memWrite8(word_t addr, byte_t val)
{
  if (_m_irm->is_active() && (addr >= 0x8000) && (addr <= 0xc000))
    ui->memory_write(addr);

  _memwptr[addr >> MemArea::PAGE_SHIFT][addr & MemArea::PAGE_MASK] = val;
}

byte_t *
Memory3::get_irm(void)
{
  return (byte_t *)_irm;
}

byte_t *
Memory3::get_char_rom(void)
{
  return (byte_t *)0;
}

void
Memory3::dumpCore(void)
{
#if 0
  ofstream os;
    
  os.open("core.z80");

  cout.form("Memory: dumping core...\n");
  if (!os)
    {
      cerr << "can't write 'core.z80'\n";
      exit(0);
    }
  
  os.write(&_ram[0], 0x4000);
  os.write(&_irm[0],   0x4000);

  os.close();
  cout.form("Memory: done.\n");
#endif
}

void
Memory3::enableCAOS_E(int v)
{
  _m_caos->set_active(v);
  reload_mem_ptr();
}

void
Memory3::enableBASIC_C(int v)
{
  _m_basic->set_active(v);
  reload_mem_ptr();
}

void
Memory3::enableIRM(int v)
{
  _m_irm->set_active(v);
  reload_mem_ptr();
}

void
Memory3::reset(bool power_on)
{
  _m_ram->set_active(true);
  _m_irm->set_active(true);
  _m_caos->set_active(true);
  _m_basic->set_active(true);
  reload_mem_ptr();

  if (!power_on)
    return;

  scratch_mem(&_ram[0],  0x4000);
  scratch_mem(&_irm[0],  0x4000);
}
