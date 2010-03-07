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

#ifndef __ui_ui_gtk3_h
#define __ui_ui_gtk3_h

#include "ui/gtk/ui_gtk_base.h"

#include "ui/generic/scanline.h"
#include "ui/generic/memaccess.h"

class UI_Gtk3 : public UI_Gtk_Base {
private:
    Scanline _scanline;
    MemAccess _memaccess;
    
public:
    UI_Gtk3(void);
    virtual ~UI_Gtk3(void);

    virtual Scanline * get_scanline(void);
    virtual MemAccess * get_mem_access(void);
};

#endif /* __ui_ui_gtk3_h */
