/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: ui_8.cc,v 1.1 2002/06/09 14:24:34 torsten_paul Exp $
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

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/z80.h"
#include "kc/pio8.h"

#include "ui/generic/ui_8.h"

UI_8::UI_8(void)
{
  int len = get_real_width() * get_real_height();

  _bitmap = new byte_t[len];
  memset(_bitmap, 0, len);
}

UI_8::~UI_8(void)
{
  delete _bitmap;
}


int
UI_8::get_real_width(void)
{
  return 460;
}

int
UI_8::get_real_height(void)
{
  return 110;
}

void
UI_8::generic_put_pixel(int x, int y, byte_t col)
{
  int idx = y * get_real_width() + x;
  _bitmap[idx] = col;
}

void
UI_8::generic_draw_hline(int x, int y, byte_t col)
{
  for (int a = 2;a < 38;a++)
    generic_put_pixel(x + a, y, col);
  for (int a = 3;a < 37;a++)
    {
      generic_put_pixel(x + a, y - 1, col);
      generic_put_pixel(x + a, y + 1, col);
    }
  for (int a = 4;a < 36;a++)
    {
      generic_put_pixel(x + a, y - 2, col);
      generic_put_pixel(x + a, y + 2, col);
    }
  for (int a = 5;a < 35;a++)
    {
      generic_put_pixel(x + a, y - 3, col);
      generic_put_pixel(x + a, y + 3, col);
    }
}

void
UI_8::generic_draw_vline(int x, int y, byte_t col)
{
  for (int a = 2;a < 38;a++)
    {
      int q = a / 8;
      generic_put_pixel(x - q, y + a, col);
    }
  for (int a = 3;a < 37;a++)
    {
      int q = a / 8;
      generic_put_pixel(x - q - 1, y + a, col);
      generic_put_pixel(x - q + 1, y + a, col);
    }
  for (int a = 4;a < 36;a++)
    {
      int q = a / 8;
      generic_put_pixel(x - q - 2, y + a, col);
      generic_put_pixel(x - q + 2, y + a, col);
    }
  for (int a = 5;a < 35;a++)
    {
      int q = a / 8;
      generic_put_pixel(x - q - 3, y + a, col);
      generic_put_pixel(x - q + 3, y + a, col);
    }
}

void
UI_8::generic_draw_led(int x, int y, byte_t col)
{
  int space[10] = {
    6,  4,  3,  2,  1,  1,  0,  0,  0,  0
  };
  int len[10] = {
    8, 12, 14, 16, 18, 18, 20, 20, 20, 20
  };

  for (int yy = 0;yy < 10;yy++)
    for (int xx = 0;xx < len[yy];xx++)
      {
	generic_put_pixel(x + xx + space[yy], y + yy, col);
	generic_put_pixel(x + xx + space[yy], y + 19 - yy, col);
      }
}

void
UI_8::generic_draw_point(int x, int y, byte_t col)
{
  int space[4] = {
    2, 1, 0, 0
  };
  int len[4] = {
    4, 6, 8, 8
  };

  for (int yy = 0;yy < 4;yy++)
    for (int xx = 0;xx < len[yy];xx++)
      {
	generic_put_pixel(x + xx + space[yy], y - 4 + yy, col);
	generic_put_pixel(x + xx + space[yy], y - 4 + 7 - yy, col);
      }
}

void
UI_8::generic_draw_digit(int x, int y, int index, byte_t led_value)
{
  int a;
  byte_t fg, bg;

  fg = 1;
  bg = 2;

  generic_draw_hline(x +  4, y     , (led_value &   4) ? bg : fg);
  generic_draw_hline(x     , y + 40, (led_value &   8) ? bg : fg);
  generic_draw_hline(x -  4, y + 80, (led_value & 128) ? bg : fg);

  generic_draw_vline(x +  4, y     , (led_value &   2) ? bg : fg);
  generic_draw_vline(x + 44, y     , (led_value &   1) ? bg : fg);
  generic_draw_vline(x     , y + 40, (led_value &  64) ? bg : fg);
  generic_draw_vline(x + 40, y + 40, (led_value &  32) ? bg : fg);

  generic_draw_point(x + 42, y + 80, (led_value &  16) ? bg : fg);
}

void
UI_8::generic_update(void)
{
  byte_t led_value;

  for (int a = 0;a < 6;a++)
    {
      led_value = ((PIO8_1 *)pio)->get_led_value(a);
      generic_draw_digit(65 * a + 60, 10, a, led_value);
    }

  /* TAPE OUT led */
  led_value = ((PIO8_1 *)pio)->get_led_value(6);
  generic_draw_led(16, 20, led_value ? 1 : 4);

  /* HALT led */
  led_value = z80->get_halt();
  generic_draw_led(16, 60, led_value ? 3 : 1);
}

UI_ModuleInterface *
UI_8::getModuleInterface(void)
{
  static DummyModuleHandler *i = new DummyModuleHandler();
  return i;
}

TapeInterface  *
UI_8::getTapeInterface(void)
{
  static DummyTapeHandler *i = new DummyTapeHandler();
  return i;
}

DebugInterface *
UI_8::getDebugInterface(void)
{
  return 0;
}
