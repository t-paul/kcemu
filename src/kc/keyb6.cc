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

#include "kc/kc.h"
#include "kc/pio.h"
#include "kc/keys.h"
#include "kc/keyb6.h"

#include "cmd/cmd.h"

#include "libdbg/dbg.h"

Keyboard6::Keyboard6(void)
{
  init();
}


Keyboard6::~Keyboard6(void)
{
}

void
Keyboard6::init(void)
{
  _key = -1;
}

void
Keyboard6::callback(void *data)
{
}

int
Keyboard6::decode_key(int keysym, bool press)
{
  return keysym;
}

void
Keyboard6::keyPressed(int keysym, int keycode)
{
  _key = keysym;
}

void
Keyboard6::keyReleased(int keysym, int keycode)
{
  if (_key == KC_KEY_ESC)
    {
      CMD_EXEC("emu-reset");
    }

  _key = -1;
}

void
Keyboard6::replayString(const char *text)
{
}

void
Keyboard6::reset(bool power_on)
{
  init();
}

void
Keyboard6::reti(void)
{
}

int
Keyboard6::callback_A_in(void)
{
  return -1;
}

int
Keyboard6::callback_B_in(void)
{
  byte_t val = 0x00;

  switch (_key)
    {
    case 'g':
    case 'G':
      if (_line_val & 0x01)
	val = 0x10;
      break;
    case KC_KEY_ENTER:
      if (_line_val & 0x04)
	val = 0x10;
      break;
    case 'B':
      if (_line_val & 0x08)
	val = 0x10;
      break;
    case 'r':
    case 'R':
      if (_line_val & 0x10)
	val = 0x10;
      break;
    case 'F':
      if (_line_val & 0x20)
	val = 0x10;
      break;
    case 's':
    case 'S':
      if (_line_val & 0x40)
	val = 0x10;
      break;
    case 'm':
    case 'M':
      if (_line_val & 0x80)
	val = 0x10;
      break;

    case '0':
      if (_line_val & 0x01)
	val = 0x20;
      break;
    case '1':
      if (_line_val & 0x08)
	val = 0x20;
      break;
    case '2':
      if (_line_val & 0x02)
	val = 0x20;
      break;
    case '3':
      if (_line_val & 0x04)
	val = 0x20;
      break;
    case '4':
      if (_line_val & 0x01)
	val = 0x80;
      break;
    case '5':
      if (_line_val & 0x08)
	val = 0x80;
      break;
    case '6':
      if (_line_val & 0x02)
	val = 0x80;
      break;
    case '7':
      if (_line_val & 0x04)
	val = 0x80;
      break;
    case '8':
      if (_line_val & 0x10)
	val = 0x20;
      break;
    case '9':
      if (_line_val & 0x20)
	val = 0x20;
      break;
    case 'a':
      if (_line_val & 0x80)
	val = 0x20;
      break;
    case 'b':
      if (_line_val & 0x40)
	val = 0x20;
      break;
    case 'c':
      if (_line_val & 0x10)
	val = 0x80;
      break;
    case 'd':
      if (_line_val & 0x20)
	val = 0x80;
      break;
    case 'e':
      if (_line_val & 0x80)
	val = 0x80;
      break;
    case 'f':
      if (_line_val & 0x40)
	val = 0x80;
      break;
    }

  pio->set_B_EXT(0xb0, val); // K1 = 0x10, K2 = 0x20, K3 = 0x80 !

  return -1;
}

void
Keyboard6::set_line(byte_t val)
{
  _line_val = val;
}
