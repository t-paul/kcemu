/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2009 Torsten Paul
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
#include "kc/bcs3/keydef.h"
#include "kc/bcs3/keyboard.h"

#include "cmd/cmd.h"

#include "libdbg/dbg.h"

KeyboardBCS3::KeyboardBCS3(void)
{
  init();
  z80->register_ic(this);
}


KeyboardBCS3::~KeyboardBCS3(void)
{
  z80->unregister_ic(this);
}

void
KeyboardBCS3::init(void)
{
  _key = 0;
}

int
KeyboardBCS3::decode_key(int keysym, bool press)
{
  return __keys[keysym];
}

void
KeyboardBCS3::keyPressed(int keysym, int keycode)
{
  if (keysym == KC_KEY_SHIFT)
    _shift = true;

  _key = decode_key(keysym, true);
  DBG(2, form("KCemu/keyboard/bcs3/key_press",
	      "##### Keyboard: keyPressed  [%03x/%02x] {%04x}\n",
	      keysym,
	      keycode,
	      _key));
}

void
KeyboardBCS3::keyReleased(int keysym, int keycode)
{
  if (keysym == KC_KEY_SHIFT)
    _shift = false;

  DBG(2, form("KCemu/keyboard/bcs3/key_released",
	      "##### Keyboard: keyReleased [%03x/%02x] {%04x}\n",
	      keysym,
	      keycode,
	      _key));

  _key = 0;
}

void
KeyboardBCS3::replayString(const char *text)
{
}

byte_t
KeyboardBCS3::memory_read(word_t addr)
{
  byte_t val = 0x00;

  if ((addr & (_key & 0x3ff)) == 0)
    val = (_key >> 12) & 0x0f;

  if (_shift)
    if ((addr & 0x0200) == 0)
       val |= 0x01;
  
  return val;
}

void
KeyboardBCS3::callback(void *data)
{
}

void
KeyboardBCS3::reset(bool power_on)
{
  init();
}

void
KeyboardBCS3::reti(void)
{
}

int
KeyboardBCS3::callback_A_in(void)
{
  return -1;
}

int
KeyboardBCS3::callback_B_in(void)
{
  return -1;
}
