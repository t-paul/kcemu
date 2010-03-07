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

#include <stdio.h>
#include <string.h>

#include "kc/system.h"

#include "kc/mod_4m.h"

Module4M::Module4M(Module4M &tmpl) :
  ModuleInterface(tmpl.get_name(), tmpl.get_id(), tmpl.get_type())
{
  bool valid = true;
  for (int a = 0;a < 4;a++)
    {
      _master[a] = NULL;
      _module[a] = tmpl.get_master(a)->clone();
      valid = valid & _module[a]->is_valid();
    }
  set_valid(valid);
}

Module4M::Module4M(const char *name, byte_t id) :
  ModuleInterface(name, id, KC_MODULE_KC_85_3)
{
  for (int a = 0;a < 4;a++)
    {
      char buf[100];
      snprintf(buf, 100, "%s/%d", name, a);
      _master[a] = new Module1M(buf, id);
      _module[a] = NULL;
    }

  set_valid(true);
}

Module4M::~Module4M(void)
{
  for (int a = 0;a < 4;a++)
    if (_module[a])
      delete _module[a];

  for (int a = 0;a < 4;a++)
    if (_master[a])
      delete _master[a];
}

Module1M *
Module4M::get_master(int idx)
{
  return _master[idx];
}

byte_t
Module4M::m_in(word_t addr)
{
  int idx = (addr >> 8) & 3;
  return _module[idx]->m_in(addr & 0xfcff);
}

void
Module4M::m_out(word_t addr, byte_t val)
{
  int idx = (addr >> 8) & 3;
  _module[idx]->m_out(addr & 0xfcff, val);
}

ModuleInterface *
Module4M::clone(void)
{
  return new Module4M(*this);
}

void
Module4M::reset(bool power_on)
{
  for (int a = 0;a < 4;a++)
    _module[a]->reset(power_on);
}
