/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2002 Torsten Paul
 *
 *  $Id: ui_gtk1.cc,v 1.11 2002/10/31 01:38:12 torsten_paul Exp $
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

#include "ui/generic/ui_1.h"

#include "ui/gtk/ui_gtk1.h"

UI_Gtk1::UI_Gtk1(void) : UI_Gtk_Base(new UI_1())
{
    _colors.push_back(UI_Color(false, 0x00, 0x00, 0x00));
    _colors.push_back(UI_Color(false, 0xd0, 0x00, 0x00));
    _colors.push_back(UI_Color(false, 0x00, 0xd0, 0x00));
    _colors.push_back(UI_Color(false, 0xd0, 0xd0, 0x00));
    _colors.push_back(UI_Color(false, 0x00, 0x00, 0xd0));
    _colors.push_back(UI_Color(false, 0xd0, 0x00, 0xd0));
    _colors.push_back(UI_Color(false, 0x00, 0xd0, 0xd0));
    _colors.push_back(UI_Color(false, 0xd0, 0xd0, 0xd0));

    _colors.push_back(UI_Color(false, 0xaa, 0xaa, 0x88));
    _colors.push_back(UI_Color(false, 0xff, 0x00, 0x88));
    _colors.push_back(UI_Color(false, 0xaa, 0xff, 0x44));
    _colors.push_back(UI_Color(false, 0xff, 0xaa, 0x44));
    _colors.push_back(UI_Color(false, 0xaa, 0xcc, 0xff));
    _colors.push_back(UI_Color(false, 0xaa, 0x44, 0xff));
    _colors.push_back(UI_Color(false, 0xaa, 0xff, 0xff));
    _colors.push_back(UI_Color(false, 0xcc, 0xcc, 0xcc));
}

UI_Gtk1::~UI_Gtk1(void)
{
}
