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

#ifndef __kc_pio6_h
#define __kc_pio6_h

#include "kc/pio.h"
#include "kc/ports.h"

class PIO6_1 : public PIO
{
 private:
  byte_t _led_value;

 public:
  PIO6_1(void);
  virtual ~PIO6_1(void);
  byte_t in(word_t addr);
  void out(word_t addr, byte_t val);
  void change_A(byte_t changed, byte_t val);
  void change_B(byte_t changed, byte_t val);

  virtual byte_t get_led_value(void);
};

class PIO6_2 : public PIO
{
 public:
  PIO6_2(void);
  virtual ~PIO6_2(void);
  byte_t in(word_t addr);
  void out(word_t addr, byte_t val);
  void change_A(byte_t changed, byte_t val);
  void change_B(byte_t changed, byte_t val);
};

#endif /* __kc_pio6_h */
