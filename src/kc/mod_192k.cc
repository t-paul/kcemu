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

#include <string.h>
#include <fstream>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/mod_192k.h"

using namespace std;

/**
 *  192 KByte RAM/EPROM Modul
 *
 *  http://vpohlers.tripod.com/192k_modul.htm
 */
Module192k::Module192k(Module192k &tmpl) :
  ModuleInterface(tmpl.get_name(), tmpl.get_id(), tmpl.get_type())
{
  init();

  _master = false;

  if (tmpl._rom_D2 == NULL)
    return;

  _rom_D2 = tmpl._rom_D2;
  _rom_D3 = tmpl._rom_D3;
  _rom_D5 = tmpl._rom_D5;

  _ram_D1 = new byte_t[0x8000];
  _ram_D4 = new byte_t[0x8000];
  _ram_D6 = new byte_t[0x4000];

  memset(_ram_D1, 0, 0x8000);
  memset(_ram_D4, 0, 0x8000);
  memset(_ram_D6, 0, 0x4000);

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
    { &_m_4000,   "RAM D1", 0x4000, 0x2000, &_ram_D1[0x0000], 10, 0, 1 },
    { &_m_6000,   "RAM D1", 0x6000, 0x2000, &_ram_D1[0x2000], 10, 0, 1 },
    { &_m_8000,   "RAM D1", 0x8000, 0x2000, &_ram_D1[0x4000], 10, 0, 1 },
    { &_m_a000,   "RAM D1", 0xa000, 0x2000, &_ram_D1[0x6000], 10, 0, 1 },

    { &_m_c000[ 0], "ROMC D2 (Bank  0)", 0xc000, 0x2000, &_rom_D2[0x0000], 10, 1, 1 },
    { &_m_c000[ 1], "ROMC D2 (Bank  1)", 0xc000, 0x2000, &_rom_D2[0x2000], 10, 1, 0 },
    { &_m_c000[ 2], "ROMC D2 (Bank  2)", 0xc000, 0x2000, &_rom_D2[0x4000], 10, 1, 0 },
    { &_m_c000[ 3], "ROMC D2 (Bank  3)", 0xc000, 0x2000, &_rom_D2[0x6000], 10, 1, 0 },
    { &_m_c000[ 4], "ROMC D2 (Bank  4)", 0xc000, 0x2000, &_rom_D2[0x8000], 10, 1, 0 },
    { &_m_c000[ 5], "ROMC D2 (Bank  5)", 0xc000, 0x2000, &_rom_D2[0xa000], 10, 1, 0 },
    { &_m_c000[ 6], "ROMC D2 (Bank  6)", 0xc000, 0x2000, &_rom_D2[0xc000], 10, 1, 0 },
    { &_m_c000[ 7], "ROMC D2 (Bank  7)", 0xc000, 0x2000, &_rom_D2[0xe000], 10, 1, 0 },

    { &_m_c000[ 8], "ROMC D3 (Bank  8)", 0xc000, 0x2000, &_rom_D3[0x0000], 10, 1, 0 },
    { &_m_c000[ 9], "ROMC D3 (Bank  9)", 0xc000, 0x2000, &_rom_D3[0x2000], 10, 1, 0 },
    { &_m_c000[10], "ROMC D3 (Bank 10)", 0xc000, 0x2000, &_rom_D3[0x4000], 10, 1, 0 },
    { &_m_c000[11], "ROMC D3 (Bank 11)", 0xc000, 0x2000, &_rom_D3[0x6000], 10, 1, 0 },

    { &_m_c000[12], "RAMC D4 (Bank 12)", 0xc000, 0x2000, &_ram_D4[0x0000], 10, 0, 0 },
    { &_m_c000[13], "RAMC D4 (Bank 13)", 0xc000, 0x2000, &_ram_D4[0x2000], 10, 0, 0 },
    { &_m_c000[14], "RAMC D4 (Bank 14)", 0xc000, 0x2000, &_ram_D4[0x4000], 10, 0, 0 },
    { &_m_c000[15], "RAMC D4 (Bank 15)", 0xc000, 0x2000, &_ram_D4[0x6000], 10, 0, 0 },

    { &_m_e000[ 0], "ROME D5 (Bank  0)", 0xe000, 0x0800, &_rom_D5[0x0000], 10, 1, 1 },
    { &_m_e000[ 1], "ROME D5 (Bank  1)", 0xe000, 0x0800, &_rom_D5[0x0800], 10, 1, 0 },
    { &_m_e000[ 2], "ROME D5 (Bank  2)", 0xe000, 0x0800, &_rom_D5[0x1000], 10, 1, 0 },
    { &_m_e000[ 3], "ROME D5 (Bank  3)", 0xe000, 0x0800, &_rom_D5[0x1800], 10, 1, 0 },
    { &_m_e000[ 4], "ROME D5 (Bank  4)", 0xe000, 0x0800, &_rom_D5[0x2000], 10, 1, 0 },
    { &_m_e000[ 5], "ROME D5 (Bank  5)", 0xe000, 0x0800, &_rom_D5[0x2800], 10, 1, 0 },
    { &_m_e000[ 6], "ROME D5 (Bank  6)", 0xe000, 0x0800, &_rom_D5[0x3000], 10, 1, 0 },
    { &_m_e000[ 7], "ROME D5 (Bank  7)", 0xe000, 0x0800, &_rom_D5[0x3800], 10, 1, 0 },

    { &_m_e000[ 8], "RAME D5 (Bank  8)", 0xe000, 0x0800, &_ram_D6[0x0000], 10, 0, 0 },
    { &_m_e000[ 9], "RAME D5 (Bank  9)", 0xe000, 0x0800, &_ram_D6[0x0800], 10, 0, 0 },
    { &_m_e000[10], "RAME D5 (Bank 10)", 0xe000, 0x0800, &_ram_D6[0x1000], 10, 0, 0 },
    { &_m_e000[11], "RAME D5 (Bank 11)", 0xe000, 0x0800, &_ram_D6[0x1800], 10, 0, 0 },
    { &_m_e000[12], "RAME D5 (Bank 12)", 0xe000, 0x0800, &_ram_D6[0x2000], 10, 0, 0 },
    { &_m_e000[13], "RAME D5 (Bank 13)", 0xe000, 0x0800, &_ram_D6[0x2800], 10, 0, 0 },
    { &_m_e000[14], "RAME D5 (Bank 14)", 0xe000, 0x0800, &_ram_D6[0x3000], 10, 0, 0 },
    { &_m_e000[15], "RAME D5 (Bank 15)", 0xe000, 0x0800, &_ram_D6[0x3800], 10, 0, 0 },

    { 0, },
  };

  for (mptr = &m[0];mptr->name;mptr++)
    {
      *(mptr->group) = memory->register_memory(mptr->name,
					       mptr->addr,
					       mptr->size,
					       mptr->mem,
					       mptr->prio,
					       mptr->ro);
      (*(mptr->group))->set_active(mptr->active);
    }

  _portg = ports->register_ports(get_name(), 0xff, 1, this, 0);

  set_valid(true);
}

