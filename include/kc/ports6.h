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

#ifndef __kc_ports6_h
#define __kc_ports6_h

#include "kc/pio.h"
#include "kc/ports.h"

class Ports6 : public PortInterface, public PIOCallbackInterface
{
 private:
  byte_t _val;
  byte_t _led[8];

 protected:
  virtual void update_led_value(byte_t port_val, byte_t pio_val);

 public:
  Ports6(void);
  virtual ~Ports6(void);
  virtual byte_t in(word_t addr);
  virtual void out(word_t addr, byte_t val);

  virtual byte_t get_led_value(int idx);

  /*
   *  PIOCallbackInterface
   */
  virtual int callback_A_in(void);
  virtual int callback_B_in(void);
  virtual void callback_A_out(byte_t val);
  virtual void callback_B_out(byte_t val);
};

#endif /* __kc_ports6_h */
