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

#include <string.h>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/mod_segm.h"

ModuleSegmentedRAM::ModuleSegmentedRAM(ModuleSegmentedRAM &tmpl) :
  ModuleInterface(tmpl.get_name(), tmpl.get_id(), tmpl.get_type())
{
  _val = 0;
  _group = NULL;
  _segments = tmpl.get_segment_count();
  _segment_size = tmpl.get_segment_size();

  int ram_size = get_segment_count() * get_segment_size();

  _ram = new byte_t[ram_size];
  if (_ram)
    {
      reset(true);
      set_valid(true);
    }
}

ModuleSegmentedRAM::ModuleSegmentedRAM(const char *name, byte_t id, int segments, int segment_size) :
  ModuleInterface(name, id, KC_MODULE_KC_85_3)
{
  _val = 0;
  _ram = NULL;
  _group = NULL;
  _segments = segments;
  _segment_size = segment_size;
  set_valid(true);
}

ModuleSegmentedRAM::~ModuleSegmentedRAM(void)
{
  if (_group)
    memory->unregister_memory(_group);
  if (_ram)
    delete[] _ram;
}

int
ModuleSegmentedRAM::get_segment_count(void)
{
  return _segments;
}

int
ModuleSegmentedRAM::get_segment_size(void)
{
  return _segment_size;
}

void
ModuleSegmentedRAM::m_out(word_t addr, byte_t val)
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

      sprintf(buf, "%s (%d)", get_name(), idx);
      _group = memory->register_memory(buf,
				       mem,
				       get_segment_size(),
				       &_ram[idx * get_segment_size()],
				       0x10,
				       (_val & 2) == 0);
    }
}

void
ModuleSegmentedRAM::reset(bool power_on)
{
  if (power_on)
    Memory::scratch_mem(_ram, get_segment_count() * get_segment_size());
}
