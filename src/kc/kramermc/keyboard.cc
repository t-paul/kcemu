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

#include <iostream>
#include <iomanip>

#include "kc/system.h"

#include "kc/z80.h"
#include "kc/keys.h"
#include "kc/kramermc/keydef.h"
#include "kc/kramermc/keyboard.h"

#include "libdbg/dbg.h"

KeyboardKramerMC::KeyboardKramerMC(void)
{
  init();
  z80->register_ic(this);
}


KeyboardKramerMC::~KeyboardKramerMC(void)
{
  z80->unregister_ic(this);
}

void
KeyboardKramerMC::init(void)
{
  _row = 0;
  _keybuf.code = -1;
}

int
KeyboardKramerMC::decode_key(int keysym, bool press)
{
  return __keys[keysym];
}

void
KeyboardKramerMC::keyPressed(int keysym, int keycode)
{
  int k = decode_key(keysym, true);
  if (k == -1)
    return;

  int mod = (k & 0xff00) >> 8;
  int key = k & 0xff;

  if (_keybuf.code != -1)
    if ((_keybuf.sym1 != mod) || (_keybuf.sym2 != 0))
      return;

  if (mod == 0)
    {
      _keybuf.sym1 = key;
      _keybuf.sym2 = 0;
    }
  else
    {
      _keybuf.sym1 = mod;
      _keybuf.sym2 = key;
      z80->addCallback(MODIFIER_PRESS_DELAY, this, (void *)0);
    }
  _keybuf.code = keycode;

  DBG(2, form("KCemu/keyboard/kramermc/key_press",
	      "##### Keyboard: keyPressed  [%03x/%02x] {%02x:%02x:%02x}\n",
	      keysym,
	      k,
	      _keybuf.sym1,
	      _keybuf.sym2,
	      _keybuf.code));

}

void
KeyboardKramerMC::keyReleased(int keysym, int keycode)
{
  _keybuf.code = -1;                                                                                
  DBG(2, form("KCemu/keyboard/kramermc/key_press",
	      "##### Keyboard: keyPressed  [%03x/%02x] {%02x:%02x:%02x}\n",
	      keysym,
	      keycode,
	      _keybuf.sym1,
	      _keybuf.sym2,
	      _keybuf.code));
}

void
KeyboardKramerMC::replayString(const char *text)
{
}

void
KeyboardKramerMC::callback(void *data)
{
  long val = (long)data;

  if (val == 0)
    {
      /*
       *  remove possible shift or control state
       */
      _keybuf.sym1 = 0;
      z80->addCallback(MODIFIER_RELEASE_DELAY, this, (void *)1);
      //printf("{remove shift}\n");
    }
  else
    {
      _keybuf.sym1 = _keybuf.sym2;
      _keybuf.sym2 = 0;
      //printf("{2nd key}\n");
    }
}

int
KeyboardKramerMC::callback_A_in(void)
{
  return -1;
}

void
KeyboardKramerMC::callback_A_out(byte_t val)
{
  _row = ((val >> 1) & 0x07) + 1;
}

int
KeyboardKramerMC::callback_B_in(void)
{
  int col = 0;
  static int old_val = -1;

  int v = _keybuf.sym1;

  //if (v != old_val)
    //printf("[%02x - %3d]\n", v, v);
  old_val = v;

  if (_keybuf.code != -1)
    if (_keybuf.sym1 > 0)
      {
	if (_row == (v & 0x0f))
	  col |= 1 << (((v >> 4) & 0x0f) - 1);
      }
  
  return col ^ 0xff;
}

void
KeyboardKramerMC::callback_B_out(byte_t val)
{
}

void
KeyboardKramerMC::reset(bool power_on)
{
  init();
}

void
KeyboardKramerMC::reti(void)
{
}
