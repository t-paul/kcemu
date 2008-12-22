/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2008 Torsten Paul
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

#include <iostream>
#include <iomanip>

#include "kc/system.h"

#include "kc/z80.h"
#include "kc/keys.h"
#include "kc/ac1/keydef.h"
#include "kc/ac1/keyboard.h"

#include "cmd/cmd.h"

#include "libdbg/dbg.h"

KeyboardAC1::KeyboardAC1(void)
{
  init();
  z80->register_ic(this);
}


KeyboardAC1::~KeyboardAC1(void)
{
  z80->unregister_ic(this);
}

void
KeyboardAC1::init(void)
{
  _key = 0;
  _control = false;
}

int
KeyboardAC1::decode_key(int keysym, bool press)
{
  return __keys[keysym];
}

void
KeyboardAC1::keyPressed(int keysym, int keycode)
{
  if (keysym == KC_KEY_CONTROL)
    _control = true;

  if (_key != 0)
    return;

  _key = decode_key(keysym, true);
  DBG(2, form("KCemu/keyboard/ac1/key_press",
	      "##### Keyboard: keyPressed  [%03x/%02x] {%02x}\n",
	      keysym,
	      keycode,
	      _key));
}

void
KeyboardAC1::keyReleased(int keysym, int keycode)
{
  if ((keysym == KC_KEY_CONTROL) || (keysym < 0))
    _control = false;

  DBG(2, form("KCemu/keyboard/ac1/key_released",
	      "##### Keyboard: keyReleased [%03x/%02x] {%02x}\n",
	      keysym,
	      keycode,
	      _key));

  _key = 0;
}

void
KeyboardAC1::replayString(const char *text)
{
}

void
KeyboardAC1::callback(void *data)
{
}

void
KeyboardAC1::reset(bool power_on)
{
  init();
}

void
KeyboardAC1::reti(void)
{
}

int
KeyboardAC1::callback_A_in(void)
{
  if (_key == 0)
    return 0x00;
  
  int val = _key;
  if (_control)
    val &= 0x1f;

  return 0x80 | val;
}

int
KeyboardAC1::callback_B_in(void)
{
  return -1;
}
