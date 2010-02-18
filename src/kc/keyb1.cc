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
#include "kc/keyb1.h"
#include "kc/keyb1k.h"

#include "libdbg/dbg.h"

Keyboard1::Keyboard1(void)
{
  init();
  z80->register_ic(this);
}


Keyboard1::~Keyboard1(void)
{
  z80->unregister_ic(this);
}

void
Keyboard1::init(void)
{
  int a;

  _shift = false;
  _control = false;
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
  /*
   *  ignore repeated key press events (e.g. generated by
   *  GTK that reports key repeat as keypress/keypress/keypress/...
   *  without the possibility to switch that off :-( )
   */
  for (int a = 0;a < MAX_KEYS;a++)
    if (_keybuf[a].code == keycode)
      return;

  int k = decode_key(keysym, true);
  if (k == -1)
    return;

  switch (keysym)
    {
    case KC_KEY_SHIFT:
      _shift = __keys[k];
      k = 0;
      break;
    case KC_KEY_CONTROL:
      _control = __keys[k];
      k = 0;
      break;
    default:
      k = __keys[k];
      break;
    }

  byte_t c1 = 0;
  byte_t r1 = 0;
  byte_t c2 = 0;
  byte_t r2 = 0;

  for (int a = 0;a < MAX_KEYS;a++)
    {
      if (_keybuf[a].code != -1)
        {
	  /*
	   *  row/column values of already pressed keys
	   */
          r1 |= 1 << (((__keys[_keybuf[a].sym] >> 4) & 0x0f) - 1);
          c1 |= 1 << (((__keys[_keybuf[a].sym]     ) & 0x0f) - 1);
        }
      if (k != 0)
        if (_keybuf[a].code == -1)
          {
            _keybuf[a].sym = k;
            _keybuf[a].code = keycode;
	    /*
	     *  row/column values of new pressed keys
	     */
	    r2 |= 1 << (((k >> 4) & 0x0f) - 1);
	    c2 |= 1 << (((k     ) & 0x0f) - 1);
	    /*
	     *  check for 2nd key (this generates duplicate entries for shift,
	     *  ctrl, ... -- but with the keycode of the key itself)
	     */
            k >>= 8;
          }
    }

  DBG(2, form("KCemu/keyboard/1/key_press",
	      "##### Keyboard: keyPressed  [%03x/%02x] {%c%c} "
	      "r/c: [%02x/%02x]=>[%02x/%02x] -",
	      keysym, keycode,
	      _shift > 0 ? 'S' : 's',
	      _control > 0 ? 'C' : 'c',
	      r1, c1, r1 | r2, c1 | c2));
  
  for (int a = 0;a < MAX_KEYS;a++)
    if (_keybuf[a].code != -1)
      DBG(2, form("KCemu/keyboard/1/key_press",
		  " {%02d:%02x}", a, _keybuf[a].code));
  
  DBG(2, form("KCemu/keyboard/1/key_press",
	      "\n"));
  
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

  if (keysym == KC_KEY_SHIFT)
    _shift = 0;

  if (keysym == KC_KEY_CONTROL)
    _control = 0;

  /*
   *  remove (maybe multiple) entries from keyboard buffer
   */
  for (a = 0;a < MAX_KEYS;a++)
    if (_keybuf[a].code == keycode)
      _keybuf[a].code = -1;

  DBG(2, form("KCemu/keyboard/1/key_release",
	      "##### Keyboard: keyReleased [%03x/%02x] {%c%c} "
	      "                      -",
	      keysym, keycode,
	      _shift > 0 ? 'S' : 's',
	      _control > 0 ? 'C' : 'c'));
  
  for (a = 0;a < MAX_KEYS;a++)
    if (_keybuf[a].code != -1)
      DBG(2, form("KCemu/keyboard/1/key_release",
		  " {%02d:%02x}", a, _keybuf[a].code));
  
  DBG(2, form("KCemu/keyboard/1/key_release",
	      "\n"));

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
  if (_in_callback)
    return -1;

  _in_callback = true;
  byte_t b = ~pio2->in_B_DATA();

  byte_t c = 0;
  int count = 0;
  for (int a = 0;a < MAX_KEYS;a++)
    {
      if (_keybuf[a].code == -1)
        continue;

      if (b & (1 << (((_keybuf[a].sym >> 4) & 0x0f) - 1)))
        {
          c |= 1 << (((_keybuf[a].sym) & 0x0f) - 1);
          count++;
        }
    }

  /*
   *  The shift key itself is only asserted when no other key is pressed.
   *  Otherwise this breaks the key handling for keys that are shifted
   *  in the pc layout but not shifted in the kc layout (like the ':' key
   *  that must not have the shift key pressed on the kc).
   */
  if ((count == 0) && (_shift > 0))
    if (b & (1 << (((_shift >> 4) & 0x0f) - 1)))
      c |= 1 << ((_shift & 0x0f) - 1);	

  if (_control > 0)
    if (b & (1 << (((_control >> 4) & 0x0f) - 1)))
      c |= 1 << (((_control) & 0x0f) - 1);

  c = ~c;
#if 0
  cout.form("##### Keyboard: Port A: new ext val: %02x [x = %d] -", c, x);
  for (a = 0;a < MAX_KEYS;a++)
    if (_keybuf[a].code != -1)
      cout.form(" {%04x}", _keybuf[a].sym);
  cout.form("\n");
#endif
  pio2->set_A_EXT(0xff, c);
  _in_callback = false;

  return -1;
}

int
Keyboard1::callback_B_in(void)
{
  if (_in_callback)
    return -1;

  _in_callback = true;
  byte_t b = ~pio2->in_A_DATA();

  byte_t c = 0;
  int count = 0;
  for (int a = 0;a < MAX_KEYS;a++)
    {
      if (_keybuf[a].code == -1)
        continue;

      if (b & (1 << (((_keybuf[a].sym) & 0x0f) - 1)))
        {
          c |= 1 << (((_keybuf[a].sym >> 4) & 0x0f) - 1);
          count++;
        }
    }

  if ((count == 0) && (_shift > 0))
    if (b & (1 << ((_shift & 0x0f) - 1)))
      c |= 1 << (((_shift >> 4) & 0x0f) - 1);	

  if (_control > 0)
    if (b & (1 << ((_control & 0x0f) - 1)))
      c |= 1 << (((_control >> 4) & 0x0f) - 1);

  c = ~c;
#if 0
  cout.form("##### Keyboard: Port B: new ext val: %02x [x = %d] -", c, x);
  for (a = 0;a < MAX_KEYS;a++)
    if (_keybuf[a].code != -1)
      cout.form(" {%04x}", _keybuf[a].sym);
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
