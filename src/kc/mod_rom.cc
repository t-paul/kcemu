/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: mod_rom.cc,v 1.6 2001/04/14 15:16:19 tp Exp $
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
#include <fstream.h>
#include <iostream.h>

#include "kc/config.h"
#include "kc/system.h"

#include "kc/kc.h"
#include "kc/mod_rom.h"

#include "libdbg/dbg.h"

ModuleROM::ModuleROM(ModuleROM &tmpl) :
  ModuleInterface(tmpl.get_name(), tmpl.get_id(), tmpl.get_type())
{
  _val = 0;
  _size = tmpl._size;
  _addr = tmpl._addr;
  _group = NULL;
  _rom = new byte_t[_size];
  if (_rom)
    {
      memcpy(_rom, tmpl._rom, _size);
      set_valid(true);
    }
}

ModuleROM::ModuleROM(const char *filename, const char *name,
                     dword_t size, byte_t id, word_t addr) :
  ModuleInterface(name, id, KC_MODULE_KC_85_3)
{
  int a, c;
  ifstream is;

  _rom = new byte_t[size];
  _addr = addr;
  _size = size;
  _group = NULL;

  a = 0;
  is.open(filename);
  if (!is) return;
  for (a = 0;a < size;a++)
    {
      c = is.get();
      if (c == EOF) return;
      _rom[a] = c;
    }
  _ok = true;
  set_valid(true);
}

ModuleROM::~ModuleROM(void)
{
  if (_group) memory->unregister_memory(_group);
  delete _rom;
}

void
ModuleROM::out(word_t addr, byte_t val)
{
  if (((_val & 1) ^ (val & 1)) != 1) return;
  if (val & 1)
    _group = memory->register_memory(get_name(), _addr, _size,
                                     _rom, (addr >> 8), true);
  else
    {
      if (_group) memory->unregister_memory(_group);
      _group = 0;
    }

  _val = val;
}

ModuleInterface *
ModuleROM::clone(void)
{
  return new ModuleROM(*this);
}
