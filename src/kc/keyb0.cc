/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
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

#include "kc/z80.h" // DEBUG
#include "kc/keys.h"
#include "kc/pio0.h"
#include "kc/keyb0.h"
#include "kc/keyb0k.h"
#include "kc/ports0.h"

#include "cmd/cmd.h"

Keyboard0::Keyboard0(void)
{
  init();
}

Keyboard0::~Keyboard0(void)
{
}

void
Keyboard0::init(void)
{
  _key = 0;
  _keyval = 0;
  _delay = SHIFT_DELAY;
  _variant = get_kc_variant();
}

void
Keyboard0::keyPressed(int keysym, int keycode)
{
  _key = __keys[keysym];
  _keysym = keysym;
  //cout << "keysym = " << keysym << ", keycode = " << keycode << " -> " << hex << _key << endl;
  keyboard_handler();
}

void
Keyboard0::keyReleased(int keysym, int keycode)
{
  _key = 0;
  _keysym = 0;
  keyboard_handler();
}

void
Keyboard0::replayString(const char *text)
{
}

void
Keyboard0::callback(void *data)
{
}

void
Keyboard0::callback_A_in(void)
{
}

void
Keyboard0::callback_B_in(void)
{
}

void
Keyboard0::reset(bool power_on)
{
  init();
}

void
Keyboard0::reti(void)
{
}

void
Keyboard0::keyboard_handler_default(void)
{
  byte_t output = 0x01f;

  if (_value == 0)
    {
      if (_key != _keyval)
	{
	  _delay = 5;
	  _keyval = _key;
	}
    }

  if (_keyval == 0)
    return;

  /*
   *  shift level (S1 = 0, S2 = 1, S3 = 2, S4 = 3)
   */
  int shift = ((_keyval >> 8) & 0xff) - 1;

  if (shift >= 0)
    if (_value == shift)
      output &= ~8;

  /*
   *  short delay for the shift keys
   */
  if (_delay > 0)
    {
      if (_value == 0)
	_delay--;
    }
  else
    {
      if (_value == ((_keyval >> 4) & 0x0f))
	output &= ~(_keyval & 0x0f);
    }

  pio->set_B_EXT(0x1f, output);
}

void
Keyboard0::keyboard_handler_rb(void)
{
  byte_t val = 0x0f;

  if ((_keysym >= '0') && (_keysym <= '7'))
    if ((_value == 14) || (_value == 15))
      val = _keysym - '0';

  bool l1 = (val == 0) || (val == 2) || (val == 4) || (val == 6);
  bool l2 = (val == 1) || (val == 2) || (val == 5) || (val == 6);
  bool l3 = (val == 3) || (val == 4) || (val == 5) || (val == 6);
  bool l4 = (val == 7);

  val = 0x1f;
  if (l1)
    val ^= 1;
  if (l2)
    val ^= 2;
  if (l3)
    val ^= 4;
  if (l4)
    val ^= 8;

  pio->set_B_EXT(0x1f, val);
}

void
Keyboard0::keyboard_handler(void)
{
  if (_variant == KC_VARIANT_Z1013_RB)
    keyboard_handler_rb();
  else
    keyboard_handler_default();
}

void
Keyboard0::set_value(byte_t value)
{
  _value = value;
  keyboard_handler();
}
