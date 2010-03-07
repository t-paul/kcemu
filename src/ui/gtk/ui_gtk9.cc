/*
 *  KCemu -- The emulator for the KC85 homecomputer series and much more.
 *  Copyright (C) 1997-2010 Torsten Paul
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
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "kc/system.h"

#include "ui/generic/ui_9.h"

#include "ui/gtk/ui_gtk9.h"

UI_Gtk9::UI_Gtk9(void) : UI_Gtk_Base(new UI_9())
{
    _colors.push_back(UI_Color(false, 0x00, 0x00, 0x00)); /* black */
    _colors.push_back(UI_Color(false, 240)); /* blue */
    _colors.push_back(UI_Color(false, 120)); /* green */
    _colors.push_back(UI_Color(false, 180)); /* cyan */
    _colors.push_back(UI_Color(false,   0)); /* red */
    _colors.push_back(UI_Color(false, 300)); /* magenta */
    _colors.push_back(UI_Color(false,  60)); /* yellow */
    _colors.push_back(UI_Color(false, 0xd0, 0xd0, 0xd0)); /* white */
    
//  saturation_fg *= 0.80;
//  brightness_fg = 1.0;
//  black_level = 0.40;
//  white_level = 1.0;

    _colors.push_back(UI_Color(true, 0x20, 0x20, 0x20)); /* black */
    _colors.push_back(UI_Color(true, 240)); /* blue */
    _colors.push_back(UI_Color(true, 120)); /* green */
    _colors.push_back(UI_Color(true, 180)); /* cyan */
    _colors.push_back(UI_Color(true,   0)); /* red */
    _colors.push_back(UI_Color(true, 300)); /* magenta */
    _colors.push_back(UI_Color(true,  60)); /* yellow */
    _colors.push_back(UI_Color(true, 0xff, 0xff, 0xff)); /* white */
}

UI_Gtk9::~UI_Gtk9(void)
{
}
