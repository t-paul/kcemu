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

#include <stdio.h>
#include <string.h>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/mod_segm.h"

ModuleSegmentedMemory::ModuleSegmentedMemory(ModuleSegmentedMemory &tmpl) :
  ModuleInterface(tmpl.get_name(), tmpl.get_id(), tmpl.get_type())
{
  _val = 0;
  _group = NULL;
  _master = false;
  _is_rom = tmpl._is_rom;
  _segments = tmpl.get_segment_count();
  _segment_size = tmpl.get_segment_size();

  if (is_rom())
    {
      _mem = tmpl._mem;
      set_valid(true);
    }
  else
    {
      int mem_size = get_segment_count() * get_segment_size();
      _mem = new byte_t[mem_size];
      if (_mem)
        {
          reset(true);
          set_valid(true);
        }
    }
}

ModuleSegmentedMemory::ModuleSegmentedMemory(const char *name, byte_t id, int segments, int segment_size) :
  ModuleInterface(name, id, KC_MODULE_KC_85_3)
{
  _val = 0;
  _mem = NULL;
  _group = NULL;
  _master = true;
  _is_rom = false;
  _segments = segments;
  _segment_size = segment_size;

  set_valid(true);
}

ModuleSegmentedMemory::ModuleSegmentedMemory(const char *name, byte_t id, int segments, int segment_size, const char *filename) :
  ModuleInterface(name, id, KC_MODULE_KC_85_3)
{
  int mem_size = segments * segment_size;

  _val = 0;
  _group = NULL;
  _master = true;
  _is_rom = true;
  _segments = segments;
  _segment_size = segment_size;
  _mem = new byte_t[mem_size];

  set_valid(Memory::load_rom(filename, _mem, mem_size, false));
}

ModuleSegmentedMemory::~ModuleSegmentedMemory(void)
{
  if (_group)
    memory->unregister_memory(_group);
  if (_master && _mem)
    delete[] _mem;
}

bool
ModuleSegmentedMemory::is_rom(void)
{
  return _is_rom;
}

int
ModuleSegmentedMemory::get_segment_count(void)
{
  return _segments;
}

int
ModuleSegmentedMemory::get_segment_size(void)
{
  return _segment_size;
}

void
ModuleSegmentedMemory::m_out(word_t addr, byte_t val)
{
  if (_val == val)
    return;

  _val = val;

  if (_group)
    {
      memory->unregister_memory(_group);
      _group = NULL;
    }

  if (_val & 1)
    {
      char buf[100];

      int idx = get_segment_index(addr, val);
      word_t mem = get_base_address(addr, val);
      bool ro = is_rom() ? true : (_val & 2) == 0;

      snprintf(buf, sizeof(buf), "%s (%d)", get_name(), idx);
      _group = memory->register_memory(buf,
				       mem,
				       get_segment_size(),
				       &_mem[idx * get_segment_size()],
				       0x10,
				       ro);
    }
}

void
ModuleSegmentedMemory::reset(bool power_on)
{
  if (is_rom())
    return;

  if (power_on)
    Memory::scratch_mem(_mem, get_segment_count() * get_segment_size());
}
