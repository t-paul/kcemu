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

#include "ui/sdl/ui_sdl9.h"

UI_SDL9::UI_SDL9(void)
{
  reset();
  z80->register_ic(this);
}

UI_SDL9::~UI_SDL9(void)
{
  z80->unregister_ic(this);
}

void
UI_SDL9::allocate_colors(double saturation_fg,
                         double saturation_bg,
                         double brightness_fg,
                         double brightness_bg,
                         double black_level,
                         double white_level)
{
  SDL_Color col[16];

  hsv_to_sdl_color(  0,             0,   black_level, &col[ 0]); /* black */
  hsv_to_sdl_color(270, saturation_fg, brightness_fg, &col[ 1]); /* blue + 30° */
  hsv_to_sdl_color(150, saturation_fg, brightness_fg, &col[ 4]); /* green + 30° */
  hsv_to_sdl_color(210, saturation_fg, brightness_fg, &col[ 5]); /* cyan + 30° */
  hsv_to_sdl_color( 30, saturation_fg, brightness_fg, &col[ 2]); /* red + 30° */
  hsv_to_sdl_color(330, saturation_fg, brightness_fg, &col[ 3]); /* magenta + 30° */
  hsv_to_sdl_color( 90, saturation_fg, brightness_fg, &col[ 6]); /* yellow + 30° */
  hsv_to_sdl_color(  0,             0,   white_level, &col[ 7]); /* white */

  hsv_to_sdl_color(  0,             0,   black_level, &col[ 8]); /* black */
  hsv_to_sdl_color(240, saturation_fg, brightness_fg, &col[ 9]); /* blue */
  hsv_to_sdl_color(120, saturation_fg, brightness_fg, &col[12]); /* green */
  hsv_to_sdl_color(180, saturation_fg, brightness_fg, &col[13]); /* cyan */
  hsv_to_sdl_color(  0, saturation_fg, brightness_fg, &col[10]); /* red */
  hsv_to_sdl_color(300, saturation_fg, brightness_fg, &col[11]); /* magenta */
  hsv_to_sdl_color( 60, saturation_fg, brightness_fg, &col[14]); /* yellow */
  hsv_to_sdl_color(  0,             0,   white_level, &col[15]); /* white */

  sdl_set_colors(col, 16);
}

void
UI_SDL9::update(bool full_update, bool clear_cache)
{
  generic_update(clear_cache);
  sdl_update(_bitmap, _dirty, get_real_width(), get_real_height(), clear_cache);
  sdl_sync();
}

void
UI_SDL9::callback(void *data)
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
      z80->addCallback(35000, this, 0);
      generic_signal_v_retrace(true);
    }
}

void
UI_SDL9::flash(bool enable)
{
}

const char *
UI_SDL9::get_title(void)
{
  return _("A5105 Emulator");
}

int
UI_SDL9::get_width(void)
{
  return kcemu_ui_scale * get_real_width();;
}

int
UI_SDL9::get_height(void)
{
  return kcemu_ui_scale * get_real_height();
}

int
UI_SDL9::get_mode(void)
{
  return generic_get_mode();
}

void
UI_SDL9::set_mode(int mode)
{
  if (generic_get_mode() != mode)
    {
      generic_set_mode(mode);
      sdl_resize();
    }
}

void
UI_SDL9::reset(bool power_on)
{
  z80->addCallback(35000, this, 0);
}
