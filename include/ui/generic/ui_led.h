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

#ifndef __ui_generic_ui_led_h
#define __ui_generic_ui_led_h

#include "kc/system.h"

#include "ui/generic/ui_base.h"

class UI_LED : public UI_Base
{
 private:
  int _width;
  int _height;
  int _a, _b, _c, _d, _e, _f, _g, _h;

 protected:
  UI_LED(int width, int height, int a, int b, int c, int d, int e, int f, int g, int h);
  virtual ~UI_LED(void);

  virtual void generic_put_pixel(int x, int y, byte_t col, bool clear_cache);
  virtual void generic_draw_led(int x, int y, byte_t col, bool clear_cache);
  virtual void generic_draw_hline(int x, int y, byte_t col, bool clear_cache);
  virtual void generic_draw_vline(int x, int y, byte_t col, bool clear_cache);
  virtual void generic_draw_point(int x, int y, byte_t col, bool clear_cache);
  virtual void generic_draw_digit(int x, int y, int index, byte_t led_value, bool clear_cache);
};

#endif /* __ui_generic_ui_led_h */
