/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2002 Torsten Paul
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

#ifndef __kc_pio0_h
#define __kc_pio0_h

#include "kc/pio.h"
#include "kc/tape.h"

class PIO0 : public PIO, public TapeCallback
{
 public:
  PIO0(void);
  virtual ~PIO0(void);
  byte_t in(word_t addr);
  void out(word_t addr, byte_t val);
  void change_A(byte_t changed, byte_t val);
  void change_B(byte_t changed, byte_t val);

  /*
   *  TapeCallback
   */
  virtual void tape_callback(byte_t val);
};

#endif /* __kc_pio0_h */
