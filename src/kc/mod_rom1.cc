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
#include "kc/prefs/prefs.h"

#include "kc/memory1.h"
#include "kc/memory7.h"
#include "kc/mod_rom1.h"

using namespace std;

ModuleROM1::ModuleROM1(ModuleROM1 &tmpl) :
  ModuleInterface(tmpl.get_name(), tmpl.get_id(), tmpl.get_type())
{
  _group = NULL;
  _addr = tmpl._addr;
  _size = tmpl._size;
  _set_romdi = tmpl._set_romdi;
  _rom = new byte_t[_size];
  if (_rom)
    {
      memcpy(_rom, tmpl._rom, _size);
      set_valid(true);
      _group = memory->register_memory(get_name(), _addr, _size, _rom, 0, true);

      /*
       * FIXME: make common base class for Memory1 and Memory7
       */
      if (_set_romdi)
	{
	  switch (Preferences::instance()->get_kc_type())
	    {
	    case KC_TYPE_85_1:
	      ((Memory1 *)memory)->register_romdi_handler(this);
	      break;
	    case KC_TYPE_87:
	      ((Memory7 *)memory)->register_romdi_handler(this);
	      break;
	    default: break;
	    }

	  set_romdi(true);
	}
    }
}

ModuleROM1::ModuleROM1(const char *filename,
		       const char *name,
		       word_t addr,
		       dword_t size,
		       bool set_romdi) :
  ModuleInterface(name, 0, KC_MODULE_KC_85_1)
{
  _group = NULL;
  _addr = addr;
  _set_romdi = set_romdi;

  _size = (size + 0x3ff) & 0xfc00;
  _rom = new byte_t[_size];
  memset(_rom, 0xff, _size);

  set_valid(Memory::load_rom(filename, _rom, size, false));
}

ModuleROM1::~ModuleROM1(void)
{
  /*
   * FIXME: make common base class for Memory1 and Memory7
   */
  if (_set_romdi)
    {
      set_romdi(false);
      switch (Preferences::instance()->get_kc_type())
	{
	case KC_TYPE_85_1:
	  ((Memory1 *)memory)->unregister_romdi_handler(this);
	  break;
	case KC_TYPE_87:
	  ((Memory7 *)memory)->unregister_romdi_handler(this);
	  break;
	default: break;
	}
    }

  if (_group)
    memory->unregister_memory(_group);

  delete[] _rom;
}

bool
ModuleROM1::is_active(void)
{
  return _group->is_active();
}

void
ModuleROM1::set_active(bool active)
{
  _group->set_active(active);
  memory->reload_mem_ptr();
}

void
ModuleROM1::m_out(word_t addr, byte_t val)
{
}

ModuleInterface *
ModuleROM1::clone(void)
{
  return new ModuleROM1(*this);
}

void
ModuleROM1::reset(bool power_on)
{
  set_romdi(_set_romdi);
}

void
ModuleROM1::set_romdi(bool val)
{
  _set_romdi = val;
  switch (Preferences::instance()->get_kc_type())
    {
    case KC_TYPE_85_1:
      ((Memory1 *)memory)->set_romdi(val);
      break;
    case KC_TYPE_87:
      ((Memory7 *)memory)->set_romdi(val);
      break;
    default:
      break;
    }
}

void
ModuleROM1::romdi(bool val)
{
}
