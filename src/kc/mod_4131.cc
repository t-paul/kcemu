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

#include <cairo/cairo.h>
#include <cairo/cairo-pdf.h>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/z80.h"
#include "kc/pio1.h"
#include "kc/plotter.h"
#include "kc/mod_4131.h"

#include "libdbg/dbg.h"

using namespace std;

ModuleXY4131::ModuleXY4131(ModuleXY4131 &tmpl) :
  ModuleInterface(tmpl.get_name(), tmpl.get_id(), tmpl.get_type())
{
  _master = false;
  pio->register_callback_B_in(this);
  pio->register_callback_B_out(this);

  pio->set_B_EXT(0xff, 0x00);

  _val = 0;
  set_valid(true);
}

ModuleXY4131::ModuleXY4131(const char *name) :
  ModuleInterface(name, 0, KC_MODULE_KC_85_1)
{
  _master = true;
  set_valid(true);
}

ModuleXY4131::~ModuleXY4131(void)
{
  if (!_master)
    plotter->close_pdf();
}

void
ModuleXY4131::m_out(word_t addr, byte_t val)
{
}

int
ModuleXY4131::callback_A_in(void)
{
  return -1;
}

void
ModuleXY4131::callback_A_out(byte_t val)
{
}

int
ModuleXY4131::callback_B_in(void)
{
  return -1;
}

/**
 *    0,  0 - 100,  0           83/87/83/87/... => X V
 *  100,  0 - 100,100           81/85/81/85/... => Y V
 *  100,100 -   0,100           82/86/82/86/... => X R
 *    0,100 -   0,  0           80/84/80/84/... => Y R
 */
void
ModuleXY4131::callback_B_out(byte_t val)
{
  if ((_val & 0x80) ^ (val & 0x80))
    {
      if (val & 0x80)
        plotter->pen_down();
      else
        plotter->pen_up();
    }

  if (((_val & 4) == 0) && ((val & 4) == 4))
    {
      int direction = -1;
      if (val & 0x01)
        direction = 1;

      if (val & 0x02)
        plotter->step(0, direction);
      else
        plotter->step(direction, 0);
    }

  _val = val;
}

ModuleInterface *
ModuleXY4131::clone(void)
{
  return new ModuleXY4131(*this);
}

void
ModuleXY4131::reset(bool power_on)
{
}
