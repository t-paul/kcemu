/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
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

#include <iostream>
#include <iomanip>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/gdc.h"
#include "kc/mod_gdc.h"

#include "ui/ui.h"
#include "ui/generic/ui_0.h"

ModuleGDC::ModuleGDC(ModuleGDC &tmpl) :
  ModuleInterface(tmpl.get_name(), tmpl.get_id(), tmpl.get_type())
{
  _portg = NULL;
  _master = &tmpl;

  if (_master->get_count() == 0)
    {
      _master->set_count(1);

      gdc = new GDC(); // global in kc.cc
      _portg = ports->register_ports("GDC", 0x18,  2, gdc, 0);

      ui->set_mode(UI_0::UI_GENERIC_MODE_GDC);

      set_valid(true);
    }
  else
    {
      char buf[1024];
      snprintf(buf, sizeof(buf),
	       _("It's not possible to have more than one\n"
		 "module of type %s!"),
	       get_name());
      set_error_text(buf);
      set_valid(false);
    }
}

ModuleGDC::ModuleGDC(const char *name) :
  ModuleInterface(name, 0, KC_MODULE_Z1013)
{
  _portg = NULL;
  _count = 0;
  set_valid(true);
}

ModuleGDC::~ModuleGDC(void)
{
  if (_portg)
    {
      _master->set_count(0);
      ports->unregister_ports(_portg);
      ui->set_mode(UI_0::UI_GENERIC_MODE_Z1013_32x32);
    }

  if (gdc)
    {
      delete gdc;
      gdc = NULL;
    }
}

void
ModuleGDC::m_out(word_t addr, byte_t val)
{
}

ModuleInterface *
ModuleGDC::clone(void)
{
  return new ModuleGDC(*this);
}

void
ModuleGDC::reset(bool power_on)
{
}

int
ModuleGDC::get_count(void)
{
  return _count;
}

void
ModuleGDC::set_count(int count)
{
  _count = count;
}
