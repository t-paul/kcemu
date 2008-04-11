/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: memory8.cc,v 1.2 2002/06/09 14:24:33 torsten_paul Exp $
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

#include "kc/z80.h"
#include "kc/memory8.h"

#include "ui/ui.h"

using namespace std;

Memory8::Memory8(void) : Memory()
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
    int            model;
  } *mptr, m[] = {
    { &_m_scr,   "-",     0x0000, 0x10000, 0,         256, 0, 1, -1                 },
    { &_m_rom1,  "ROM1",  0x0000,  0x0400, &_rom1[0],   0, 1, 1, KC_VARIANT_LC80_1k },
    { &_m_rom2,  "ROM2",  0x0800,  0x0400, &_rom2[0],   0, 1, 1, KC_VARIANT_LC80_1k },
    { &_m_rom,   "ROM",   0x0000,  0x0800, &_rom[0],    0, 1, 1, KC_VARIANT_LC80_2k },
    { &_m_ram,   "RAM",   0x2000,  0x0400, &_ram[0],    0, 0, 1, -1                 },
    { 0, },
  };

  string datadir(kcemu_datadir);
  string lc80_romdir = datadir + "/roms/lc80";
  string lc80_system_00_rom = lc80_romdir + "/lc80__00.rom";
  string lc80_system_08_rom = lc80_romdir + "/lc80__08.rom";
  string lc80_system_2k_rom = lc80_romdir + "/lc80__2k.rom";

  load_rom(lc80_system_00_rom.c_str(), &_rom1, 0x0400, true);
  load_rom(lc80_system_08_rom.c_str(), &_rom2, 0x0400, true);
  load_rom(lc80_system_2k_rom.c_str(), &_rom,  0x0800, true);

  for (mptr = &m[0];mptr->name;mptr++)
    {
      *(mptr->group) = NULL;

      if ((mptr->model >= 0) && (mptr->model != Preferences::instance()->get_kc_variant()))
	continue;

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

Memory8::~Memory8(void)
{
  z80->unregister_ic(this);
}

byte_t
Memory8::memRead8(word_t addr)
{
  return _memrptr[addr >> MemArea::PAGE_SHIFT][addr & MemArea::PAGE_MASK];
}

void
Memory8::memWrite8(word_t addr, byte_t val)
{
  _memwptr[addr >> MemArea::PAGE_SHIFT][addr & MemArea::PAGE_MASK] = val;
}

byte_t *
Memory8::get_irm(void)
{
  return (byte_t *)0;
}

byte_t *
Memory8::get_char_rom(void)
{
  return (byte_t *)0;
}

void
Memory8::reset(bool power_on)
{
  if (!power_on)
    return;

  scratch_mem(&_ram[0], 0x0400);
}

void
Memory8::dumpCore(void)
{
}
