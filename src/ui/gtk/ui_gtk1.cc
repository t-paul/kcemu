/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: ui_gtk1.cc,v 1.10 2002/06/09 14:24:34 torsten_paul Exp $
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

#include "ui/font1.h"

#include "ui/gtk/ui_gtk1.h"

#include "libdbg/dbg.h"

UI_Gtk1::UI_Gtk1(void) : UI_Gtk()
{
  reset();
}

UI_Gtk1::~UI_Gtk1(void)
{
}

int
UI_Gtk1::get_width(void)
{
  return kcemu_ui_scale * 320;
}

int
UI_Gtk1::get_height(void)
{
  return kcemu_ui_scale * 192;
}

int
UI_Gtk1::get_callback_offset(void)
{
  return CB_OFFSET;
}

void
UI_Gtk1::callback(void * /* data */)
{
  ui_callback();
}

const char *
UI_Gtk1::get_title(void)
{
  return "KC 85/1 Emulator";
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
	  if (val & 1)
	    gdk_image_put_pixel(image, x, y, fg);
	  else
	    gdk_image_put_pixel(image, x, y, bg);
	  
	  x++;
	  val >>= 1;
	}
      break;
    case 2:
      x *= 2;
      y *= 2;
      for (a = 0;a < 8;a++)
	{
	  if (val & 1)
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
	  val >>= 1;
	}
      break;
    }
}

void
UI_Gtk1::allocate_colors(double saturation_fg,
			 double saturation_bg,
			 double brightness_fg,
			 double brightness_bg,
			 double black_level,
			 double white_level)
{
    int a;
    char *color_names[] = {
      "rgb:00/00/00",
      "rgb:d0/00/00",
      "rgb:00/d0/00",
      "rgb:d0/d0/00",
      "rgb:00/00/d0",
      "rgb:d0/00/d0",
      "rgb:00/d0/d0",
      "rgb:d0/d0/d0",
      0,
    };

    _colormap = gdk_colormap_get_system();
    for (a = 0;color_names[a];a++) {
	gdk_color_parse(color_names[a], &_col[a]);
	gdk_color_alloc(_colormap, &_col[a]);
    }
}

void
UI_Gtk1::update(bool full_update, bool clear_cache)
{
  byte_t c;
  int x, y, z, a, yscale;
  byte_t col;
  gulong fg, bg;
  byte *irm = memory->getIRM();

  if (full_update)
    {
      gdk_draw_image(GTK_WIDGET(_main.canvas)->window, _gc, _image,
                     0, 0, 0, 0, get_width(), get_height());
      return;
    }

  //fg = _col[7].pixel;
  //bg = _col[0].pixel;

  z = 0;
  for (y = 0;y < 24;y++)
    {
      for (x = 0;x < 40;x++)
        {
          col = irm[z];
          fg = _col[(col >> 4) & 7].pixel;
          bg = _col[col & 7].pixel;
          if (col != _dirty_col[z])
            {
              _dirty_col[z] = col;
              _changed[y]++;
            }

          c = irm[0x400 + z];

          if (c != _dirty_val[z])
            {
              _dirty_val[z] = c;
              _changed[y]++;
              for (a = 0;a < 8;a++)
                put_pixels(_image, 8 * x, 8 * y + a,
                           __font[8 * c + a], fg, bg);
            }
          z++;
        }
    }

  yscale = kcemu_ui_scale * 8;
  for (y = 0;y < 24;y++)
    {
      if (_changed[y])
        {
          _changed[y] = 0;
          gdk_draw_image(GTK_WIDGET(_main.canvas)->window, _gc, _image,
                         0, yscale * y, 0, yscale * y, get_width(), yscale);
        }
    }
}

void
UI_Gtk1::flash(bool enable)
{
}

void
UI_Gtk1::memWrite(int addr, char val)
{
}

void
UI_Gtk1::reset(bool power_on)
{
  int y;

  z80->addCallback(CB_OFFSET, this, 0);
  for (y = 0;y < 24;y++)
    _changed[y] = 1;
}
