/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: ui_gtk3.cc,v 1.14 2002/06/09 14:24:34 torsten_paul Exp $
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
#include "kc/memory.h"

#include "ui/hsv2rgb.h"
#include "ui/gtk/ui_gtk3.h"

#include "libdbg/dbg.h"

int UI_Gtk3::_bitswap1[256];
int UI_Gtk3::_bitswap2[256];
int UI_Gtk3::_bitswap3[256];

UI_Gtk3::UI_Gtk3(void) : UI_Gtk()
{
  int a, n1, n2;

  for (a = 0;a < 256;a++)
    {
      n1 = (a & 0x03) << 2;
      n2 = (a & 0x0c) >> 2;
      _bitswap1[a] = ((a & 0xf0) | n1 | n2);
      n1 = (a & 0x03) << 4;
      n2 = (a & 0x30) >> 4;
      _bitswap2[a] = ((a & 0xcc) | n1 | n2);
    }

  for (a = 0;a < 256;a++)
    _bitswap3[a] = 0x3000 + (_bitswap1[_bitswap2[a] >> 2] << 3);
  
  reset();
  // z80->register_ic(this); already done in base class!
}

UI_Gtk3::~UI_Gtk3(void)
{
  // z80->unregister_ic(this); already done in base class!
}

int
UI_Gtk3::get_width(void)
{
  return kcemu_ui_scale * 320;
}

int
UI_Gtk3::get_height(void)
{
  return kcemu_ui_scale * 256;
}

int
UI_Gtk3::get_callback_offset(void)
{
  return CB_OFFSET;
}

void
UI_Gtk3::callback(void * /* data */)
{
  ui_callback();
}

const char *
UI_Gtk3::get_title(void)
{
  return "KC 85/3 Emulator";
}

static inline void
put_pixels(GdkImage *image, int x, int y, byte_t val, gulong fg, gulong bg)
{
  int a;

  switch (kcemu_ui_scale)
    {
    case 1:
      for (a = 0;a < 8;a++)
	{
	  if (val & 0x80)
	    gdk_image_put_pixel(image, x, y, fg);
	  else
	    gdk_image_put_pixel(image, x, y, bg);
	  
	  x++;
	  val <<= 1;
	}
      break;
    case 2:
      x *= 2;
      y *= 2;
      for (a = 0;a < 8;a++)
	{
	  if (val & 0x80)
	    {
	      gdk_image_put_pixel(image, x,     y + 1, fg);
	      gdk_image_put_pixel(image, x,     y,     fg);
	      gdk_image_put_pixel(image, x + 1, y + 1, fg);
	      gdk_image_put_pixel(image, x + 1, y,     fg);
	    }
	  else
	    {
	      gdk_image_put_pixel(image, x,     y + 1, bg);
	      gdk_image_put_pixel(image, x,     y,     bg);
	      gdk_image_put_pixel(image, x + 1, y + 1, bg);
	      gdk_image_put_pixel(image, x + 1, y,     bg);
	    }
	  x += 2;
	  val <<= 1;
	}
      break;
    }
}

static inline void
hsv_to_gdk_color(double h, double s, double v, GdkColor *col)
{
  int r, g, b;

  hsv2rgb(h, s, v, &r, &g, &b);
  col->red   = r << 8;
  col->green = g << 8;
  col->blue  = b << 8;
}

