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

#ifndef __ui_generic_ui_9_h
#define __ui_generic_ui_9_h

#include "kc/system.h"

#include "ui/generic/ui_base.h"

class UI_9 : public UI_Base
{
 public:
  enum {
    UI_GENERIC_MODE_LORES = 0,
    UI_GENERIC_MODE_HIRES = 1,
  };

  int     _mode;
  int     _width;
  int     _height;

 protected:
  byte_t *_pix_cache;
  byte_t *_col_cache;
  int     _max_ptr;

 protected:
  void init(void);
  void dispose(void);
  void set_pixel(int x, int y, int color);
  void set_char(int x, int y, int c, int fg, int bg, int width, int lines);
  void generic_update_border(int border, int lines);
  void generic_update_graphic_2(bool clear_cache);
  void generic_update_graphic_3(bool clear_cache);
  void generic_update_graphic_5(bool clear_cache);
  void generic_update_text(int width, int height, int lines, bool clear_cache);
  
 public:
  UI_9(void);
  virtual ~UI_9(void);

  virtual int  generic_get_mode(void);
  virtual void generic_set_mode(int mode);

  virtual void generic_signal_v_retrace(bool value);

  virtual void generic_update(Scanline *scanline, MemAccess *memaccess, bool clear_cache);
};

#endif /* __ui_generic_ui_9_h */
