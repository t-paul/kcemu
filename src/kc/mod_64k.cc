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
#include "kc/mod_64k.h"

Module64k::Module64k(Module64k &tmpl) :
  ModuleInterface(tmpl.get_name(), tmpl.get_id(), tmpl.get_type())
{
  _val = 0;
  _group[0] = NULL;
  _group[1] = NULL;
  _group[2] = NULL;
  _group[3] = NULL;
  _ram = new byte_t[RAM_SIZE];
  if (_ram)
    {
      memset(_ram, 0, RAM_SIZE);
      set_valid(true);
    }
}

Module64k::Module64k(const char *name, byte_t id) :
  ModuleInterface(name, id, KC_MODULE_KC_85_3)
{
  int a;
  
  _val = 0;
  _ram = new byte_t[RAM_SIZE];
  if (_ram)
    {
      for (a = 0;a < SEGMENTS;a++)
        _group[a] = NULL;
      memset(_ram, 0, RAM_SIZE);
      set_valid(true);
    }
}

Module64k::~Module64k(void)
{
  int a;
  
  for (a = 0;a < SEGMENTS;a++)
    if (_group[a])
      memory->unregister_memory(_group[a]);
  delete[] _ram;
}

void
Module64k::m_out(word_t addr, byte_t val)
{
  int a;
  word_t mem;
  char buf[100];
  
  if (((_val & 0xc3) ^ (val & 0xc3)) == 0) return;
  for (a = 0;a < SEGMENTS;a++)
    if (_group[a])
      {
        memory->unregister_memory(_group[a]);
        _group[a] = 0;
      }
  if (val & 3)
    {
      mem = (val & 0xc0) << 8;
      for (a = 0;a < SEGMENTS;a++)
        {
          sprintf(buf, "%s (%d)", get_name(), a);
          _group[a] = memory->register_memory(buf, mem,
                                              SEGMENT_SIZE,
                                              &_ram[a * SEGMENT_SIZE],
                                              (addr >> 8),
                                              (val & 2) == 0);
          mem = (mem + SEGMENT_SIZE) & (RAM_SIZE - 1);
        }
    }
  _val = val;
}

ModuleInterface *
Module64k::clone(void)
{
  return new Module64k(*this);
}

void
Module64k::reset(bool power_on)
{
  if (power_on)
    Memory::scratch_mem(_ram, RAM_SIZE);
}
