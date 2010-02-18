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

#include "kc/kc.h"
#include "kc/fdc7.h"
#include "kc/mod_cpm.h"

using namespace std;

ModuleCPMZ9::ModuleCPMZ9(ModuleCPMZ9 &tmpl) :
  ModuleInterface(tmpl.get_name(), tmpl.get_id(), tmpl.get_type())
{
  fdc_fdc = new FDC7(); // FIXME: global variable in kc.cc !!!

  _portg1 = ports->register_ports(get_name(), 0x98, 2, fdc_fdc, 0);
  _portg2 = ports->register_ports(get_name(), 0xa0, 2, fdc_fdc, 0);
  _portgX = ports->register_ports(get_name(), 0x10, 2, fdc_fdc, 0);

  set_valid(true);
}

ModuleCPMZ9::ModuleCPMZ9(const char *name) :
  ModuleInterface(name, 0, KC_MODULE_KC_85_1)
{
  _portg1 = _portg2 = _portgX = NULL;
  set_valid(true);
}

ModuleCPMZ9::~ModuleCPMZ9(void)
{
  if (_portg1)
    ports->unregister_ports(_portg1);
  if (_portg2)
    ports->unregister_ports(_portg2);
  if (_portgX)
    ports->unregister_ports(_portgX);

  if (fdc_fdc)
    delete fdc_fdc;

  fdc_fdc = NULL;
}

void
ModuleCPMZ9::m_out(word_t addr, byte_t val)
{
}

ModuleInterface *
ModuleCPMZ9::clone(void)
{
  return new ModuleCPMZ9(*this);
}

void
ModuleCPMZ9::reset(bool power_on)
{
}

byte_t
ModuleCPMZ9::in(word_t addr)
{
  return 0xff;
}

void
ModuleCPMZ9::out(word_t addr, byte_t val)
{
}
