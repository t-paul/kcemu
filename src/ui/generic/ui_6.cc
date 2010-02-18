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
#include "kc/ports6.h"

#include "ui/generic/ui_6.h"

UI_6::UI_6(void) : UI_LED(560, 112, 32, 64, 1, 2, 4, 128, 16, 8)
{
  _idx = 0;
  for (int a = 0;a < NR_OF_LEDS;a++)
    for (int b = 0;b < HISTORY_LEN;b++)
      _data[a][b] = 0;
}

UI_6::~UI_6(void)
{
}

void
UI_6::generic_update(Scanline *scanline, MemAccess *memaccess, bool clear_cache)
{

  for (int a = 0;a < NR_OF_LEDS;a++)
    {
      _data[a][_idx] = ((Ports6 *)porti)->get_led_value(a);

      byte_t led_value = 0;
      for (int b = 0;b < HISTORY_LEN;b++)
	led_value |= _data[a][b];

      generic_draw_digit(65 * a + 32, 12, a, led_value, clear_cache);
    }

  _idx = (_idx + 1) % HISTORY_LEN;
}
