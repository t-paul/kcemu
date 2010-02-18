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
#include "kc/c80/display.h"

#include "ui/generic/ui_c80.h"

#include "libdbg/dbg.h"

UI_C80::UI_C80(void) : UI_LED(560, 112, 1, 32, 16, 8, 4, 2, 64, 128)
{
}

UI_C80::~UI_C80(void)
{
}

void
UI_C80::generic_update(Scanline *scanline, MemAccess *memaccess, bool clear_cache)
{
  for (int a = 0;a < C80_NR_OF_LEDS;a++)
    {
      byte_t led_value = display_c80->get_led_value(a);
      generic_draw_digit(65 * a + 32, 12, a, led_value, clear_cache);
    }
}
