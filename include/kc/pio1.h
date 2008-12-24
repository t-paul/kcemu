/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: pio1.h,v 1.4 2002/10/31 01:46:33 torsten_paul Exp $
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

#ifndef __kc_pio1_h
#define __kc_pio1_h

#include "kc/pio.h"
#include "kc/tape.h"
#include "kc/ports.h"

class PIO1_1 : public PIO, public TapeCallback
{
 private:
  bool _color_mode;
  bool _charset_mode;
 
 public:
  PIO1_1(void);
  virtual ~PIO1_1(void);
  byte_t in(word_t addr);
  void out(word_t addr, byte_t val);
  byte_t pio_in(word_t addr);
  void pio_out(word_t addr, byte_t val);
  void change_A(byte_t changed, byte_t val);
  void change_B(byte_t changed, byte_t val);

  byte_t get_color_mode(void);
  byte_t get_border_color(void);
  byte_t get_border_color_16(void);
  byte_t get_line_mode(void); // returns true if in 20 line mode

  /*
   *  TapeCallback
   */
  virtual void tape_callback(byte_t val);
};

class PIO1_2 : public PIO
{
 public:
  PIO1_2(void);
  virtual ~PIO1_2(void);
  byte_t in(word_t addr);
  void out(word_t addr, byte_t val);
  void change_A(byte_t changed, byte_t val);
  void change_B(byte_t changed, byte_t val);
};

#endif /* __kc_pio1_h */
