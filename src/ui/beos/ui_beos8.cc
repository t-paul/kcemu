/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: ui_beos8.cc,v 1.1 2002/06/09 14:24:34 torsten_paul Exp $
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

#include <Bitmap.h>

#include "kc/z80.h"
#include "kc/pio8.h"

#include "cmd/cmd.h"
#include "cmd/cmdargs.h"

#include "ui/beos/ui_beos8.h"

UI_BeOS8::UI_BeOS8(void)
{
  reset();
}

UI_BeOS8::~UI_BeOS8(void)
{
}

void
UI_BeOS8::allocate_colors(double saturation_fg,
                          double saturation_bg,
                          double brightness_fg,
                          double brightness_bg,
                          double black_level,
                          double white_level)
{
  rgb_to_beos_color(0x00, 0x00, 0x00, &_col[0]); /* black */
  rgb_to_beos_color(0x00, 0x20, 0x00, &_col[1]); /* really dark green */
  rgb_to_beos_color(0x00, 0xe0, 0x00, &_col[2]); /* green */
  rgb_to_beos_color(0xff, 0x8c, 0x00, &_col[3]); /* dark orange */
  rgb_to_beos_color(0x9a, 0xcd, 0x32, &_col[4]); /* led green */

  set_color_map(_col, 5);
}

void
UI_BeOS8::update(bool full_update = false, bool clear_cache = false)
{
  generic_update();
  copy_bitmap(_bitmap, get_real_width(), get_real_height());
  sync();
}

void
UI_BeOS8::memWrite(int addr, char val)
{
}

void
UI_BeOS8::callback(void *data)
{
  z80->addCallback(18000, this, 0);
  update();
}

void
UI_BeOS8::flash(bool enable)
{
}

const char *
UI_BeOS8::get_title(void)
{
  return "BeKC - LC 80";
}

int
UI_BeOS8::get_width(void)
{
	return kcemu_ui_scale * get_real_width();
}

int
UI_BeOS8::get_height(void)
{
	return kcemu_ui_scale * get_real_height();
}

void
UI_BeOS8::reset(bool power_on = false)
{
  z80->addCallback(18000, this, 0);
}

void
UI_BeOS8::processEvents(void)
{
}

void
UI_BeOS8::init(int *argc, char ***argv)
{
  UI_BeOS::init(argc, argv);
}
