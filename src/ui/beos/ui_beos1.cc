/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: ui_beos1.cc,v 1.1 2002/06/09 14:24:34 torsten_paul Exp $
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

#include "kc/z80.h"
#include "kc/memory.h"

#include "cmd/cmd.h"
#include "cmd/cmdargs.h"

#include "ui/font1.h"

#include "ui/beos/ui_beos1.h"

UI_BeOS1::UI_BeOS1(void)
{
  reset();
}

UI_BeOS1::~UI_BeOS1(void)
{
}

void
UI_BeOS1::allocate_colors(double saturation_fg,
                          double saturation_bg,
                          double brightness_fg,
                          double brightness_bg,
                          double black_level,
                          double white_level)
{
  rgb_to_beos_color(0x00, 0x00, 0x00, &_col[0]);
  rgb_to_beos_color(0xd0, 0x00, 0x00, &_col[1]);
  rgb_to_beos_color(0x00, 0xd0, 0x00, &_col[2]);
  rgb_to_beos_color(0xd0, 0xd0, 0x00, &_col[3]);
  rgb_to_beos_color(0x00, 0x00, 0xd0, &_col[4]);
  rgb_to_beos_color(0xd0, 0x00, 0xd0, &_col[5]);
  rgb_to_beos_color(0x00, 0xd0, 0xd0, &_col[6]);
  rgb_to_beos_color(0xd0, 0xd0, 0xd0, &_col[7]);

  set_color_map(_col, 8);
}

void
UI_BeOS1::update(bool full_update = false, bool clear_cache = false)
{
  generic_update();
  copy_bitmap(_bitmap, get_real_width(), get_real_height());
  sync();
}

void
UI_BeOS1::memWrite(int addr, char val)
{
}

void
UI_BeOS1::callback(void *data)
{
  z80->addCallback(35000, this, 0);
  update();
}

void
UI_BeOS1::flash(bool enable)
{
}

const char *
UI_BeOS1::get_title(void)
{
	return "BeKC - KC 85/1";
}

int
UI_BeOS1::get_width(void)
{
	return kcemu_ui_scale * get_real_width();
}

int
UI_BeOS1::get_height(void)
{
	return kcemu_ui_scale * get_real_height();
}

void
UI_BeOS1::reset(bool power_on = false)
{
  z80->addCallback(35000, this, 0);
}

void
UI_BeOS1::processEvents(void)
{
}

void
UI_BeOS1::init(int *argc, char ***argv)
{
  UI_BeOS::init(argc, argv);
}

