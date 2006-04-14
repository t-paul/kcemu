/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2002 Torsten Paul
 *
 *  $Id$
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

#include "ui/gtk/ui_gtk9.h"

#include "libdbg/dbg.h"

UI_Gtk9::UI_Gtk9(void) : UI_Gtk()
{
  reset();
  z80->register_ic(this);
}

UI_Gtk9::~UI_Gtk9(void)
{
  z80->unregister_ic(this); 
}

void
UI_Gtk9::callback(void *data)
{
  if (data == (void *)1)
    {
      generic_signal_v_retrace(false);
    }
  else
    {
      update();
      z80->addCallback(1000, this, (void *)1);
      z80->addCallback(75000, this, 0);
      generic_signal_v_retrace(true);
    }
}

int
UI_Gtk9::get_width(void)
{
  return kcemu_ui_scale * get_real_width();
}

int
UI_Gtk9::get_height(void)
{
  return kcemu_ui_scale * get_real_height();
}

const char *
UI_Gtk9::get_title(void)
{
  return _("A5105 Emulator");
}

void
UI_Gtk9::allocate_colors(double saturation_fg,
			 double saturation_bg,
			 double brightness_fg,
			 double brightness_bg,
			 double black_level,
			 double white_level)
{
  int a;

  black_level = 0.0;
  white_level = 0.80;

  hsv_to_gdk_color(  0,             0,   black_level, &_col[ 0]); /* black */
  hsv_to_gdk_color(240, saturation_fg, brightness_fg, &_col[ 1]); /* blue */
  hsv_to_gdk_color(120, saturation_fg, brightness_fg, &_col[ 2]); /* green */
  hsv_to_gdk_color(180, saturation_fg, brightness_fg, &_col[ 3]); /* cyan */
  hsv_to_gdk_color(  0, saturation_fg, brightness_fg, &_col[ 4]); /* red */
  hsv_to_gdk_color(300, saturation_fg, brightness_fg, &_col[ 5]); /* magenta */
  hsv_to_gdk_color( 60, saturation_fg, brightness_fg, &_col[ 6]); /* yellow */
  hsv_to_gdk_color(  0,             0,   white_level, &_col[ 7]); /* white */

  saturation_fg *= 0.80;
  brightness_fg = 1.0;
  black_level = 0.40;
  white_level = 1.0;

  hsv_to_gdk_color(  0,             0,   black_level, &_col[ 8]); /* black */
  hsv_to_gdk_color(240, saturation_fg, brightness_fg, &_col[ 9]); /* blue */
  hsv_to_gdk_color(120, saturation_fg, brightness_fg, &_col[10]); /* green */
  hsv_to_gdk_color(180, saturation_fg, brightness_fg, &_col[11]); /* cyan */
  hsv_to_gdk_color(  0, saturation_fg, brightness_fg, &_col[12]); /* red */
  hsv_to_gdk_color(300, saturation_fg, brightness_fg, &_col[13]); /* magenta */
  hsv_to_gdk_color( 60, saturation_fg, brightness_fg, &_col[14]); /* yellow */
  hsv_to_gdk_color(  0,             0,   white_level, &_col[15]); /* white */

  _colormap = gdk_colormap_get_system();
  for (a = 0;a < 16;a++)
    gdk_color_alloc(_colormap, &_col[a]);
}

void
UI_Gtk9::init(void)
{
}

void
UI_Gtk9::update(bool full_update, bool clear_cache)
{
  generic_update(clear_cache);
  gtk_update(_bitmap, get_dirty_buffer(), get_dirty_buffer_size(),
	     get_real_width(), get_real_height(), full_update);
  processEvents();
  gtk_sync();
}

void
UI_Gtk9::flash(bool enable)
{
}

void
UI_Gtk9::memory_read(word_t addr)
{
}

void
UI_Gtk9::memory_write(word_t addr)
{
}

int
UI_Gtk9::get_mode(void)
{
  return generic_get_mode();
}

void
UI_Gtk9::set_mode(int mode)
{
  generic_set_mode(mode);
  gtk_resize();
}

void
UI_Gtk9::reset(bool power_on)
{
  z80->addCallback(75000, this, 0);
}
