/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2002 Torsten Paul
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

#include <string.h>
#include <stdlib.h>

#include <fstream>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/z80.h"
#include "kc/memory0.h"

#include "ui/ui.h"

Memory0::Memory0(void) : Memory()
{
  int l;
  char *ptr;
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
    { &_m_scr,      "-",    0x0000, 0x10000, 0,             256, 0, 1, -1 },
    { &_m_ram,    "RAM",    0x0000, 0x04000, &_ram[0x0000],  10, 0, 1, KC_VARIANT_Z1013_01   },
    { &_m_ram,    "RAM",    0x0000, 0x00400, &_ram[0x0000],  10, 0, 1, KC_VARIANT_Z1013_12   },
    { &_m_ram,    "RAM",    0x0000, 0x04000, &_ram[0x0000],  10, 0, 1, KC_VARIANT_Z1013_16   },
    { &_m_ram,    "RAM",    0x0000, 0x10000, &_ram[0x0000],  10, 0, 1, KC_VARIANT_Z1013_64   },
    { &_m_ram,    "RAM",    0x0000, 0x10000, &_ram[0x0000],  10, 0, 1, KC_VARIANT_Z1013_A2   },
    { &_m_ram,    "RAM",    0x0000, 0x10000, &_ram[0x0000],  10, 0, 1, KC_VARIANT_Z1013_RB   },
    { &_m_ram,    "RAM",    0x0000, 0x0ec00, &_ram[0x0000],  10, 0, 1, KC_VARIANT_Z1013_SURL },
    { &_m_ram_f0, "RAM-F0", 0xf000, 0x00800, &_ram[0x0000],  10, 0, 1, KC_VARIANT_Z1013_SURL },
    { &_m_ram_f8, "RAM-F8", 0xf800, 0x00800, &_ram[0x0000],  10, 0, 1, KC_VARIANT_Z1013_SURL },
    { &_m_ram,    "RAM",    0x0000, 0x0ec00, &_ram[0x0000],  10, 0, 1, KC_VARIANT_Z1013_TEST },
    { &_m_ram_f0, "RAM-F0", 0xf000, 0x00800, &_ram[0x0000],  10, 0, 1, KC_VARIANT_Z1013_TEST },
    { &_m_ram_f8, "RAM-F8", 0xf800, 0x00800, &_ram[0x0000],  10, 0, 1, KC_VARIANT_Z1013_TEST },
    { &_m_irm,    "IRM",    0xec00, 0x00400, &_irm[0x0000],   0, 0, 1, -1 },
    { &_m_rom,    "ROM",    0xf000, 0x00800, &_rom[0x0000],   0, 1, 1, -1 },
    { &_m_rom_f8, "ROM",    0xf800, 0x00800, &_rom[0x0800],   0, 1, 1, KC_VARIANT_Z1013_RB },
    { &_m_rom_f8, "ROM",    0xf800, 0x00800, &_rom[0x0800],   0, 1, 1, KC_VARIANT_Z1013_TEST },
    { 0, },
  };
  
  l = strlen(kcemu_datadir);
  ptr = new char[l + 14];
  strcpy(ptr, kcemu_datadir);

  _portg = NULL;
  switch (get_kc_variant())
    {
    case KC_VARIANT_Z1013_A2:
      strcpy(ptr + l, "/z1013_a2.rom");
      loadROM(ptr, &_rom, 0x0800, 1);
      break;
    case KC_VARIANT_Z1013_RB:
      strcpy(ptr + l, "/z1013_rb.rom");
      loadROM(ptr, &_rom, 0x1000, 1);
      break;
    case KC_VARIANT_Z1013_SURL:
      strcpy(ptr + l, "/z1013_ul.rom");
      loadROM(ptr, &_rom, 0x0800, 1);
      _portg = ports->register_ports("MEMORY0", 4, 1, this, 0);
      break;
    case KC_VARIANT_Z1013_TEST:
      strcpy(ptr + l, "/z1013_z1.rom");
      loadROM(ptr, &_rom, 0x1000, 1);
      _portg = ports->register_ports("MEMORY0", 4, 1, this, 0);
      break;
    default:
      strcpy(ptr + l, "/z1013_20.rom");
      loadROM(ptr, &_rom, 0x0800, 1);
      break;
    }

  for (mptr = &m[0];mptr->name;mptr++)
    {
      *(mptr->group) = NULL;

      if ((mptr->model >= 0) && (mptr->model != get_kc_variant()))
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

Memory0::~Memory0(void)
{
  if (_portg)
    ports->unregister_ports(_portg);

  z80->unregister_ic(this);
}

#ifdef MEMORY_SLOW_ACCESS
byte_t
Memory0::memRead8(word_t addr)
{
  return _memrptr[addr >> PAGE_SHIFT][addr & PAGE_MASK];
}

void
Memory0::memWrite8(word_t addr, byte_t val)
{
  _memwptr[addr >> PAGE_SHIFT][addr & PAGE_MASK] = val;
}
#endif /* MEMORY_SLOW_ACCESS */

byte_t *
Memory0::getIRM(void)
{
  return (byte_t *)_irm;
}

void
Memory0::reset(bool power_on)
{
  if (!power_on)
    return;

  memset(&_ram[0], 0, 0x10000);
  //scratch_mem(&_ram[0], 0x10000);
}

void
Memory0::dumpCore(void)
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
  
  for (int a = 0;a < 0x10000;a++)
    os.put(memRead8(a));

  os.close();
  cout.form("Memory: done.\n");
#endif
}

byte_t
Memory0::in(word_t addr)
{
  return 0xff;
}

void
Memory0::out(word_t addr, byte_t val)
{
  bool rom = val & 0x10;
  bool ram_f0 = val & 0x04;
  bool ram_f8 = val & 0x02;

  if (_m_rom)
    _m_rom->set_active(!rom);

  if (_m_ram_f0)
    _m_ram_f0->set_readonly(!ram_f0);

  if (_m_ram_f8)
    _m_ram_f8->set_readonly(!ram_f8);

  reload_mem_ptr();
}
