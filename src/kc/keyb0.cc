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
#include "kc/prefs/prefs.h"

#include "kc/z80.h"
#include "kc/keys.h"
#include "kc/pio0.h"
#include "kc/keyb0.h"
#include "kc/keyb0k.h"
#include "kc/ports0.h"

#include "cmd/cmd.h"

Keyboard0::Keyboard0(void)
{
  init();
  z80->register_ic(this);
}

Keyboard0::~Keyboard0(void)
{
  z80->unregister_ic(this);
}

void
Keyboard0::init(void)
{
  _ext = 0;
  _key = 0;
  _keysym = 0;
  _variant = Preferences::instance()->get_kc_variant();
}

void
Keyboard0::keyPressed(int keysym, int keycode)
{
  switch (keysym)
    {
    case KC_KEY_CONTROL:
      _control = true;
      break;
    default:
      break;
    }

  _keysym = keysym;
  keyboard_handler();
}

void
Keyboard0::keyReleased(int keysym, int keycode)
{
  switch (keysym)
    {
    case KC_KEY_CONTROL:
      _control = false;
      break;
    default:
      break;
    }

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

int
Keyboard0::callback_A_in(void)
{
  return -1;
}

int
Keyboard0::callback_B_in(void)
{
  return _ext;
}

void
Keyboard0::callback_A_out(byte_t val)
{
}

void
Keyboard0::callback_B_out(byte_t val)
{
  if (_pio_value != (val & 0x10))
    {
      _pio_value = val & 0x10;
      keyboard_handler();
    }
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
  _ext = 0x1f;

  /*
   *  latch key value only at start of keyboard scan to
   *  prevent missing recognition of the shift keys
   */
  if (_value == 0)
    _key = __keys[_keysym];

  if (_key == 0)
    {
      pio->set_B_EXT(0x1f, _ext);
      return;
    }

  /*
   *  shift level (S1 = 0, S2 = 1, S3 = 2, S4 = 3)
   */
  int shift = ((_key >> 8) & 0xff) - 1;

  if (shift >= 0)
    if (_value == shift)
      _ext &= ~8;

  if (_value == ((_key >> 4) & 0x0f))
    _ext &= ~(_key & 0x0f);

  pio->set_B_EXT(0x1f, _ext);
}

void
Keyboard0::keyboard_handler_a2(void)
{
  _key = __keys_a2[_keysym];

  if (_key == 0)
    {
      _ext = 0x0f;
      pio->set_B_EXT(0x0f, _ext);
      return;
    }

  int row1 = (_key & 0x0f) - 1;
  int col1 = ((_key & 0xf0) >> 4);

  int row2 = ((_key >> 8) & 0x0f) - 1;
  int col2 = ((_key & 0xf000) >> 12);

  _ext = 0x0f;

  if (_value == row1)
    {
      int v = 1 << col1;

      if (_pio_value)
	v >>= 4;

      _ext &= (v ^ 0xff);
    }

  if (_value == row2)
    {
      int v = 1 << col2;

      if (_pio_value)
	v >>= 4;

      _ext &= (v ^ 0xff);
    }

  pio->set_B_EXT(0x0f, _ext);
}

void
Keyboard0::keyboard_handler_rb(void)
{
  _key = __keys_rb_k7659[_keysym];

  if (_key == 0)
    {
      _ext = 0x0f;
      pio->set_B_EXT(0x1f, _ext);
      return;
    }

  int row1 = (_key & 0x0f) - 1;
  int col1 = ((_key & 0xf0) >> 4);

  byte_t val = 0x00;
  if ((_value == row1) || (_value == 15))
    val = (1 << col1);

  if (_key & 0xff00)
    {
      int row2 = ((_key >> 8) & 0x0f) - 1;
      int col2 = ((_key & 0xf000) >> 12);

      if ((_value == row2) || (_value == 15))
	val |= (1 << col2);
    }

  if (_control)
    if ((_value == 7) || (_value == 15))
      val |= (1 << 7);

  int l1 = (val & 0x01) || (val & 0x04) || (val & 0x10) || (val & 0x40);
  int l2 = (val & 0x02) || (val & 0x04) || (val & 0x20) || (val & 0x40);
  int l3 = (val & 0x08) || (val & 0x10) || (val & 0x20) || (val & 0x40);
  int l4 = (val & 0x80);

  val = 0x1f;
  if (l1)
    val ^= 1;
  if (l2)
    val ^= 2;
  if (l3)
    val ^= 4;
  if (l4)
    val ^= 8;

  _ext = val;
  pio->set_B_EXT(0x1f, _ext);
}

void
Keyboard0::keyboard_handler(void)
{
  switch (_variant)
    {
    case KC_VARIANT_Z1013_01:
    case KC_VARIANT_Z1013_12:
    case KC_VARIANT_Z1013_16:
    case KC_VARIANT_Z1013_64:
      keyboard_handler_default();
      break;
    case KC_VARIANT_Z1013_A2:
      keyboard_handler_a2();
      break;
    case KC_VARIANT_Z1013_RB:
    case KC_VARIANT_Z1013_BL4:
    case KC_VARIANT_Z1013_SURL:
      keyboard_handler_rb();
      break;
    default:
      break;
    }
}

void
Keyboard0::set_value(byte_t value)
{
  _value = value;
  keyboard_handler();
}
