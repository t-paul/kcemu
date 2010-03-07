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

#include <iostream>
#include <iomanip>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/z80.h"
#include "kc/c80/display.h"

#include "libdbg/dbg.h"

DisplayC80::DisplayC80(void)
{
  _idx = 0;
  _reset = true;

  for (int a = 0;a < C80_NR_OF_LEDS;a++)
    _led_value[a] = 0;
}

DisplayC80::~DisplayC80(void)
{
}

byte_t
DisplayC80::get_led_value(int idx)
{
  return _led_value[idx];
}

void
DisplayC80::callback(void *data)
{
  long idx = (long)data;

  DBG(2, form("KCemu/DisplayC80/callback",
              "DisplayC80::callback(): idx = %d\n",
              idx));

  if ((idx > 0) && (idx < 9))
    {
      _led_value[idx - 1] = pio->in_B_DATA();

      DBG(2, form("KCemu/DisplayC80/callback",
		  "DisplayC80::callback(): LED: [%d] %02x %02x %02x %02x - %02x %02x %02x %02x\n",
		  idx - 1,
		  _led_value[7], _led_value[6], _led_value[5], _led_value[4],
		  _led_value[3], _led_value[2], _led_value[1], _led_value[0]));
    }

  pio->set_A_EXT(0x10, 0x10);
}

int
DisplayC80::callback_A_in(void)
{
  return -1;
}

int
DisplayC80::callback_B_in(void)
{
  return -1;
}

void
DisplayC80::callback_A_out(byte_t val)
{
  int a5 = (val >> 5) & 1;
  DBG(2, form("KCemu/DisplayC80/out_A",
              "DisplayC80::out_A(): val = %02x, A5 = %d\n",
              val, a5));

  if (a5)
    {
      _idx = 0;
      _reset = true;
    }
  else
    {
      _reset = false;
    }
}

void
DisplayC80::callback_B_out(byte_t val)
{
  DBG(2, form("KCemu/DisplayC80/out_B",
              "DisplayC80::out_B(): val = %02x, idx = %d, reset = %d\n",
              val, _idx, _reset));

  if (_reset)
    return;
  
  _idx++;
  z80->addCallback(C80_DISPLAY_CB_OFFSET, this, (void *)_idx);
  pio->set_A_EXT(0x10, 0x00);
  pio->strobe_B();
}
