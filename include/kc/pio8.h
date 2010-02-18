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

#ifndef __kc_pio8_h
#define __kc_pio8_h

#include "kc/pio.h"
#include "kc/tape.h"
#include "kc/ports.h"

class PIO8_1 : public PIO, public TapeCallback
{
 private:
  int _led_value[7];
  int _led_value_latch;

 protected:
  void draw_leds(void);
  void tape_bit(int freq);
  void tape_byte(int byte);
  void tape_signal(int val);
  
 public:
  PIO8_1(void);
  virtual ~PIO8_1(void);
  byte_t in(word_t addr);
  void out(word_t addr, byte_t val);
  void change_A(byte_t changed, byte_t val);
  void change_B(byte_t changed, byte_t val);

  byte_t get_led_value(int index);

  /*
   *  InterfaceCircuit
   */
  virtual void reset(bool power_on = false);

  /*
   *  TapeCallback
   */
  virtual void tape_callback(byte_t val);
};

class PIO8_2 : public PIO
{
 public:
  PIO8_2(void);
  virtual ~PIO8_2(void);
  byte_t in(word_t addr);
  void out(word_t addr, byte_t val);
  void change_A(byte_t changed, byte_t val);
  void change_B(byte_t changed, byte_t val);
};

#endif /* __kc_pio8_h */
