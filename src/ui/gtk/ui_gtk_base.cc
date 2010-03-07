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

#include "ui/gtk/ui_gtk_base.h"

UI_Gtk_Base::UI_Gtk_Base(UI_Base *generic_ui) {
    _generic_ui = generic_ui;
}

UI_Gtk_Base::~UI_Gtk_Base(void) {
    
}

UI_Base *
UI_Gtk_Base::get_generic_ui(void) const {
    return _generic_ui;
}

const list<UI_Color> &
UI_Gtk_Base::get_colors(void) const {
    return _colors;
}

Scanline *
UI_Gtk_Base::get_scanline(void) {
    return NULL;
}

MemAccess *
UI_Gtk_Base::get_mem_access(void) {
    return NULL;
}
