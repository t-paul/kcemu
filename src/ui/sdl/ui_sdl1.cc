/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: ui_sdl1.cc,v 1.2 2002/10/31 01:16:25 torsten_paul Exp $
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

#include "ui/sdl/ui_sdl1.h"

UI_SDL1::UI_SDL1(void)
{
  reset();
  z80->register_ic(this);
}

UI_SDL1::~UI_SDL1(void)
{
  z80->unregister_ic(this);
}

void
UI_SDL1::allocate_colors(double saturation_fg,
                         double saturation_bg,
                         double brightness_fg,
                         double brightness_bg,
                         double black_level,
                         double white_level)
{
  SDL_Color col[8];

  hsv_to_sdl_color(  0,             0,   black_level, &col[0]); /* black */
  hsv_to_sdl_color(  0, saturation_fg, brightness_fg, &col[1]); /* red */
  hsv_to_sdl_color(120, saturation_fg, brightness_fg, &col[2]); /* green */
  hsv_to_sdl_color( 60, saturation_fg, brightness_fg, &col[3]); /* yellow */
  hsv_to_sdl_color(240, saturation_fg, brightness_fg, &col[4]); /* blue */
  hsv_to_sdl_color(300, saturation_fg, brightness_fg, &col[5]); /* magenta */
  hsv_to_sdl_color(180, saturation_fg, brightness_fg, &col[6]); /* cyan */
  hsv_to_sdl_color(  0,             0,   white_level, &col[7]); /* white */

  sdl_set_colors(col, 8);
}

void
UI_SDL1::update(bool full_update, bool clear_cache)
{
  generic_update(clear_cache);
  sdl_update(_bitmap, _dirty, get_real_width(), get_real_height(), clear_cache);
  sdl_sync();
}

void
UI_SDL1::callback(void *data)
{
  z80->addCallback(35000, this, 0);
  update();
  sdl_process_events();
}

void
UI_SDL1::flash(bool enable)
{
}

const char *
UI_SDL1::get_title(void)
{
  if (get_kc_type() == KC_TYPE_87)
    return _("KC 87 Emulator");

  return _("KC 85/1 Emulator");
}

int
UI_SDL1::get_width(void)
{
  return kcemu_ui_scale * get_real_width();;
}

int
UI_SDL1::get_height(void)
{
  return kcemu_ui_scale * get_real_height();
}

int
UI_SDL1::get_mode(void)
{
  return generic_get_mode();
}

void
UI_SDL1::set_mode(int mode)
{
  if (generic_get_mode() != mode)
    {
      generic_set_mode(mode);
      sdl_resize();
    }
}

void
UI_SDL1::reset(bool power_on)
{
  z80->addCallback(35000, this, 0);
}
