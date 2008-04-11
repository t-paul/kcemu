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

#include <stdlib.h>
#include <fstream>

#include "kc/system.h"
#include "kc/prefs/prefs.h"

#include "kc/z80.h"
#include "kc/memory6.h"

using namespace std;

Memory6::Memory6(void) : Memory()
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
    { &_m_scr,      "-",     0x0000, 0x10000, 0,               256, 0, 1, -1 },
    { &_m_rom0,    "ROM0", 0x0000,  0x0400, &_rom0[0x0000],      0, 1, 1, KC_VARIANT_POLY880 },
    { &_m_rom_sc1, "ROM",  0x0000,  0x1000, &_rom_sc1[0x0000],   0, 1, 1, KC_VARIANT_POLY880_SC1 },
    { &_m_rom1,    "ROM1", 0x1000,  0x0400, &_rom1[0x0000],      0, 1, 1, KC_VARIANT_POLY880 },
    { &_m_rom2,    "ROM2", 0x2000,  0x0400, &_rom2[0x0000],      0, 1, 1, -1 },
    { &_m_rom3,    "ROM3", 0x3000,  0x0400, &_rom3[0x0000],      0, 1, 1, -1 },
    { &_m_ram,     "RAM",  0x4000,  0x0400, &_ram[0x0000],       0, 0, 1, -1 },
    { 0, },
  };

  string datadir(kcemu_datadir);
  string poly880_romdir = datadir + "/roms/poly880";
  string poly880_sc1_rom = poly880_romdir + "/sc1.rom";
  string poly880_system_rom_a = poly880_romdir + "/poly880a.rom";
  string poly880_system_rom_b = poly880_romdir + "/poly880b.rom";
  string poly880_system_rom_c = poly880_romdir + "/poly880c.rom";
  string poly880_system_rom_d = poly880_romdir + "/poly880d.rom";

  load_rom(poly880_sc1_rom.c_str(), &_rom_sc1, 0x1000, true);
  load_rom(poly880_system_rom_a.c_str(), &_rom0, 0x0400, true);
  load_rom(poly880_system_rom_b.c_str(), &_rom1, 0x0400, true);

  if (!load_rom(poly880_system_rom_c.c_str(), &_rom2, 0x0400, false))
    memset(_rom2, 0, 0x0400);

  if (!load_rom(poly880_system_rom_d.c_str(), &_rom3, 0x0400, false))
    memset(_rom3, 0, 0x0400);

  /*
   *  The content of the poly880 rom is stored inverted. We use
   *  the original rom and restore the machine readable code here.
   */
  for (int a = 0;a < 0x0400;a++)
    {
      _rom0[a] = ~_rom0[a];
      _rom1[a] = ~_rom1[a];
      _rom2[a] = ~_rom2[a];
      _rom3[a] = ~_rom3[a];
    }

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

Memory6::~Memory6(void)
{
  z80->unregister_ic(this);
}

byte_t *
Memory6::get_irm(void)
{
  return (byte_t *)0;
}

byte_t *
Memory6::get_char_rom(void)
{
  return (byte_t *)0;
}

void
Memory6::reset(bool power_on)
{
  if (!power_on)
    return;

  memset(&_ram[0], 0, 0x0400);
}

void
Memory6::dumpCore(void)
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
