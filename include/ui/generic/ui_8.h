/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: ui_8.h,v 1.1 2002/06/09 14:24:32 torsten_paul Exp $
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

#ifndef __ui_generic_ui_8_h
#define __ui_generic_ui_8_h

#include "ui/ui.h"

#include "ui/generic/tape.h"
#include "ui/generic/module.h"

class UI_8 : public UI
{
 protected:
  byte_t *_bitmap;

 protected:
  virtual void generic_put_pixel(int x, int y, byte_t col);
  virtual void generic_draw_led(int x, int y, byte_t col);
  virtual void generic_draw_hline(int x, int y, byte_t col);
  virtual void generic_draw_vline(int x, int y, byte_t col);
  virtual void generic_draw_point(int x, int y, byte_t col);
  virtual void generic_draw_digit(int x, int y, int index, byte_t led_value);

 public:
  UI_8(void);
  virtual ~UI_8(void);

  virtual int get_real_width(void);
  virtual int get_real_height(void);

  virtual void generic_update(void);

  /*
   *  interface handling
   */
  virtual UI_ModuleInterface * getModuleInterface(void);
  virtual TapeInterface  * getTapeInterface(void);
  virtual DebugInterface * getDebugInterface(void);
};

#endif /* __ui_generic_ui_8_h */
