/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: keyb8.h,v 1.1 2002/03/23 20:04:39 torsten_paul Exp $
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

#ifndef __kc_keyb8_h
#define __kc_keyb8_h

#include "kc/kc.h"
#include "kc/cb.h"
#include "kc/pio.h"
#include "kc/keyb8k.h"
#include "kc/keyboard.h"

class Keyboard8 : public Keyboard, public PIOCallbackInterface
{
 private:
  unsigned int _key;

 protected:
  void init(void);

 public:
  Keyboard8(void);
  virtual ~Keyboard8(void);
  virtual void keyPressed(int keysym, int keycode);
  virtual void keyReleased(int keysym, int keycode);
  virtual void replayString(const char *text);
  
  virtual void callback(void *data);

  /*
   *  InterfaceCircuit
   */
  virtual void reti(void);
  virtual void irqreq(void) {}
  virtual word_t irqack() { return IRQ_NOT_ACK; }
  virtual void reset(bool power_on = false);
  
  /*
   *  PIOCallbackInterface
   */
  virtual void callback_A_in(void);
  virtual void callback_A_out(byte_t val) {}
  virtual void callback_B_in(void);
  virtual void callback_B_out(byte_t val) {}
};

#endif /* __kc_keyb8_h */
