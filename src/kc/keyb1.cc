/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: keyb1.cc,v 1.8 2002/10/31 01:46:35 torsten_paul Exp $
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

#include "kc/keys.h"
#include "kc/keyb1.h"
#include "kc/keyb1k.h"

#define PARANOIA_CHECK
#define KEYB_DEBUG

Keyboard1::Keyboard1(void)
{
  init();
}

Keyboard1::~Keyboard1(void)
{
}

void
Keyboard1::init(void)
{
  int a;

  _in_callback = false;
  for (a = 0;a < MAX_KEYS;a++)
    {
      _keybuf[a].sym = -1;
      _keybuf[a].code = -1;
    }
}

int
Keyboard1::decode_key(int keysym, bool press)
{
  return keysym;
}

void
Keyboard1::keyPressed(int keysym, int keycode)
{
  int a;
  int k;
#ifdef KEYB_DEBUG
  byte_t c1, c2, r1, r2;
#endif /* KEYB_DEBUG */

  k = decode_key(keysym, true);
  if (k == -1)
    return;

  k = __keys[k];

  c1 = r1 = 0;
  for (a = 0;a < MAX_KEYS;a++)
    {
#ifdef KEYB_DEBUG
      if (_keybuf[a].code != -1)
        {
          r1 |= 1 << (((__keys[_keybuf[a].sym] >> 4) & 0x0f) - 1);
          c1 |= 1 << (((__keys[_keybuf[a].sym]     ) & 0x0f) - 1);
        }
#endif /* KEYB_DEBUG */
      if (k != 0)
        if (_keybuf[a].code == -1)
          {
            _keybuf[a].sym = k & 0xff;
            _keybuf[a].code = keycode;
#ifdef KEYB_DEBUG
            r2 = 1 << (((k >> 4) & 0x0f) - 1);
            c2 = 1 << (((k     ) & 0x0f) - 1);
#endif /* KEYB_DEBUG */
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

  callback_B_in();
}

void
Keyboard1::keyReleased(int keysym, int keycode)
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

  callback_B_in();
}

void
Keyboard1::replayString(const char *text)
{
}

void
Keyboard1::callback(void *data)
{
}

int
Keyboard1::callback_A_in(void)
{
  int a, x;
  byte_t b, c;

  if (_in_callback)
    return -1;

  _in_callback = true;
  b = ~pio2->in_B_DATA();
  x = 0;
  c = 0;
  for (a = 0;a < MAX_KEYS;a++)
    {
      if (_keybuf[a].code == -1)
        continue;
      if (b & (1 << (((_keybuf[a].sym >> 4) & 0x0f) - 1)))
        {
          c |= 1 << (((_keybuf[a].sym) & 0x0f) - 1);
          x++;
        }
    }

  //if (c != 0)
  //cout.form("* PORT A: value = %02x\n", c);
  c = ~c;
#if 0
  cout.form("##### Keyboard: Port A: new ext val: %02x [x = %d] -", c, x);
  for (a = 0;a < MAX_KEYS;a++)
    if (_keybuf[a].code != -1)
      cout.form(" {%02x}", _keybuf[a].sym);
  cout.form("\n");
#endif
  pio2->set_A_EXT(0xff, c);
  _in_callback = false;

  return -1;
}

int
Keyboard1::callback_B_in(void)
{
  int a, x;
  byte_t b, c;

  if (_in_callback)
    return -1;

  _in_callback = true;
  b = ~pio2->in_A_DATA();
  x = 0;
  c = 0;
  for (a = 0;a < MAX_KEYS;a++)
    {
      if (_keybuf[a].code == -1)
        continue;
      if (b & (1 << (((_keybuf[a].sym) & 0x0f) - 1)))
        {
          c |= 1 << (((_keybuf[a].sym >> 4) & 0x0f) - 1);
          x++;
        }
    }

  //if (c != 0)
  //cout.form("* PORT B: value = %02x\n", c);
  c = ~c;
#if 0
  cout.form("##### Keyboard: Port B: new ext val: %02x [x = %d] -", c, x);
  for (a = 0;a < MAX_KEYS;a++)
    if (_keybuf[a].code != -1)
      cout.form(" {%02x}", _keybuf[a].sym);
  cout.form("\n");
#endif
  pio2->set_B_EXT(0xff, c);
  _in_callback = false;

  return -1;
}

void
Keyboard1::reset(bool power_on)
{
  init();
}

void
Keyboard1::reti(void)
{
}
