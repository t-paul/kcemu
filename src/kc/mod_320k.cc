/*
 *  KCemu -- The emulator for the KC85 homecomputer series and much more.
 *  Copyright (C) 1997-2010 Torsten Paul
 *  Copyright (C) 2005 Alexander Schön
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

#include "ui/status.h"

#include "kc/z80.h"
#include "kc/memory7.h"
#include "kc/mod_320k.h"

using namespace std;

Module320k::Module320k(Module320k &tmpl) :
  ModuleInterface(tmpl.get_name(), tmpl.get_id(), tmpl.get_type()),
  InterfaceCircuit(tmpl.get_name())
{
  _bank = 0xff;
  _master = false;

  _rom = tmpl._rom;

  register_memory_bank(0);

  if (Preferences::instance()->get_kc_type() == KC_TYPE_87)
    {
      ((Memory7 *)memory)->set_romdi(true);
      ((Memory7 *)memory)->register_romdi_handler(this);
    }

  _portg = ports->register_ports(get_name(), 0xff, 1, this, 0);

  z80->register_ic(this);
  
  set_valid(true);
}

Module320k::Module320k(const char *filename, const char *name) :
  ModuleInterface(name, 0, KC_MODULE_KC_85_1),
  InterfaceCircuit(name)
{
  _bank = 0xff;
  _master = true;
  for (int a = 0;a < 5;a++)
    _group[a] = NULL;

  _rom = new byte_t[0x280000];

  set_valid(Memory::load_rom(filename, &_rom[0x00000], 0x280000, false));
}

Module320k::~Module320k(void)
{
  if (Preferences::instance()->get_kc_type() == KC_TYPE_87)
    {
      ((Memory7 *)memory)->set_romdi(false);
      ((Memory7 *)memory)->unregister_romdi_handler(this);
    }

  unregister_memory_bank();

  if (_master)
    delete[] _rom;
  else
    z80->unregister_ic(this);
}

void
Module320k::register_memory_bank(byte_t bank)
{
  for (int a = 0;a < 5;a++)
    _group[a] = memory->register_memory(get_name(),
					0xc000 + a * 0x800,
					0x0800,
					_rom + a * 0x80000 + bank * 0x800,
					0,
					true);
}

void
Module320k::unregister_memory_bank(void)
{
  for (int a = 0;a < 5;a++)
    {
      if (_group[a] != NULL)
	memory->unregister_memory(_group[a]);
      _group[a] = NULL;
    }
}

void
Module320k::m_out(word_t addr, byte_t val)
{
}

ModuleInterface *
Module320k::clone(void)
{
  return new Module320k(*this);
}

void
Module320k::romdi(bool val)
{
  for (int a = 0;a < 5;a++)
    if (_group[a])
      _group[a]->set_active(!val);
}


byte_t
Module320k::in(word_t addr)
{
  return 0xff;   
}

void
Module320k::out(word_t addr, byte_t val)
{
  val &= 0xff;
  if (val == _bank)
    return;

  _bank = val;

  char buf[1024];
  const char *fmt = _("2,5 MB Modul Bank (%d / %02xh)");
  snprintf(buf, sizeof(buf), fmt, _bank, _bank);
  Status::instance()->setMessage(buf);

  unregister_memory_bank();
  register_memory_bank(_bank);
}  

void
Module320k::reset(bool power_on)
{
  /*
   *  initialize memory bank switch after reset / power on
   */
  out(0xff, 0);
}
