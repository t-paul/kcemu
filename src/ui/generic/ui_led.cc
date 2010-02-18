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

#include "ui/generic/ui_led.h"

UI_LED::UI_LED(int width,
	       int height,
	       int a,
	       int b,
	       int c,
	       int d,
	       int e,
	       int f,
	       int g,
	       int h)
{
  int len = width * height;

  _a = a;
  _b = b;
  _c = c;
  _d = d;
  _e = e;
  _f = f;
  _g = g;
  _h = h;
  
  set_real_screen_size(width, height);

  _bitmap = new byte_t[len];
  _dirty_size = len / 64;
  _dirty = new byte_t[_dirty_size];

  for (int i = 0;i < len;i++)
    _bitmap[i] = 0;

  for (int i = 0;i < _dirty_size;i++)
    _dirty[i] = 1;
}

UI_LED::~UI_LED(void)
{
  delete[] _bitmap;
  delete[] _dirty;
}

void
UI_LED::generic_put_pixel(int x, int y, byte_t col, bool clear_cache)
{
  int idx = y * get_real_width() + x;
  if ((_bitmap[idx] != col) || clear_cache)
    {
      _bitmap[idx] = col;
      _dirty[(y / 8) * (get_real_width() / 8) + (x / 8)] = 1;
    }
}

void
UI_LED::generic_draw_hline(int x, int y, byte_t col, bool clear_cache)
{
  for (int a = 2;a < 38;a++)
    generic_put_pixel(x + a, y, col, clear_cache);
  for (int a = 3;a < 37;a++)
    {
      generic_put_pixel(x + a, y - 1, col, clear_cache);
      generic_put_pixel(x + a, y + 1, col, clear_cache);
    }
  for (int a = 4;a < 36;a++)
    {
      generic_put_pixel(x + a, y - 2, col, clear_cache);
      generic_put_pixel(x + a, y + 2, col, clear_cache);
    }
  for (int a = 5;a < 35;a++)
    {
      generic_put_pixel(x + a, y - 3, col, clear_cache);
      generic_put_pixel(x + a, y + 3, col, clear_cache);
    }
}

void
UI_LED::generic_draw_vline(int x, int y, byte_t col, bool clear_cache)
{
  for (int a = 2;a < 38;a++)
    {
      int q = a / 8;
      generic_put_pixel(x - q, y + a, col, clear_cache);
    }
  for (int a = 3;a < 37;a++)
    {
      int q = a / 8;
      generic_put_pixel(x - q - 1, y + a, col, clear_cache);
      generic_put_pixel(x - q + 1, y + a, col, clear_cache);
    }
  for (int a = 4;a < 36;a++)
    {
      int q = a / 8;
      generic_put_pixel(x - q - 2, y + a, col, clear_cache);
      generic_put_pixel(x - q + 2, y + a, col, clear_cache);
    }
  for (int a = 5;a < 35;a++)
    {
      int q = a / 8;
      generic_put_pixel(x - q - 3, y + a, col, clear_cache);
      generic_put_pixel(x - q + 3, y + a, col, clear_cache);
    }
}

void
UI_LED::generic_draw_led(int x, int y, byte_t col, bool clear_cache)
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
	generic_put_pixel(x + xx + space[yy], y + yy, col, clear_cache);
	generic_put_pixel(x + xx + space[yy], y + 19 - yy, col, clear_cache);
      }
}

void
UI_LED::generic_draw_point(int x, int y, byte_t col, bool clear_cache)
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
	generic_put_pixel(x + xx + space[yy], y - 4 + yy, col, clear_cache);
	generic_put_pixel(x + xx + space[yy], y - 4 + 7 - yy, col, clear_cache);
      }
}

/*
 *
 *                  LC 80          POLY 880        VCS 80           C 80
 *
 *                    4              32               1              1
 *      -a-	     ---      	     ---             ---            ---
 *     |   |	 2  |   | 1   	 64 |   | 128    64 |   | 2     32 |   | 2
 *     b   f	    | 8 |     	    |16 |           |32 |          |64 |
 *      -g-	     ---      	     ---             ---            ---
 *     |   |	 64 |   | 32  	  1 |   | 4      16 |   | 4     16 |   | 4
 *     c   e	    |   |     	    |   |           |   |          |   |
 *      -d- (h)	     ---      	     ---             ---            ---
 * 		     128  (16)	      2  (8)          8  (-)         8  (128)
 */
void
UI_LED::generic_draw_digit(int x, int y, int index, byte_t led_value, bool clear_cache)
{
  byte_t fg, bg;

  fg = 1;
  bg = 2;

  generic_draw_hline(x +  4, y     , (led_value & _a) ? bg : fg, clear_cache);
  generic_draw_hline(x     , y + 40, (led_value & _g) ? bg : fg, clear_cache);
  generic_draw_hline(x -  4, y + 80, (led_value & _d) ? bg : fg, clear_cache);

  generic_draw_vline(x +  4, y     , (led_value & _b) ? bg : fg, clear_cache);
  generic_draw_vline(x + 44, y     , (led_value & _f) ? bg : fg, clear_cache);
  generic_draw_vline(x     , y + 40, (led_value & _c) ? bg : fg, clear_cache);
  generic_draw_vline(x + 40, y + 40, (led_value & _e) ? bg : fg, clear_cache);

  generic_draw_point(x + 42, y + 80, (led_value & _h) ? bg : fg, clear_cache);
}
