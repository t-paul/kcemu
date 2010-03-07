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

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/mod_ram.h"

ModuleRAM::ModuleRAM(ModuleRAM &tmpl) :
  ModuleInterface(tmpl.get_name(), tmpl.get_id(), tmpl.get_type())
{
  _val = 0;
  _group = NULL;
  _ram = new byte_t[RAM_SIZE];
  if (_ram)
    {
      memset(_ram, 0, RAM_SIZE);
      set_valid(true);
    }
}

ModuleRAM::ModuleRAM(const char *name, byte_t id) :
  ModuleInterface(name, id, KC_MODULE_KC_85_3)
{
  _ram = new byte_t[RAM_SIZE];
  if (_ram)
    {
      _group = 0;
      memset(_ram, 0, RAM_SIZE);
      set_valid(true);
    }
}

ModuleRAM::~ModuleRAM(void)
{
  if (_group)
    memory->unregister_memory(_group);
  delete[] _ram;
}

void
ModuleRAM::m_out(word_t addr, byte_t val)
{
  if (((_val & 0xc3) ^ (val & 0xc3)) == 0)
    return;

  word_t a = (val & 0xc0) << 8;

  if (_group)
    {
      memory->unregister_memory(_group);
      _group = 0;
    }
  if (val & 3)
      _group = memory->register_memory(get_name(), a, RAM_SIZE,
                                       _ram, (addr >> 8),
                                       (val & 2) == 0);

  _val = val;
}

ModuleInterface *
ModuleRAM::clone(void)
{
  return new ModuleRAM(*this);
}

void
ModuleRAM::reset(bool power_on)
{
  if (power_on)
    Memory::scratch_mem(_ram, RAM_SIZE);
}