void
UI_Gtk3::allocate_colors(double saturation_fg,
			 double saturation_bg,
			 double brightness_fg,
			 double brightness_bg,
			 double black_level,
			 double white_level)
{
  int a;
  
  hsv_to_gdk_color(  0,             0,   black_level, &_col[ 0]); /* black */
  hsv_to_gdk_color(240, saturation_fg, brightness_fg, &_col[ 1]); /* blue */
  hsv_to_gdk_color(  0, saturation_fg, brightness_fg, &_col[ 2]); /* red */
  hsv_to_gdk_color(300, saturation_fg, brightness_fg, &_col[ 3]); /* magenta */
  hsv_to_gdk_color(120, saturation_fg, brightness_fg, &_col[ 4]); /* green */
  hsv_to_gdk_color(180, saturation_fg, brightness_fg, &_col[ 5]); /* cyan */
  hsv_to_gdk_color( 60, saturation_fg, brightness_fg, &_col[ 6]); /* yellow */
  hsv_to_gdk_color(  0,             0,   white_level, &_col[ 7]); /* white */
  
  hsv_to_gdk_color(  0,             0,   black_level, &_col[ 8]); /* black */
  hsv_to_gdk_color(270, saturation_fg, brightness_fg, &_col[ 9]); /* blue + 30° */
  hsv_to_gdk_color( 30, saturation_fg, brightness_fg, &_col[10]); /* red + 30° */
  hsv_to_gdk_color(330, saturation_fg, brightness_fg, &_col[11]); /* magenta + 30° */
  hsv_to_gdk_color(150, saturation_fg, brightness_fg, &_col[12]); /* green + 30° */
  hsv_to_gdk_color(210, saturation_fg, brightness_fg, &_col[13]); /* cyan + 30° */
  hsv_to_gdk_color( 90, saturation_fg, brightness_fg, &_col[14]); /* yellow + 30° */
  hsv_to_gdk_color(  0,             0,   white_level, &_col[15]); /* white */
  
  hsv_to_gdk_color(  0,             0,   black_level, &_col[16]); /* black */
  hsv_to_gdk_color(240, saturation_bg, brightness_bg, &_col[17]); /* blue */
  hsv_to_gdk_color(  0, saturation_bg, brightness_bg, &_col[18]); /* red */
  hsv_to_gdk_color(300, saturation_bg, brightness_bg, &_col[19]); /* magenta */
  hsv_to_gdk_color(120, saturation_bg, brightness_bg, &_col[20]); /* green */
  hsv_to_gdk_color(180, saturation_bg, brightness_bg, &_col[21]); /* cyan */
  hsv_to_gdk_color( 60, saturation_bg, brightness_bg, &_col[22]); /* yellow */
  hsv_to_gdk_color(  0,             0, brightness_bg, &_col[23]); /* white */
  
  _colormap = gdk_colormap_get_system();
  for (a = 0;a < 24;a++)
    gdk_color_alloc(_colormap, &_col[a]);
}

void
UI_Gtk3::update(bool full_update, bool clear_cache)
{
  int c, x, y;
  gulong fg, bg;
  byte_t val, col;
  int p, pc, ys, yc;
  byte_t *irm = memory->getIRM();

  if (clear_cache)
    {
      full_update = false;
      memset(_dirty_buf, 0xff, 8192);
    }
  
  if (full_update)
    {
      gdk_draw_image(GTK_WIDGET(_main.canvas)->window, _gc, _image,
                     0, 0, 0, 0, get_width(), get_height());
      return;
    }

  p = 0;
  for (y = 0;y < 256;y++)
    {
      pc = (p & 0x7f) | ((p & 0xfe00) >> 2) + 0x2800;
      ys = _bitswap1[y];
      _dirty_buf[ys] = 0;
      for (x = 0;x < 32;x++)
        {
          int changed = 0;

          val = irm[p];
	  col = irm[pc + x];

          if (val != _pix_mem[p]) { changed++; _pix_mem[p] = val; }
          if (col != _col_mem[p]) { changed++; _col_mem[p] = col; }
	  changed += clear_cache;
          p++;
          if (!changed) continue;
          
          bg = _col[(col & 7) | 0x10].pixel;
          fg = _col[(col >> 3) & 15].pixel;

          put_pixels(_image, 8 * x, ys, val, fg, bg);
          _dirty_buf[ys & 0xf8] = 1;
        }
    }

  p = 0x2000;
  for (y = 0;y < 256;y++)
    {
      ys = _bitswap2[y];
      yc = _bitswap3[y];
      for (x = 0;x < 8;x++)
        {
          int changed = 0;

          val = irm[p];
	  col = irm[yc + x];

          if (val != _pix_mem[p]) { changed++; _pix_mem[p] = val; }
          if (col != _col_mem[p]) { changed++; _col_mem[p] = col; }
	  changed += clear_cache;
          p++;
          if (!changed) continue;
          
          bg = _col[(col & 7) | 0x10].pixel;
          fg = _col[(col >> 3) & 15].pixel;

          put_pixels(_image, 8 * x + 256, ys, val, fg, bg);
          _dirty_buf[ys & 0xf8] = 1;
        }
    }
  
  y = 0;
  _dirty_buf[256] = 0;
  while (242)
    {
      while (!_dirty_buf[y]) y += 8;
      if (y >= 256) break;
      ys = y;
      while (_dirty_buf[ys]) ys += 8;
      gdk_draw_image(GTK_WIDGET(_main.canvas)->window, _gc, _image,
                     0, kcemu_ui_scale * y,
		     0, kcemu_ui_scale * y,
		     get_width(), kcemu_ui_scale * (ys - y));
      y = ys;
    }
}

void
UI_Gtk3::flash(bool enable)
{
}

void
UI_Gtk3::memWrite(int addr, char val)
{
}

void
UI_Gtk3::reset(bool power_on)
{
  z80->addCallback(CB_OFFSET, this, 0);
}
