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
#include "kc/keyb9.h"
#include "kc/keyb9k.h"

#include "cmd/cmd.h"

Keyboard9::Keyboard9(void)
{
  init();
  z80->register_ic(this);
}

Keyboard9::~Keyboard9(void)
{
  z80->unregister_ic(this);
}

void
Keyboard9::init(void)
{
  int a;

  for (a = 0;a < MAX_KEYS;a++)
    {
      _keybuf[a].sym = -1;
      _keybuf[a].code = -1;
    }
}

int
Keyboard9::decode_key(int keysym, bool press)
{
  return keysym;
}

void
Keyboard9::keyPressed(int keysym, int keycode)
{
  int a;
  int k;
  byte_t c1, c2, r1, r2;

  k = decode_key(keysym, true);
  if (k == -1)
    return;

  k = __keys[k];

  c1 = r1 = 0;
  for (a = 0;a < MAX_KEYS;a++)
    {
      if (_keybuf[a].code != -1)
        {
          r1 |= 1 << (((__keys[_keybuf[a].sym] >> 4) & 0x0f) - 1);
          c1 |= 1 << (((__keys[_keybuf[a].sym]     ) & 0x0f) - 1);
        }
      if (k != 0)
        if (_keybuf[a].code == -1)
          {
            _keybuf[a].sym = k & 0xff;
            _keybuf[a].code = keycode;
            r2 = 1 << (((k >> 4) & 0x0f) - 1);
            c2 = 1 << (((k     ) & 0x0f) - 1);
            k >>= 8;
          }
    }

#if 0
  cout.form("##### Keyboard: keyPressed  [%02x/%02x] "
            "r/c: [%02x/%02x]=>[%02x/%02x] -",
            keysym, keycode,
            r1, c1, r1 | r2, c1 | c2);
  for (a = 0;a < MAX_KEYS;a++)
    if (_keybuf[a].code != -1)
      cout.form(" {%02d:%02x}", a, _keybuf[a].code);
  cout.form("\n");
#endif
}

void
Keyboard9::keyReleased(int keysym, int keycode)
{
  int a;

  if (keysym == -1)
    {
      for (a = 0;a < MAX_KEYS;a++)
        _keybuf[a].code = -1;
      return;
    }

  for (a = 0;a < MAX_KEYS;a++)
    if (_keybuf[a].code == keycode)
      _keybuf[a].code = -1;

#if 0
  cout.form("##### Keyboard: keyReleased [%02x/%02x] -",
            keysym,
            keycode);
  for (a = 0;a < MAX_KEYS;a++)
    if (_keybuf[a].code != -1)
      cout.form(" {%02d:%02x}", a, _keybuf[a].code);
  cout.form("\n");
#endif
}

void
Keyboard9::replayString(const char *text)
{
}

void
Keyboard9::callback(void *data)
{
}

void
Keyboard9::reset(bool power_on)
{
  init();
}

void
Keyboard9::reti(void)
{
}

void
Keyboard9::set_val(byte_t val)
{
  _line = val;
}

byte_t
Keyboard9::get_val()
{
  int a, c;

  c = 0;
  for (a = 0;a < MAX_KEYS;a++)
    {
      if (_keybuf[a].code == -1)
	continue;

      if (_line == (((_keybuf[a].sym >> 4) & 0x0f) - 1))
	c |= 1 << ((_keybuf[a].sym & 0x0f) - 1);
    }

  return ~c;
}
