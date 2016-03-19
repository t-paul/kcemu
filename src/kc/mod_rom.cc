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
#include <iostream>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/mod_rom.h"

#include "libdbg/dbg.h"

using namespace std;

ModuleROM::ModuleROM(ModuleROM &tmpl) :
  ModuleInterface(tmpl.get_name(), tmpl.get_id(), tmpl.get_type())
{
  _val = 0;
  _size = tmpl._size;
  _group = NULL;
  _rom = new byte_t[_size];
  if (_rom)
    {
      memcpy(_rom, tmpl._rom, _size);
      set_valid(true);
    }
}

ModuleROM::ModuleROM(const char *rom_key, const char *name, byte_t id) :
  ModuleInterface(name, id, KC_MODULE_KC_85_3)
{
  _size = 0x2000;
  _rom = new byte_t[_size]; // FIXME: need to get size from type info
  _group = NULL;

  set_valid(Memory::load_rom(rom_key, _rom));
}

ModuleROM::ModuleROM(const char *filename, const char *name, dword_t size, byte_t id) :
  ModuleInterface(name, id, KC_MODULE_KC_85_3)
{
  _rom = new byte_t[size];
  _size = size;
  _group = NULL;

  set_valid(Memory::load_rom(filename, _rom, size, false));
}

ModuleROM::ModuleROM(byte_t *rom, const char *name, dword_t size, byte_t id) :
  ModuleInterface(name, id, KC_MODULE_KC_85_3)
{
  _id = id;
  _size = size;
  _rom = new byte_t[size];
  _group = NULL;

  memcpy(_rom, rom, size);
  set_valid(true);
}

ModuleROM::~ModuleROM(void)
{
  if (_group)
    memory->unregister_memory(_group);
  delete[] _rom;
}

word_t
ModuleROM::get_addr(byte_t val)
{
  return (val & 0xc0) << 8;
}

dword_t
ModuleROM::get_size(byte_t val)
{
  return _size;
}

byte_t *
ModuleROM::get_rom_ptr(byte_t val)
{
  return _rom;
}

void
ModuleROM::m_out(word_t addr, byte_t val)
{
  if (_val == val)
    return;

  byte_t *rom = get_rom_ptr(val);

  if (_group)
    {
      memory->unregister_memory(_group);
      _group = 0;
    }

  if ((val & 1) && (rom != NULL))
    {
      _group = memory->register_memory(get_name(),
				       get_addr(val),
				       get_size(val),
				       rom,
				       (addr >> 8),
				       true);
    }

  _val = val;
}

ModuleInterface *
ModuleROM::clone(void)
{
  return new ModuleROM(*this);
}

void
ModuleROM::reset(bool power_on)
{
}
