/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: ui_sdl3.h,v 1.1 2002/06/09 14:24:33 torsten_paul Exp $
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

#ifndef __ui_sdl_ui_sdl3_h
#define __ui_sdl_ui_sdl3_h

#include "ui/sdl/ui_sdl.h"

#include "ui/generic/ui_3.h"

class UI_SDL3 : public UI_SDL, public UI_3
{
 public:
  UI_SDL3(void);
  virtual ~UI_SDL3(void);

  virtual void update(bool full_update = false, bool clear_cache = false);
  virtual void memWrite(int addr, char val);
  virtual void callback(void *data);
  virtual void flash(bool enable);

  virtual void allocate_colors(double saturation_fg,
                               double saturation_bg,
                               double brightness_fg,
                               double brightness_bg,
                               double black_level,
                               double white_level);

  virtual const char * get_title(void);
  virtual int get_width(void);
  virtual int get_height(void);

  virtual void reset(bool power_on = false);

  void processEvents();
  void init(int *, char ***);
};

#endif /* __ui_sdl_ui_sdl3_h */
