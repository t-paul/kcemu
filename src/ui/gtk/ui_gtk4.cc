/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2002 Torsten Paul
 *
 *  $Id: ui_gtk4.cc,v 1.16 2002/10/31 01:38:12 torsten_paul Exp $
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

#include "ui/gtk/ui_gtk4.h"

#include "libdbg/dbg.h"

UI_Gtk4::UI_Gtk4(void)
{
  reset();
  z80->register_ic(this);
}

UI_Gtk4::~UI_Gtk4(void)
{
  z80->unregister_ic(this);
}

void
UI_Gtk4::callback(void * /* data */)
{
  z80->addCallback(CB_OFFSET, this, 0);
  update();
}

int
UI_Gtk4::get_width(void)
{
  return kcemu_ui_scale * get_real_width();;
}

int
UI_Gtk4::get_height(void)
{
  return kcemu_ui_scale * get_real_height();
}

const char *
UI_Gtk4::get_title(void)
{
  return _("KC 85/4 Emulator");
}

void
UI_Gtk4::allocate_colors(double saturation_fg,
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
UI_Gtk4::init(void)
{
}

void
UI_Gtk4::update(bool full_update, bool clear_cache)
{
  scanline.update();
  generic_update(&scanline, clear_cache);
  gtk_update(_bitmap, get_dirty_buffer(), get_dirty_buffer_size(),
	     get_real_width(), get_real_height(), full_update);
  processEvents();  
  gtk_sync();
}

void
UI_Gtk4::flash(bool enable)
{
  scanline.trigger(enable);
}

void
UI_Gtk4::memory_read(word_t addr)
{
}

void
UI_Gtk4::memory_write(word_t addr)
{
}

int
UI_Gtk4::get_mode(void)
{
  return generic_get_mode();
}

void
UI_Gtk4::set_mode(int mode)
{
  generic_set_mode(mode);
  gtk_resize();
}

void
UI_Gtk4::reset(bool power_on)
{
  z80->addCallback(CB_OFFSET, this, 0);
}
