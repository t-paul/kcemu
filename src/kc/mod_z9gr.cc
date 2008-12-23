/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2008 Torsten Paul
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
#include "kc/mod_z9gr.h"

#include "ui/ui.h"
#include "ui/generic/ui_1.h"

#include "libdbg/dbg.h"

ModuleZ9001Graphic::ModuleZ9001Graphic(ModuleZ9001Graphic &tmpl) :
  ModuleInterface(tmpl.get_name(), tmpl.get_id(), tmpl.get_type())
{
  _portg = NULL;
  _master = &tmpl;

  _color_names[0] = "black";
  _color_names[1] = "red";
  _color_names[2] = "green";
  _color_names[3] = "yellow";
  _color_names[4] = "blue";
  _color_names[5] = "violet";
  _color_names[6] = "cyan";
  _color_names[7] = "white";

  if (_master->get_count() == 0)
    {
      _master->set_count(1);
      _portg = ports->register_ports("Graphic", 0xb8, 8, this, 0);
      z9001_graphic = new byte_t[0x1800];
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

ModuleZ9001Graphic::ModuleZ9001Graphic(const char *name) :
  ModuleInterface(name, 0, KC_MODULE_KC_85_1)
{
  _portg = NULL;
  _count = 0;
  set_valid(true);
}

ModuleZ9001Graphic::~ModuleZ9001Graphic(void)
{
  if (_portg)
    {
      _master->set_count(0);
      ports->unregister_ports(_portg);
      ui->set_mode(0);
      delete[] z9001_graphic;
      z9001_graphic = NULL;
    }
  _portg = NULL;
}

byte_t
ModuleZ9001Graphic::in(word_t addr)
{
  byte_t val = 0xff;

  word_t graddr;

  switch (addr & 0x0f)
    {
    case 0x0a:
      graddr = (addr & 0xff00) | _addr_low;
      if (graddr < 0x1800)
        val = z9001_graphic[graddr];

      DBG(2, form("KCemu/z9001graphic/data_in",
                  "ModuleZ9001Graphic::in(): addr = %04x, val = %02x, read from %04x\n",
                  addr, val, graddr));
      break;
    }

  DBG(2, form("KCemu/z9001graphic/in",
              "ModuleZ9001Graphic::in(): addr = %04x, val = %02x\n",
              addr, val));

  return val;
}

void
ModuleZ9001Graphic::out(word_t addr, byte_t val)
{
  DBG(2, form("KCemu/z9001graphic/out",
              "ModuleZ9001Graphic::out(): addr = %04x, val = %02x\n",
              addr, val));

  word_t graddr;

  switch (addr & 0x0f)
    {
    case 0x08:
      DBG(2, form("KCemu/z9001graphic/control",
                  "ModuleZ9001Graphic::out(): addr = %04x, val = %02x, ink = %s, paper = %s, graphic = %s\n",
                  addr, val, _color_names[(val >> 4) & 7], _color_names[val & 7], val & 8 ? "ON" : "OFF"));
      ui->set_mode(val);
      break;
    case 0x09:
      DBG(2, form("KCemu/z9001graphic/addr",
                  "ModuleZ9001Graphic::out(): addr = %04x, val = %02x\n",
                  addr, val, graddr));
      _addr_low = val;
      break;
    case 0x0a:
      graddr = (addr & 0xff00) | _addr_low;
      DBG(2, form("KCemu/z9001graphic/data_out",
                  "ModuleZ9001Graphic::out(): addr = %04x, val = %02x, write to %04x\n",
                  addr, val, (addr & 0xff00) | _addr_low));
      if (graddr < 0x1800)
        z9001_graphic[graddr] = val;
      break;
    }
}

void
ModuleZ9001Graphic::m_out(word_t addr, byte_t val)
{
}

ModuleInterface *
ModuleZ9001Graphic::clone(void)
{
  return new ModuleZ9001Graphic(*this);
}

void
ModuleZ9001Graphic::reset(bool power_on)
{
}

int
ModuleZ9001Graphic::get_count(void)
{
  return _count;
}

void
ModuleZ9001Graphic::set_count(int count)
{
  _count = count;
}
