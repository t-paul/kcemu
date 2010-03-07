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
