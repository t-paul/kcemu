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
#include "kc/pio6.h"
#include "kc/keyb6.h"
#include "kc/ports6.h"

#include "libdbg/dbg.h"

Ports6::Ports6(void)
{
  _val = 0;
  for (int a = 0;a < 8;a++)
    _led[a] = 0;
}

Ports6::~Ports6(void)
{
}

byte_t
Ports6::in(word_t addr)
{
  byte_t val = 0xff;

  DBG(2, form("KCemu/Ports/6/in",
	      "Ports6: in():  addr = %04x (returning %02x)\n",
	      addr, val));
  
  return val;
}

void
Ports6::out(word_t addr, byte_t val)
{
  _val = val;

  DBG(2, form("KCemu/Ports/6/out",
              "Ports6: out(): addr = %04x, val = %02x\n",
              addr, val));
  
  // FIXME: decouple classes
  ((Keyboard6 *)keyboard)->set_line(val);
  byte_t pio_val = ((PIO6_1 *)pio)->get_led_value();

  update_led_value(_val, pio_val);
}

void
Ports6::update_led_value(byte_t port_val, byte_t pio_val)
{
  for (int a = 0;a < 8;a++)
    if (port_val & (128 >> a))
      _led[a] = pio_val;
}

byte_t
Ports6::get_led_value(int idx)
{
  return _led[idx];
}

int
Ports6::callback_A_in(void)
{
  return 0;
}

int
Ports6::callback_B_in(void)
{
  return 0;
}

void
Ports6::callback_A_out(byte_t val)
{
  update_led_value(_val, val);
}

void
Ports6::callback_B_out(byte_t val)
{
}
