/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2004 Torsten Paul
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

#ifndef __kc_c80_display_h
#define __kc_c80_display_h

#include "kc/cb.h"
#include "kc/pio.h"

class DisplayC80 : public Callback, public PIOCallbackInterface
{
 private:
  enum {
    C80_NR_OF_LEDS = 8,
    C80_DISPLAY_CB_OFFSET = 2000
  };

  int _idx;
  bool _reset;
  byte_t _led_value[C80_NR_OF_LEDS];

 public:
  DisplayC80(void);
  virtual ~DisplayC80(void);

  virtual byte_t get_led_value(int idx);

  /*
   *  Callback
   */
  virtual void callback(void *data);

  /*
   *  PIOCallbackInterface
   */
  virtual int callback_A_in(void);
  virtual int callback_B_in(void);
  virtual void callback_A_out(byte_t val);
  virtual void callback_B_out(byte_t val);
};

#endif /* __kc_c80_display_h */
