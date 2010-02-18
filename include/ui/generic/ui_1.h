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

#ifndef __ui_generic_ui_1_h
#define __ui_generic_ui_1_h

#include "kc/system.h"

#include "ui/generic/ui_base.h"

class UI_1 : public UI_Base
{
 protected:
  byte_t *_pix_cache;
  byte_t *_col_cache;

 protected:
  inline void generic_put_pixels(byte_t *ptr, byte_t val, word_t color);

  void generic_update_20(int width, int height, int fchg, byte_t flash, bool clear_cache);
  void generic_update_24(int width, int height, int fchg, byte_t flash, bool clear_cache);

  void generic_set_border_20(int width, int height, byte_t border);
  void generic_set_border_24(int width, int height, byte_t border);

 public:
  UI_1(void);
  virtual ~UI_1(void);

  virtual void generic_update(Scanline *scanline, MemAccess *memaccess, bool clear_cache);
};

#endif /* __ui_generic_ui_1_h */
