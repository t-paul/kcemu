/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-1998 Torsten Paul
 *
 *  $Id: mod_ram.cc,v 1.3 2000/05/21 16:54:24 tp Exp $
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

#include "kc/config.h"
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
  if (_group) memory->unregister_memory(_group);
  delete _ram;
}

void
ModuleRAM::out(word_t addr, byte_t val)
{
  word_t a;
  
  if (((_val & 3) ^ (val & 3)) == 0) return;
  a = (val & 0xc0) << 8;
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
