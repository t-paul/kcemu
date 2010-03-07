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

#include <fstream>
#include <iostream>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/mod_romb.h"

using namespace std;

ModuleROMBank::ModuleROMBank(ModuleROMBank &tmpl) :
  ModuleInterface(tmpl.get_name(), tmpl.get_id(), tmpl.get_type())
{
  _rom = tmpl._rom;
  _group = NULL;

  _portg = ports->register_ports(get_name(), 0x78, 8, this, 0);

  _bank = 0;
  activate_bank();
  set_valid(true);
}

ModuleROMBank::ModuleROMBank(const char *filename, const char *name) :
  ModuleInterface(name, 0, KC_MODULE_KC_85_1)
{
  _group = NULL;
  _portg = NULL;
  _rom = new byte_t[ROM_BANK_SIZE];

  set_valid(Memory::load_rom(filename, _rom, ROM_BANK_SIZE, false));
}

ModuleROMBank::~ModuleROMBank(void)
{
  if (_portg)
    ports->unregister_ports(_portg);

  /*
   *  the template object owns the _rom memory and the
   *  actual module object always has an memory group
   *  registered so if we have _group == NULL this is
   *  the template object
   */
  if (_group)
    memory->unregister_memory(_group);
  else
    delete[] _rom;
}

void
ModuleROMBank::activate_bank()
{
  if (_group)
    memory->unregister_memory(_group);

  int size = (_bank & 1) ? 0x1800 : 0x2800;
  int offset = (_bank / 2) * 0x4000 + (_bank & 1) * 0x2800;

  //printf("activate bank %2d : size = %04xh, offset = %05xh\n", _bank, size, offset);

  _group = memory->register_memory(get_name(), 0xc000, size, &_rom[offset], 10, 0);
}

void
ModuleROMBank::m_out(word_t addr, byte_t val)
{
  //printf("m_out %04x, %02x\n", addr, val);
}

ModuleInterface *
ModuleROMBank::clone(void)
{
  return new ModuleROMBank(*this);
}

void
ModuleROMBank::reset(bool power_on)
{
  _bank = 0;
  activate_bank();
}

byte_t
ModuleROMBank::in(word_t addr)
{
  return 0xff;
}

void
ModuleROMBank::out(word_t addr, byte_t val)
{
  _bank = (_bank + 1) & 0x0f;
  activate_bank();
}