Module192k::Module192k(const char *d2, const char *d3, const char *d5, const char *name) :
  ModuleInterface(name, 0, KC_MODULE_KC_85_1)
{
  init();

  _master = true;

  _rom_D2 = new byte_t[65536];
  _rom_D3 = new byte_t[32768];
  _rom_D5 = new byte_t[16384];

  memset(_rom_D2, 0, 65536);
  memset(_rom_D3, 0, 32768);
  memset(_rom_D5, 0, 16384);

  bool valid = true;
  if (!Memory::load_rom(d2, _rom_D2, 65536, false))
    valid = false;
  if (!Memory::load_rom(d3, _rom_D3, 32768, false))
    valid = false;
  if (!Memory::load_rom(d5, _rom_D5, 8192, false))
    valid = false;
  if (!Memory::load_rom(d5, _rom_D5 + 8192, 8192, false))
    valid = false;

  if (!valid)
    {
      delete[] _rom_D2;
      delete[] _rom_D3;
      delete[] _rom_D5;

      _rom_D2 = NULL;
      _rom_D3 = NULL;
      _rom_D5 = NULL;
    }

  set_valid(valid);
}

Module192k::~Module192k(void)
{
  if (_portg)
    ports->unregister_ports(_portg);

  if (_master)
    {
      if (_rom_D2)
	delete[] _rom_D2;

      if (_rom_D3)
	delete[] _rom_D3;

      if (_rom_D5)
	delete[] _rom_D5;
    }

  if (_m_4000)
    memory->unregister_memory(_m_4000);
  if (_m_6000)
    memory->unregister_memory(_m_6000);
  if (_m_8000)
    memory->unregister_memory(_m_8000);
  if (_m_a000)
    memory->unregister_memory(_m_a000);

  for (int a = 0;a < 16;a++)
    {
      if (_m_c000[a])
	memory->unregister_memory(_m_c000[a]);
      if (_m_e000[a])
	memory->unregister_memory(_m_e000[a]);
    }

}

void
Module192k::init(void)
{
  _val = 0;
  _portg = NULL;
  _ram_D1 = NULL;
  _ram_D4 = NULL;
  _ram_D6 = NULL;
  _rom_D2 = NULL;
  _rom_D3 = NULL;
  _rom_D5 = NULL;

  _m_4000 = NULL;
  _m_6000 = NULL;
  _m_8000 = NULL;
  _m_a000 = NULL;
  for (int a = 0;a < 16;a++)
    _m_c000[a] = _m_e000[a] = NULL;
}

void
Module192k::m_out(word_t addr, byte_t val)
{
}

ModuleInterface *
Module192k::clone(void)
{
  return new Module192k(*this);
}

byte_t
Module192k::in(word_t addr)
{
  return 0xff;
}

void
Module192k::out(word_t addr, byte_t val)
{
  _m_c000[(_val >> 4) & 0x0f]->set_active(false);
  _m_c000[(val >> 4) & 0x0f]->set_active(true);
  _m_e000[_val & 0x0f]->set_active(false);
  _m_e000[val & 0x0f]->set_active(true);
  _val = val;
  memory->reload_mem_ptr();
}

void
Module192k::reset(bool power_on)
{
  /*
   *  initialize memory bank switch after reset / power on
   */
  out(0xff, 0);
}
