/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2006 Torsten Paul
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

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/mod_auto.h"

#include "libdbg/dbg.h"

using namespace std;

ModuleAutoStart::ModuleAutoStart(ModuleAutoStart &tmpl) :
  ModuleROM(tmpl)
{
}

ModuleAutoStart::ModuleAutoStart(byte_t *rom, const char *name, dword_t size, byte_t id) :
  ModuleROM(rom, name, size, id)
{
}

ModuleAutoStart::~ModuleAutoStart(void)
{
}

word_t
ModuleAutoStart::get_addr(byte_t val)
{
  return (val & 0xe0) << 8;
}

dword_t
ModuleAutoStart::get_size(byte_t val)
{
  return PAGE_SIZE;
}

byte_t *
ModuleAutoStart::get_rom_ptr(byte_t val)
{
  int page = (val >> 2) & 0x07;

  DBG(2, form("KCemu/ModuleAutoStart/page",
	      "ModuleAutoStart: val = %02xh, page = %d, size = %04xh (PAGE_SIZE = %04xh)\n",
	      val, page, _size, PAGE_SIZE));

  if ((page * PAGE_SIZE) > _size)
    return NULL;
 
  return _rom + (page * PAGE_SIZE);
}

ModuleInterface *
ModuleAutoStart::clone(void)
{
  return new ModuleAutoStart(*this);
}

