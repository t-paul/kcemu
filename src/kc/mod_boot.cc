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
#include "kc/mod_boot.h"

using namespace std;

ModuleBOOT::ModuleBOOT(ModuleBOOT &tmpl) :
  ModuleROM1(tmpl)
{
  _romdi_handling = tmpl._romdi_handling;
  switch (Preferences::instance()->get_kc_type())
    {
    case KC_TYPE_85_1:
      ((Memory1 *)memory)->register_memory_handler(this);
      break;
    case KC_TYPE_87:
      ((Memory7 *)memory)->register_memory_handler(this);
      break;
    default:
      break;
    }

  reset(true);
}

ModuleBOOT::ModuleBOOT(const char *filename,
		       const char *name,
		       word_t addr,
		       dword_t size,
		       bool set_romdi) :
  ModuleROM1(filename, name, addr, size, set_romdi)
{
  _romdi_handling = set_romdi;
}

ModuleBOOT::~ModuleBOOT(void)
{
  switch (Preferences::instance()->get_kc_type())
    {
    case KC_TYPE_85_1:
      ((Memory1 *)memory)->unregister_memory_handler(this);
      break;
    case KC_TYPE_87:
      ((Memory7 *)memory)->unregister_memory_handler(this);
      break;
    default:
      break;
    }
}

ModuleInterface *
ModuleBOOT::clone(void)
{
  return new ModuleBOOT(*this);
}

byte_t
ModuleBOOT::memory_read_byte(word_t addr)
{
  return 0xff;
}

void
ModuleBOOT::memory_write_byte(word_t addr, byte_t val)
{
  if (addr < 0xf800)
    return;

  bool state = (addr & 0x0400) == 0;
  
  set_active(state);
}

void
ModuleBOOT::set_active(bool active)
{
  if (is_active() == active)
    return;

  if (_romdi_handling)
    set_romdi(active);

  ModuleROM1::set_active(active);
}

void
ModuleBOOT::reset(bool power_on)
{
  set_active(true);
}
