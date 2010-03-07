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
#include "kc/c80/keydef.h"
#include "kc/c80/keyboard.h"

#include "cmd/cmd.h"

#include "libdbg/dbg.h"

KeyboardC80::KeyboardC80(void)
{
  init();
  z80->register_ic(this);
}


KeyboardC80::~KeyboardC80(void)
{
  z80->unregister_ic(this);
}

void
KeyboardC80::init(void)
{
  _key = 0;
}

int
KeyboardC80::decode_key(int keysym, bool press)
{
  return __keys[keysym];
}

void
KeyboardC80::keyPressed(int keysym, int keycode)
{
  if (_key != 0)
    return;

  _key = decode_key(keysym, true);
  DBG(2, form("KCemu/keyboard/c80/key_press",
	      "##### Keyboard: keyPressed  [%03x/%02x] {%02x}\n",
	      keysym,
	      keycode,
	      _key));
}

void
KeyboardC80::keyReleased(int keysym, int keycode)
{
  DBG(2, form("KCemu/keyboard/c80/key_released",
	      "##### Keyboard: keyReleased [%03x/%02x] {%02x}\n",
	      keysym,
	      keycode,
	      _key));

  _key = 0;

  if (keysym == 0x1b)
    CMD_EXEC("emu-reset");
}

void
KeyboardC80::replayString(const char *text)
{
}

void
KeyboardC80::callback(void *data)
{
}

void
KeyboardC80::reset(bool power_on)
{
  init();
}

void
KeyboardC80::reti(void)
{
}

int
KeyboardC80::callback_A_in(void)
{
  if (_key == 0)
    return -1;

  int col = ~pio->in_B_DATA();

  if (col & (_key & 0xff))
    pio->set_A_EXT(0x07, (~(_key >> 8)) & 0x07);
  else
    pio->set_A_EXT(0x07, 0x07);

  return -1;
}

int
KeyboardC80::callback_B_in(void)
{
  return -1;
}
