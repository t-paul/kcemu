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
  sdl_init(get_width(), get_height(), get_title());
  allocate_colors(0.4, 0.5, 0.8, 0.6, 0.1, 0.9);
}

UI_SDL1::~UI_SDL1(void)
{
}

void
UI_SDL1::allocate_colors(double saturation_fg,
                         double saturation_bg,
                         double brightness_fg,
                         double brightness_bg,
                         double black_level,
                         double white_level)
{
  SDL_Color colors[8] = {
      { 0x00, 0x00, 0x00 },
      { 0xd0, 0x00, 0x00 },
      { 0x00, 0xd0, 0x00 },
      { 0xd0, 0xd0, 0x00 },
      { 0x00, 0x00, 0xd0 },
      { 0xd0, 0x00, 0xd0 },
      { 0x00, 0xd0, 0xd0 },
      { 0xd0, 0xd0, 0xd0 }
    };

  sdl_set_colors(colors, 8);
}

void
UI_SDL1::update(bool full_update = false, bool clear_cache = false)
{
  generic_update();
  sdl_update(_bitmap, _dirty, get_real_width(), get_real_height());
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
  return "KC 85/1";
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

void
UI_SDL1::reset(bool power_on = false)
{
  z80->addCallback(35000, this, 0);
}
