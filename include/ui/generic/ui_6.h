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

#ifndef __ui_generic_ui_6_h
#define __ui_generic_ui_6_h

#include "kc/system.h"

#include "ui/generic/ui_led.h"

class UI_6 : public UI_LED
{
 private:
  enum {
    HISTORY_LEN = 4,
    NR_OF_LEDS  = 8,
  };

  int _idx;
  byte_t _data[NR_OF_LEDS][HISTORY_LEN];

 public:
  UI_6(void);
  virtual ~UI_6(void);

  virtual void generic_update(bool clear_cache = false);

  virtual int  generic_get_mode(void);
  virtual void generic_set_mode(int mode);
};

#endif /* __ui_generic_ui_6_h */
