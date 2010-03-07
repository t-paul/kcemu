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

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/pio.h"
#include "kc/z80.h"

#include "ui/generic/ui_vcs80.h"

#include "libdbg/dbg.h"

UI_VCS80::UI_VCS80(void) : UI_LED(624, 112, 1, 64, 16, 8, 4, 2, 32, 128)
{
  _counter = 7;
  for (int a = 0;a < VCS80_NR_OF_LEDS;a++)
    _led_value[a] = 0;
}

UI_VCS80::~UI_VCS80(void)
{
}

void
UI_VCS80::generic_update(Scanline *scanline, MemAccess *memaccess, bool clear_cache)
{
  generic_draw_digit(65 * 4 + 32, 12, 4, 0, clear_cache);
  
  for (int a = 0;a < VCS80_NR_OF_LEDS;a++)
    {
      int idx = 7 - a;
      
      if (idx > 3)
	idx++;
      
      generic_draw_digit(65 * idx + 32, 12, a, _led_value[a], clear_cache);
    }
}

void
UI_VCS80::generic_signal_v_retrace(bool value)
{
  led_update();
}

void
UI_VCS80::led_update(void)
{
  _led_value[_counter] = pio->in_B_DATA() & 0x7f;
  DBG(2, form("KCemu/ui/vcs80/led_update",
	      "UI_VCS80::led_update(): LED: [%d] %02x %02x %02x %02x - %02x %02x %02x %02x\n",
	      _counter,
	      _led_value[7], _led_value[6], _led_value[5], _led_value[4],
	      _led_value[3], _led_value[2], _led_value[1], _led_value[0]));

  // trigger next display interrupt
  _counter = (_counter + 1) % 8;

  pio->set_A_EXT(0xf7, _counter | 0x70);
  pio->set_A_EXT(0xf7, _counter | 0xf0);
}
