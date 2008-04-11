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

#ifndef __ui_generic_ui_base_h
#define __ui_generic_ui_base_h

class Scanline;
class MemAccess;

class UI_Base {
private:
    int     _width;
    int     _height;
    
protected:
    byte_t *_bitmap;
    byte_t *_dirty;
    int     _dirty_size;
    
    void create_buffer(int buffer_size);
    void set_real_screen_size(int width, int height);
    
public:
    UI_Base(void);
    virtual ~UI_Base(void);
    
    int  get_real_width(void);
    int  get_real_height(void);
    
    virtual byte_t * get_buffer(void);
    virtual byte_t * get_dirty_buffer(void);
    virtual int get_dirty_buffer_size(void);
    
    virtual int  generic_get_mode(void);
    virtual void generic_set_mode(int mode);

    virtual void generic_signal_v_retrace(bool value);

    virtual void generic_update(Scanline *scanline, MemAccess *memaccess, bool clear_cache) = 0;
};

#endif /* __ui_generic_ui_base_h */
