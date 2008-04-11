/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2008 Torsten Paul
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

#include "ui/generic/ui_c80.h"

#include "ui/gtk/ui_gtk_c80.h"

UI_Gtk_C80::UI_Gtk_C80(void) : UI_Gtk_Base(new UI_C80())
{
    _colors.push_back(UI_Color(false, 0x00, 0x00, 0x00)); /* black */
    _colors.push_back(UI_Color(false, 0x00, 0x20, 0x00)); /* really dark green */
    _colors.push_back(UI_Color(false, 0x00, 0xe0, 0x00)); /* green */
    _colors.push_back(UI_Color(false, 0xff, 0x8c, 0x00)); /* dark orange */
    _colors.push_back(UI_Color(false, 0x9a, 0xcd, 0x32)); /* led green */
}

UI_Gtk_C80::~UI_Gtk_C80(void)
{
}
