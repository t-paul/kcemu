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

#ifndef __ui_generic_ui_4_h
#define __ui_generic_ui_4_h

#include "kc/system.h"

#include "ui/generic/ui_base.h"
#include "ui/generic/scanline.h"

class UI_4 : public UI_Base
{
 protected:
  byte_t *_pix_cache;
  byte_t *_col_cache;
  byte_t *_scn_cache;

 protected:
  void generic_put_pixels(int x, int y, byte_t pixel, byte_t fg, byte_t bg);

  virtual void generic_update_hires(Scanline *scanline, byte_t *irm, bool clear_cache);
  virtual void generic_update_lores(Scanline *scanline, byte_t *irm, bool clear_cache);

public:
  UI_4(void);
  virtual ~UI_4(void);

  virtual void generic_update(Scanline *scanline, MemAccess *memaccess, bool clear_cache);
};

#endif /* __ui_generic_ui_4_h */
