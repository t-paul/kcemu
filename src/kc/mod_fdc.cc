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

#include <iostream>
#include <iomanip>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/fdc0s.h"
#include "kc/mod_fdc.h"

ModuleFDC::ModuleFDC(ModuleFDC &tmpl) :
  ModuleInterface(tmpl.get_name(), tmpl.get_id(), tmpl.get_type())
{
  _portg = NULL;
  _master = &tmpl;

  if ((_master->get_count() == 0) && (fdc_fdc == NULL))
    {
      _master->set_count(1);
      _fdc_type = _master->_fdc_type;

      switch (_fdc_type)
	{
	case FDC_INTERFACE_SCHNEIDER:
	  fdc_fdc = new FDC0S(); // global in kc.cc
	  _portg = ports->register_ports("FDC", 0xf0, 10, fdc_fdc, 0);
	  break;
	case FDC_INTERFACE_KRAMER:
	  return; // not yet implemented
	}

      set_valid(true);
    }
  else
    {
      char buf[1024];
      snprintf(buf, sizeof(buf),
	       _("It's not possible to have more than one\n"
		 "module of type %s!\n\n"
		 "(And due to some technical points of the emulator\n"
		 "it's also not possible to have both the Kramer and\n"
		 "Schneider variant of the floppy module active at\n"
		 "the same time.)"),
	       get_name());
      set_error_text(buf);
      set_valid(false);
    }
}

ModuleFDC::ModuleFDC(const char *name, fdc_interface_type_t fdc_type) :
  ModuleInterface(name, 0, KC_MODULE_Z1013)
{
  _count = 0;
  _portg = NULL;
  _fdc_type = fdc_type;

  switch (_fdc_type)
    {
    case FDC_INTERFACE_SCHNEIDER:
      set_valid(true);
      break;
    case FDC_INTERFACE_KRAMER:
      set_valid(false); // not yet implemented
      break;
    }
}

ModuleFDC::~ModuleFDC(void)
{
  if (_portg)
    {
      _master->set_count(0);
      ports->unregister_ports(_portg);
      delete fdc_fdc;
      fdc_fdc = NULL;
    }
}

void
ModuleFDC::m_out(word_t addr, byte_t val)
{
}

ModuleInterface *
ModuleFDC::clone(void)
{
  return new ModuleFDC(*this);
}

void
ModuleFDC::reset(bool power_on)
{
}

int
ModuleFDC::get_count(void)
{
  return _count;
}

void
ModuleFDC::set_count(int count)
{
  _count = count;
}
