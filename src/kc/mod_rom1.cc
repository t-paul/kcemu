/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: mod_rom1.cc,v 1.1 2002/10/31 01:46:35 torsten_paul Exp $
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
#include <fstream>
#include <iostream>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/mod_rom1.h"

using namespace std;

ModuleROM1::ModuleROM1(ModuleROM1 &tmpl) :
  ModuleInterface(tmpl.get_name(), tmpl.get_id(), tmpl.get_type())
{
  _group = NULL;
  _addr = tmpl._addr;
  _size = tmpl._size;
  _rom = new byte_t[_size];
  if (_rom)
    {
      memcpy(_rom, tmpl._rom, _size);
      set_valid(true);
      _group = memory->register_memory(get_name(), _addr, _size, _rom, 0, 0);
    }
}

ModuleROM1::ModuleROM1(const char *filename, const char *name, word_t addr, dword_t size) :
  ModuleInterface(name, 0, KC_MODULE_KC_85_1)
{
  int c;
  ifstream is;
  unsigned int a;

  _group = NULL;
  _addr = addr;
  _size = size;
  _rom = new byte_t[_size];

  is.open(filename);
  if (!is)
    return;
  
  for (a = 0;a < size;a++)
    {
      c = is.get();
      if (c == EOF)
	return;
      
      _rom[a] = c;
    }
  set_valid(true);
}

ModuleROM1::~ModuleROM1(void)
{
  if (_group)
    memory->unregister_memory(_group);

  delete[] _rom;
}

void
ModuleROM1::out(word_t addr, byte_t val)
{
}

ModuleInterface *
ModuleROM1::clone(void)
{
  return new ModuleROM1(*this);
}
