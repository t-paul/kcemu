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
#include <fstream>
#include <iostream>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/memory1.h"
#include "kc/memory7.h"
#include "kc/mod_boot.h"

using namespace std;

ModuleBOOT::ModuleBOOT(ModuleBOOT &tmpl) :
  ModuleROM1(tmpl)
{
  _romdi_val = 0;
  switch (get_kc_type())
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
}

ModuleBOOT::ModuleBOOT(const char *filename,
		       const char *name,
		       word_t addr,
		       dword_t size,
		       bool set_romdi) :
  ModuleROM1(filename, name, addr, size, (get_kc_type() == KC_TYPE_87) ? set_romdi : false)
{
  _romdi_val = 0;
}

ModuleBOOT::~ModuleBOOT(void)
{
  switch (get_kc_type())
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
  return 0;
}

void
ModuleBOOT::reset(bool power_on)
{
  set_romdi(1);
}

void
ModuleBOOT::set_romdi(bool romdi_val)
{
  if (_romdi_val == romdi_val)
    return;

  set_active(romdi_val);

  _romdi_val = romdi_val;

  if (get_kc_type() != KC_TYPE_87)
    return;

  ModuleROM1::set_romdi(_romdi_val);
}

void
ModuleBOOT::memory_write_byte(word_t addr, byte_t val)
{
  if (addr < 0xf800)
    return;
  
  set_romdi((addr & 0x0400) == 0);
}
