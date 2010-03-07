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

#ifndef __kc_muglerpc_keyboard_h
#define __kc_muglerpc_keyboard_h

#include "kc/kc.h"
#include "kc/cb.h"
#include "kc/pio.h"
#include "kc/keyboard.h"

class KeyboardMuglerPC : public Keyboard, public PIOCallbackInterface
{
 public:
  enum {
    MAX_KEYS = 8,
  };
  
 private:
  struct _keybuf
  {
    int sym;
    int code;
  } _keybuf[MAX_KEYS];

 protected:
  void init(void);
  int  decode_key(int keysym, bool press);

 public:
  KeyboardMuglerPC(void);
  virtual ~KeyboardMuglerPC(void);
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
  virtual int callback_A_in(void);
  virtual int callback_B_in(void);
  virtual void callback_A_out(byte_t val);
  virtual void callback_B_out(byte_t val);
};

#endif /* __kc_muglerpc_keyboard_h */
