/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2003 Torsten Paul
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

#include "kc/z80.h"

#include "ui/sdl/ui_sdl0.h"

UI_SDL0::UI_SDL0(void)
{
  reset();
  z80->register_ic(this);
}

UI_SDL0::~UI_SDL0(void)
{
  z80->unregister_ic(this);
}

void
UI_SDL0::allocate_colors(double saturation_fg,
                         double saturation_bg,
                         double brightness_fg,
                         double brightness_bg,
                         double black_level,
                         double white_level)
{
  SDL_Color col[2];

  hsv_to_sdl_color(  0,             0,   black_level, &col[0]); /* black */
  hsv_to_sdl_color(  0,             0,   white_level, &col[1]); /* white */

  sdl_set_colors(col, 2);
}

void
UI_SDL0::update(bool full_update, bool clear_cache)
{
  generic_update(clear_cache);
  sdl_update(_bitmap, _dirty, get_real_width(), get_real_height(), clear_cache);
  sdl_sync();
}

void
UI_SDL0::callback(void *data)
{
  if (data == (void *)1)
    {
      generic_signal_v_retrace(false);
    }
  else
    {
      update();
      sdl_process_events();
      z80->addCallback(1000, this, (void *)1);
      z80->addCallback(40000, this, (void *)0);
      generic_signal_v_retrace(true);
    }
}

void
UI_SDL0::flash(bool enable)
{
}

const char *
UI_SDL0::get_title(void)
{
  return _("Z1013 Emulator");
}

int
UI_SDL0::get_width(void)
{
  return kcemu_ui_scale * get_real_width();;
}

int
UI_SDL0::get_height(void)
{
  return kcemu_ui_scale * get_real_height();
}

int
UI_SDL0::get_mode(void)
{
  return generic_get_mode();
}

void
UI_SDL0::set_mode(int mode)
{
  if (generic_get_mode() != mode)
    {
      generic_set_mode(mode);
      sdl_resize();
    }
}

void
UI_SDL0::reset(bool power_on)
{
  z80->addCallback(35000, this, 0);
}
