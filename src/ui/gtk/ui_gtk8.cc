/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: ui_gtk8.cc,v 1.2 2002/06/09 14:24:34 torsten_paul Exp $
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

#include <iostream.h>
#include <iomanip.h>

#include <unistd.h>
#include <sys/time.h>

#include "kc/z80.h"
#include "kc/pio8.h"

#include "ui/gtk/ui_gtk8.h"

#include "libdbg/dbg.h"

// FIXME: correct LC80_CB_OFFSET
#define LC80_CB_OFFSET (18000)

UI_Gtk8::UI_Gtk8(void) : UI_Gtk()
{
  reset();
}

UI_Gtk8::~UI_Gtk8(void)
{
}

int
UI_Gtk8::get_width(void)
{
  return kcemu_ui_scale * 460;
}

int
UI_Gtk8::get_height(void)
{
  return kcemu_ui_scale * 110;
}

int
UI_Gtk8::get_callback_offset(void)
{
  return LC80_CB_OFFSET;
}

void
UI_Gtk8::callback(void * /* data */)
{
  ui_callback();
}

const char *
UI_Gtk8::get_title(void)
{
  return "LC80 Emulator";
}

void
UI_Gtk8::allocate_colors(double saturation_fg,
			 double saturation_bg,
			 double brightness_fg,
			 double brightness_bg,
			 double black_level,
			 double white_level)
{
    int a;
    char *color_names[] = {
      "rgb:00/00/00", /* black */
      "rgb:00/20/00", /* really dark green */
      "rgb:00/e0/00", /* green */
      "rgb:ff/8c/00", /* dark orange */
      "rgb:9a/cd/32", /* led green */
      0,
    };

    _colormap = gdk_colormap_get_system();
    for (a = 0;color_names[a];a++) {
	gdk_color_parse(color_names[a], &_col[a]);
	gdk_color_alloc(_colormap, &_col[a]);
    }
}

void
UI_Gtk8::draw_hline(GdkImage *image, int x, int y, gulong color)
{
  for (int a = 2;a < 38;a++)
    gdk_image_put_pixel(image, x + a, y, color);
  for (int a = 3;a < 37;a++)
    {
      gdk_image_put_pixel(image, x + a, y - 1, color);
      gdk_image_put_pixel(image, x + a, y + 1, color);
    }
  for (int a = 4;a < 36;a++)
    {
      gdk_image_put_pixel(image, x + a, y - 2, color);
      gdk_image_put_pixel(image, x + a, y + 2, color);
    }
  for (int a = 5;a < 35;a++)
    {
      gdk_image_put_pixel(image, x + a, y - 3, color);
      gdk_image_put_pixel(image, x + a, y + 3, color);
    }
}

void
UI_Gtk8::draw_vline(GdkImage *image, int x, int y, gulong color)
{
  for (int a = 2;a < 38;a++)
    {
      int q = a / 8;
      gdk_image_put_pixel(image, x - q, y + a, color);
    }
  for (int a = 3;a < 37;a++)
    {
      int q = a / 8;
      gdk_image_put_pixel(image, x - q - 1, y + a, color);
      gdk_image_put_pixel(image, x - q + 1, y + a, color);
    }
  for (int a = 4;a < 36;a++)
    {
      int q = a / 8;
      gdk_image_put_pixel(image, x - q - 2, y + a, color);
      gdk_image_put_pixel(image, x - q + 2, y + a, color);
    }
  for (int a = 5;a < 35;a++)
    {
      int q = a / 8;
      gdk_image_put_pixel(image, x - q - 3, y + a, color);
      gdk_image_put_pixel(image, x - q + 3, y + a, color);
    }
}

void
UI_Gtk8::draw_led(GdkImage *image, int x, int y, gulong color)
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
	gdk_image_put_pixel(image, x + xx + space[yy], y + yy, color);
	gdk_image_put_pixel(image, x + xx + space[yy], y + 19 - yy, color);
      }
}

void
UI_Gtk8::draw_point(GdkImage *image, int x, int y, gulong color)
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
	gdk_image_put_pixel(image, x + xx + space[yy], y - 4 + yy, color);
	gdk_image_put_pixel(image, x + xx + space[yy], y - 4 + 7 - yy, color);
      }
}

void
UI_Gtk8::draw_digit(GdkImage *image, int x, int y, int index, byte_t led_value)
{
  int a;
  gulong fg, bg;

  fg = _col[1].pixel;
  bg = _col[2].pixel;

  draw_hline(image, x +  4, y     , (led_value &   4) ? bg : fg);
  draw_hline(image, x     , y + 40, (led_value &   8) ? bg : fg);
  draw_hline(image, x -  4, y + 80, (led_value & 128) ? bg : fg);

  draw_vline(image, x +  4, y     , (led_value &   2) ? bg : fg);
  draw_vline(image, x + 44, y     , (led_value &   1) ? bg : fg);
  draw_vline(image, x     , y + 40, (led_value &  64) ? bg : fg);
  draw_vline(image, x + 40, y + 40, (led_value &  32) ? bg : fg);

  draw_point(image, x + 42, y + 80, (led_value &  16) ? bg : fg);
}

void
UI_Gtk8::update(bool full_update, bool clear_cache)
{
  int a;
  byte_t led_value;

  if (full_update)
    {
      gdk_draw_image(GTK_WIDGET(_main.canvas)->window, _gc, _image,
                     0, 0, 0, 0, get_width(), get_height());
      return;
    }

  for (a = 0;a < 6;a++)
    {
      led_value = ((PIO8_1 *)pio)->get_led_value(a);
      draw_digit(_image, 65 * a + 60, 10, a, led_value);
    }

  /* TAPE OUT led */
  led_value = ((PIO8_1 *)pio)->get_led_value(6);
  draw_led(_image, 16, 20, led_value ? _col[1].pixel : _col[4].pixel);

  /* HALT led */
  draw_led(_image, 16, 60, z80->get_halt() ? _col[3].pixel : _col[1].pixel);

  gdk_draw_image(GTK_WIDGET(_main.canvas)->window, _gc, _image,
		 0, 0, 0, 0, get_width(), get_height());
}

void
UI_Gtk8::flash(bool enable)
{
}

void
UI_Gtk8::memWrite(int addr, char val)
{
}

void
UI_Gtk8::reset(bool power_on)
{
  z80->addCallback(LC80_CB_OFFSET, this, 0);
}
