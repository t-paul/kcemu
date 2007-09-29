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

#include "kc/kc.h"
#include "kc/z80.h"
#include "kc/c80/memory.h"

using namespace std;

MemoryC80::MemoryC80(void) : Memory()
{
  struct {
    MemAreaGroup **group;
    const char    *name;
    word_t         addr;
    dword_t        size;
    byte_t        *mem;
    int            prio;
    bool           ro;
    bool           active;
  } *mptr, m[] = {
    { &_m_scr,         "-",            0x0000, 0x10000, 0,                256, 0, 1 },
    { &_m_rom_monitor, "Monitor",      0x0000,  0x0400, &_rom_monitor[0],   1, 1, 1 },
    { &_m_rom_user,    "User ROM",     0x0400,  0x0400, &_rom_user[0],      1, 1, 1 },
    { &_m_ram_mirror,  "RAM (mirror)", 0x0800,  0x0400, &_ram[0],           1, 0, 1 },
    { &_m_ram,         "RAM",          0x0c00,  0x0400, &_ram[0],           1, 0, 1 },
    { 0, },
  };

  string datadir(kcemu_datadir);
  string c80_romdir = datadir + "/roms/c80";
  string c80_rom_monitor = c80_romdir + "/monitor.rom";
  string c80_rom_user = c80_romdir + "/user.rom";

  load_rom(c80_rom_monitor.c_str(), &_rom_monitor, 0x0400, true);

  if (!load_rom(c80_rom_user.c_str(), &_rom_user, 0x0400, false))
    memset(_rom_user, 0xff, 0x400);

  for (mptr = &m[0];mptr->name;mptr++)
    {
      *(mptr->group) = new MemAreaGroup(mptr->name,
					mptr->addr,
					mptr->size,
					mptr->mem,
					mptr->prio,
					mptr->ro);
      (*(mptr->group))->add(get_mem_ptr());
      if (mptr->active)
	(*(mptr->group))->set_active(true);
    }

  reload_mem_ptr();

  reset(true);
  z80->register_ic(this);
}

MemoryC80::~MemoryC80(void)
{
  z80->unregister_ic(this);

  delete _m_scr;
  delete _m_rom_monitor;
  delete _m_rom_user;
  delete _m_ram_mirror;
  delete _m_ram;
}

byte_t
MemoryC80::memRead8(word_t addr)
{
  return _memrptr[addr >> MemArea::PAGE_SHIFT][addr & MemArea::PAGE_MASK];
}

void
MemoryC80::memWrite8(word_t addr, byte_t val)
{
  _memwptr[addr >> MemArea::PAGE_SHIFT][addr & MemArea::PAGE_MASK] = val;
}

byte_t *
MemoryC80::get_irm(void)
{
  return 0;
}

byte_t *
MemoryC80::get_char_rom(void)
{
  return 0;
}

void
MemoryC80::reset(bool power_on)
{
  if (!power_on)
    return;

  scratch_mem(&_ram[0], 0x0400);
}

void
MemoryC80::dumpCore(void)
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
