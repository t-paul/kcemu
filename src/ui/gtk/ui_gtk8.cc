/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2002 Torsten Paul
 *
 *  $Id: ui_gtk8.cc,v 1.3 2002/10/31 01:38:12 torsten_paul Exp $
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

#include "ui/generic/ui_8.h"

#include "ui/gtk/ui_gtk8.h"

UI_Gtk8::UI_Gtk8(void) : UI_Gtk_Base(new UI_8())
{
    _colors.push_back(UI_Color(false, 0x00, 0x00, 0x00)); /* black */
    _colors.push_back(UI_Color(false, 0x00, 0x20, 0x00)); /* really dark green */
    _colors.push_back(UI_Color(false, 0x00, 0xe0, 0x00)); /* green */
    _colors.push_back(UI_Color(false, 0xff, 0x8c, 0x00)); /* dark orange */
    _colors.push_back(UI_Color(false, 0x9a, 0xcd, 0x32)); /* led green */
}

UI_Gtk8::~UI_Gtk8(void)
{
}
