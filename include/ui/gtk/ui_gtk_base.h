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

#ifndef __ui_gtk_ui_gtk_base_h
#define __ui_gtk_ui_gtk_base_h

#include <list>

#include "kc/system.h"

#include "ui/ui.h"
#include "ui/generic/ui_base.h"

using namespace std;

class UI_Color {
private:
    bool _is_bg;
    bool _is_rgb;
    int _r, _g, _b;
    double _hue;
public:
    UI_Color(const UI_Color &c): _is_bg(c._is_bg), _is_rgb(c._is_rgb), _r(c._r), _g(c._g), _b(c._b), _hue(c._hue) {}
    UI_Color(bool bg, int r, int g, int b) : _is_bg(bg), _is_rgb(true), _r(r), _g(g), _b(b), _hue(0) {}
    UI_Color(bool bg, double hue) : _is_bg(bg), _is_rgb(false), _r(0), _g(0), _b(0), _hue(hue) {}
    bool is_bg(void) const { return _is_bg; }
    bool is_rgb(void) const { return _is_rgb; }
    int get_red(void) const { return _r; };
    int get_green(void) const { return _g; };
    int get_blue(void) const { return _b; };
    double get_hue(void) const { return _hue; }
};

class UI_Gtk_Base {
private:
    UI_Base *_generic_ui;

protected:
    list<UI_Color> _colors;

public:
    UI_Gtk_Base(UI_Base *generic_ui);
    virtual ~UI_Gtk_Base(void);

    virtual UI_Base * get_generic_ui(void) const;
    virtual const list<UI_Color> & get_colors(void) const;

    virtual Scanline * get_scanline(void);
    virtual MemAccess * get_mem_access(void);
};

#endif /* __ui_gtk_ui_gtk_base_h */
