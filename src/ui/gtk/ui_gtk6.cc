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

#include "ui/gtk/ui_gtk6.h"

#include "libdbg/dbg.h"

UI_Gtk6::UI_Gtk6(void) : UI_Gtk()
{
  reset();
  z80->register_ic(this);
}

UI_Gtk6::~UI_Gtk6(void)
{
  z80->unregister_ic(this); 
}

void
UI_Gtk6::callback(void * /* data */)
{
  z80->addCallback(POLY880_CB_OFFSET, this, 0);
  update();
}

int
UI_Gtk6::get_width(void)
{
  return kcemu_ui_scale * get_real_width();
}

int
UI_Gtk6::get_height(void)
{
  return kcemu_ui_scale * get_real_height();
}

const char *
UI_Gtk6::get_title(void)
{
  return _("Poly880 Emulator");
}

void
UI_Gtk6::allocate_colors(double saturation_fg,
			 double saturation_bg,
			 double brightness_fg,
			 double brightness_bg,
			 double black_level,
			 double white_level)
{
    int a;
    char *color_names[] = {
      "#000000", /* black */
      "#002000", /* really dark green */
      "#00e000", /* green */
      "#ff8c00", /* dark orange */
      "#9acd32", /* led green */
      0,
    };

    _colormap = gdk_colormap_get_system();
    for (a = 0;color_names[a];a++) {
	gdk_color_parse(color_names[a], &_col[a]);
	gdk_color_alloc(_colormap, &_col[a]);
    }
}

void
UI_Gtk6::init(void)
{
}

void
UI_Gtk6::update(bool full_update, bool clear_cache)
{
  generic_update(clear_cache);
  gtk_update(_bitmap, get_dirty_buffer(), get_dirty_buffer_size(),
	     get_real_width(), get_real_height(), full_update);
  processEvents();
  gtk_sync();
}

void
UI_Gtk6::flash(bool enable)
{
}

void
UI_Gtk6::memory_read(word_t addr)
{
}

void
UI_Gtk6::memory_write(word_t addr)
{
}

int
UI_Gtk6::get_mode(void)
{
  return generic_get_mode();
}

void
UI_Gtk6::set_mode(int mode)
{
  generic_set_mode(mode);
  gtk_resize();
}

void
UI_Gtk6::reset(bool power_on)
{
  z80->addCallback(POLY880_CB_OFFSET, this, 0);
}
