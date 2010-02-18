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
#include "kc/vcs80/keydef.h"
#include "kc/vcs80/keyboard.h"

#include "cmd/cmd.h"

#include "libdbg/dbg.h"

KeyboardVCS80::KeyboardVCS80(void)
{
  init();
  z80->register_ic(this);
}


KeyboardVCS80::~KeyboardVCS80(void)
{
  z80->unregister_ic(this);
}

void
KeyboardVCS80::init(void)
{
  _key = 0;
}

int
KeyboardVCS80::decode_key(int keysym, bool press)
{
  return __keys[keysym];
}

void
KeyboardVCS80::keyPressed(int keysym, int keycode)
{
  if (_key != 0)
    return;

  _key = decode_key(keysym, true);
  DBG(2, form("KCemu/keyboard/vcs80/key_press",
	      "##### Keyboard: keyPressed  [%03x/%02x] {%02x}\n",
	      keysym,
	      keycode,
	      _key));
}

void
KeyboardVCS80::keyReleased(int keysym, int keycode)
{
  DBG(2, form("KCemu/keyboard/vcs80/key_released",
	      "##### Keyboard: keyReleased [%03x/%02x] {%02x}\n",
	      keysym,
	      keycode,
	      _key));

  _key = 0;

  if (keysym == 0x1b)
    CMD_EXEC("emu-reset");
}

void
KeyboardVCS80::replayString(const char *text)
{
}

void
KeyboardVCS80::callback(void *data)
{
}

void
KeyboardVCS80::reset(bool power_on)
{
  init();
}

void
KeyboardVCS80::reti(void)
{
}

int
KeyboardVCS80::callback_A_in(void)
{
  static bool recursion = false; // HACK to prevent recursive callback :-(
  
  if (recursion)
    return -1;

  if (_key == 0)
    return -1;

  recursion = true;
  int row = pio->in_A_DATA() & 7;
  recursion = false;

  if (row == (_key & 7))
    pio->set_A_EXT(0x70, ~(_key & 0x70));
  else
    pio->set_A_EXT(0x70, 0x70);

  return -1;
}

int
KeyboardVCS80::callback_B_in(void)
{
  return -1;
}
