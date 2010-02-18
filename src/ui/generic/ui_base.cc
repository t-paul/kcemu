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

#include <stdlib.h>

#include "kc/system.h"

#include "ui/generic/ui_base.h"

#include "libdbg/dbg.h"

UI_Base::UI_Base(void) {
    _width = 0;
    _height = 0;
    _dirty_size  = 0;
    
    _dirty = 0;
    _bitmap = 0;
}

UI_Base::~UI_Base(void) {
}

void
UI_Base::create_buffer(int buffer_size) {
  _bitmap = new byte_t[buffer_size];
  memset(_bitmap, 0, buffer_size);
}

void
UI_Base::set_real_screen_size(int width, int height) {
    _width = width;
    _height = height;
}

byte_t *
UI_Base::get_buffer(void) {
    return _bitmap;
}

byte_t *
UI_Base::get_dirty_buffer(void) {
    return _dirty;
}

int
UI_Base::get_dirty_buffer_size(void) {
    return _dirty_size;
}

int
UI_Base::get_real_width(void) {
    if (_width == 0) {
        DBG(0, form("KCemu/internal_error",
                "KCemu: call to UI_Base::get_real_width() while width is still 0!\n"));
        abort();
    }
    return _width;
}

int
UI_Base::get_real_height(void) {
    if (_height == 0) {
        DBG(0, form("KCemu/internal_error",
                "KCemu: call to UI_Base::get_real_height() while height is still 0!\n"));
        abort();
    }
    return _height;
}

void
UI_Base::generic_signal_v_retrace(bool value) {
}

int
UI_Base::generic_get_mode(void) {
    return 0;
}

void
UI_Base::generic_set_mode(int mode) {
}
