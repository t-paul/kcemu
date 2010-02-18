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
#include "kc/z80.h"
#include "kc/pio8.h"

#include "ui/generic/ui_8.h"

UI_8::UI_8(void) : UI_LED(464, 112, 4, 2, 64, 128, 32, 1, 8, 16)
{
}

UI_8::~UI_8(void)
{
}

void
UI_8::generic_update(Scanline *scanline, MemAccess *memaccess, bool clear_cache)
{
  byte_t led_value;

  for (int a = 0;a < 6;a++)
    {
      led_value = ((PIO8_1 *)pio)->get_led_value(a);
      generic_draw_digit(65 * a + 62, 12, a, led_value, clear_cache);
    }

  /* TAPE OUT led */
  led_value = ((PIO8_1 *)pio)->get_led_value(6);
  generic_draw_led(18, 20, led_value ? 1 : 4, clear_cache);

  /* HALT led */
  led_value = z80->get_halt();
  generic_draw_led(18, 60, led_value ? 3 : 1, clear_cache);
}
