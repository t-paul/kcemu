/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: ui_beos4.cc,v 1.1 2002/06/09 14:24:34 torsten_paul Exp $
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

#include <View.h>
#include <Window.h>

#include "ui/hsv2rgb.h"
#include "ui/beos/ui_beos4.h"

#include "kc/z80.h"
#include "kc/memory.h"

#include "cmd/cmd.h"
#include "cmd/cmdargs.h"

UI_BeOS4::UI_BeOS4(void)
{
	reset();
}

UI_BeOS4::~UI_BeOS4(void)
{
}

void
UI_BeOS4::allocate_colors(double saturation_fg,
                          double saturation_bg,
                          double brightness_fg,
                          double brightness_bg,
                          double black_level,
                          double white_level)
{
  hsv_to_beos_color(  0,             0,   black_level, &_col[ 0]); /* black */
  hsv_to_beos_color(240, saturation_fg, brightness_fg, &_col[ 1]); /* blue */
  hsv_to_beos_color(  0, saturation_fg, brightness_fg, &_col[ 2]); /* red */
  hsv_to_beos_color(300, saturation_fg, brightness_fg, &_col[ 3]); /* magenta */
  hsv_to_beos_color(120, saturation_fg, brightness_fg, &_col[ 4]); /* green */
  hsv_to_beos_color(180, saturation_fg, brightness_fg, &_col[ 5]); /* cyan */
  hsv_to_beos_color( 60, saturation_fg, brightness_fg, &_col[ 6]); /* yellow */
  hsv_to_beos_color(  0,             0,   white_level, &_col[ 7]); /* white */

  hsv_to_beos_color(  0,             0,   black_level, &_col[ 8]); /* black */
  hsv_to_beos_color(270, saturation_fg, brightness_fg, &_col[ 9]); /* blue + 30° */
  hsv_to_beos_color( 30, saturation_fg, brightness_fg, &_col[10]); /* red + 30° */
  hsv_to_beos_color(330, saturation_fg, brightness_fg, &_col[11]); /* magenta + 30° */
  hsv_to_beos_color(150, saturation_fg, brightness_fg, &_col[12]); /* green + 30° */
  hsv_to_beos_color(210, saturation_fg, brightness_fg, &_col[13]); /* cyan + 30° */
  hsv_to_beos_color( 90, saturation_fg, brightness_fg, &_col[14]); /* yellow + 30° */
  hsv_to_beos_color(  0,             0,   white_level, &_col[15]); /* white */

  hsv_to_beos_color(  0,             0,   black_level, &_col[16]); /* black */
  hsv_to_beos_color(240, saturation_bg, brightness_bg, &_col[17]); /* blue */
  hsv_to_beos_color(  0, saturation_bg, brightness_bg, &_col[18]); /* red */
  hsv_to_beos_color(300, saturation_bg, brightness_bg, &_col[19]); /* magenta */
  hsv_to_beos_color(120, saturation_bg, brightness_bg, &_col[20]); /* green */
  hsv_to_beos_color(180, saturation_bg, brightness_bg, &_col[21]); /* cyan */
  hsv_to_beos_color( 60, saturation_bg, brightness_bg, &_col[22]); /* yellow */
  hsv_to_beos_color(  0,             0, brightness_bg, &_col[23]); /* white */

  set_color_map(_col, 24);
}

void
UI_BeOS4::update(bool full_update = false, bool clear_cache = false)
{
  generic_update();
  copy_bitmap(_bitmap, get_real_width(), get_real_height());
  sync();
}

void
UI_BeOS4::memWrite(int addr, char val)
{
}

void
UI_BeOS4::callback(void *data)
{
  z80->addCallback(35000, this, 0);
  update();
}

void
UI_BeOS4::flash(bool enable)
{
}

const char *
UI_BeOS4::get_title(void)
{
	return "BeKC - KC 85/4";
}

int
UI_BeOS4::get_width(void)
{
	return kcemu_ui_scale * get_real_width();
}

int
UI_BeOS4::get_height(void)
{
	return kcemu_ui_scale * get_real_height();
}

void
UI_BeOS4::reset(bool power_on = false)
{
  z80->addCallback(35000, this, 0);
}

void
UI_BeOS4::processEvents(void)
{
}

void
UI_BeOS4::init(int *argc, char ***argv)
{
  UI_BeOS::init(argc, argv);
}
