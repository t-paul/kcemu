/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
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

#ifndef __ui_generic_ui_0_h
#define __ui_generic_ui_0_h

#include "kc/system.h"

class UI_0
{
 public:
  enum {
    UI_GENERIC_MODE_Z1013_32x32 = 0,
    UI_GENERIC_MODE_Z1013_64x16 = 1,
    UI_GENERIC_MODE_GDC         = 2,
  };

 protected:
  byte_t *_font;
  byte_t *_bitmap;
  byte_t *_dirty;
  byte_t *_pix_cache;
  byte_t *_col_cache;
  int     _dirty_size;

  int     _mode;
  int     _width;
  int     _height;

 protected:
  void init(void);
  void dispose(void);
  inline void generic_put_pixels(byte_t *ptr, byte_t val, word_t color);

 public:
  UI_0(void);
  virtual ~UI_0(void);

  virtual int get_real_width(void);
  virtual int get_real_height(void);

  virtual byte_t * get_dirty_buffer(void);
  virtual int get_dirty_buffer_size(void);

  virtual void generic_signal_v_retrace(bool value);
  virtual void generic_update(bool clear_cache = false);
  virtual void generic_update_gdc(byte_t *font, bool clear_cache);
  virtual void generic_update_32x32(byte_t *font, bool clear_cache);
  virtual void generic_update_64x16(byte_t *font, bool clear_cache);

  virtual int  generic_get_mode(void);
  virtual void generic_set_mode(int mode);
};

#endif /* __ui_generic_ui_0_h */