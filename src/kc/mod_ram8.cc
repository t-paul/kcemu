/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: mod_ram8.cc,v 1.3 2002/10/31 01:46:35 torsten_paul Exp $
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
#include "kc/mod_ram8.h"

ModuleRAM8::ModuleRAM8(ModuleRAM8 &tmpl) :
  ModuleInterface(tmpl.get_name(), tmpl.get_id(), tmpl.get_type())
{
  _group = NULL;
  _size = tmpl._size;
  _ram = new byte_t[_size];
  _addr = tmpl._addr;
  if (_ram)
    {
      memset(_ram, 0, _size);
      set_valid(true);
    }
  _group = memory->register_memory(get_name(), _addr, _size, _ram, 0, 0);
}

ModuleRAM8::ModuleRAM8(const char *name, word_t addr, dword_t size) :
  ModuleInterface(name, 0, KC_MODULE_LC_80)
{
  _size = size;
  _ram = new byte_t[_size];
  _addr = addr;
  if (_ram)
    {
      _group = 0;
      memset(_ram, 0, _size);
      set_valid(true);
    }
}

ModuleRAM8::~ModuleRAM8(void)
{
  if (_group) memory->unregister_memory(_group);
  delete[] _ram;
}

void
ModuleRAM8::out(word_t addr, byte_t val)
{
}

ModuleInterface *
ModuleRAM8::clone(void)
{
  return new ModuleRAM8(*this);
}
