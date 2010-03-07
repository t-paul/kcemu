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

#include <ctype.h>
#include <iostream>
#include <iomanip>

#include "kc/system.h"

#include "cmd/cmd.h"

#include "kc/z80.h"
#include "kc/pio.h"
#include "kc/keys.h"
#include "kc/keyb3.h"
#include "kc/keyb3k.h"

#include "libdbg/dbg.h"

/* #define KEYBOARD_CB_DEBUG */
/* #define KEYBOARD_ADD_CB_DEBUG */

class CMD_keyboard_replay : public CMD
{
private:
  Keyboard *_k;
public:
  CMD_keyboard_replay(Keyboard *k) : CMD("keyboard-replay")
    {
      _k = k;
      register_cmd("keyboard-replay");
    }

  void execute(CMD_Args *args, CMD_Context context)
    {
      const char *text;

      if (!args)
	return;

      text = args->get_string_arg("text");
      if (!text)
	return;
      _k->replayString(text);
    }
};

Keyboard3::Keyboard3(void)
{
  init();
  z80->register_ic(this);
  _cmd = new CMD_keyboard_replay(this);
}

Keyboard3::~Keyboard3(void)
{
  delete _cmd;
  z80->unregister_ic(this);
}

void
Keyboard3::init(void)
{
  _key = -1;
  _lock = -1;
  _keysym = 0;
  _release = 0;

  _shift = false;
  _control = false;

  _replay_idx = 0;
  //_replay_text = NULL;
  _replay_offset = 50 * 256;
}

/*
 *  keysym is the translated key (including shift!)
 *  keycode is only the number of the pressed key
 */
void
Keyboard3::keyPressed(int keysym, int keycode)
{
  int key;

  switch (keysym)
    {
    case KC_KEY_SHIFT:
      _shift = true;
      return;
    case KC_KEY_CONTROL:
      _control = true;
      return;
    case KC_KEY_ALT_GR:
      return;
    default:
      break;
    }

  if (_control)
    if ((keysym >= 0x61) && (keysym <= 0x7a))
      keysym -= 0x60;
  key = __keys[keysym];
  if (key == 0xffff)
    return;

  if ((key & 0x100) || _shift)
    key |= 1; /* set shift state if required for this key, see keyb3k.h */

  if (key & 0x1000)
    key &= ~1; /* force unshifted key if the key is shifted on pc keyboard
		* but not on the kc keyboard */

  key &= 0xff;

  key ^= 1; /*
             *  well, I really don't understand why we need to send
             *  the value with the lowest bit toggled. this doesn't
             *  match the key table in the system manual :-(
             */
  DBG(2, form("KCemu/keyboard/3/key_press",
	      "Keyboard::keyPressed():  <%03xh> %3d [%c] -> %02xh (%3d)\n",
	      keysym & 0xfff, keysym, isprint(keysym) ? keysym : '.', key, key));

  _release = 0;
  _keysym = keysym;
  // cerr.form("-> %3d, %3d [keyPressed]\n", _key, _lock);
  if (_key == -1)
    {
      _key = key;
      //cerr.form("-> %3d, %3d [free]\n", _key, _lock);
      sendKey();
    }
  else
    {
      _lock = key;
      //cerr.form("-> %3d, %3d [locked]\n", _key, _lock);
    }
}

void
Keyboard3::keyReleased(int keysym, int keycode)
{
  DBG(2, form("KCemu/keyboard/3/key_release",
	      "Keyboard::keyReleased(): <%03xh> %3d [%c] - _keysym = %02xh\n",
	      keysym & 0xfff, keysym, isprint(keysym) ? keysym : '.', _keysym));

  switch (keysym)
    {
    case KC_KEY_SHIFT:
      _shift = false;
      return;
    case KC_KEY_CONTROL:
      _control = false;
      return;
    case KC_KEY_ALT_GR:
      return;
    default:
      break;
    }

  if ((keysym < 0) || (_keysym == keysym))
    _release = 1;
}

void
Keyboard3::replayString(const char *text)
{
/*
  if (_replay_text == NULL)
    _replay_text = g_string_new(NULL);
  g_string_append(_replay_text, text);
  checkReplay();
*/
}

void
Keyboard3::sendKey(void)
{
  int key;
  int a, b;
  int offset;

  //cerr.form("=> %3d, %3d [sendKey]\n", _key, _lock);
#ifdef KEYBOARD_ADD_CB_DEBUG
  cerr.form("Keyboard: addCallback(): %8Ld, %8ld, %8ld\n",
	    z80->getCounter(), 0, 0);
#endif
  offset = 0;

  z80->addCallback(offset, this, (void *)0);

  key = _key;
  for (a = 1;a < 8;a++) {
    b = (key & 1) ? KEY_CYCLES_BIT_1 : KEY_CYCLES_BIT_0;
    offset += b;
    key >>= 1;
#ifdef KEYBOARD_ADD_CB_DEBUG
    cerr.form("Keyboard: addCallback(): %8Ld, %8ld, %8ld\n",
	      z80->getCounter() + offset, a, offset);
#endif
    z80->addCallback(offset, this, (void *)a);
  }

  offset += KEY_CYCLES_T_W;
#ifdef KEYBOARD_ADD_CB_DEBUG
  cerr.form("Keyboard: addCallback(): %8Ld, %8ld, %8ld\n",
	    z80->getCounter() + offset, 8, offset);
#endif
  z80->addCallback(offset, this, (void *)8);

  key = _key;
  for (a = 9;a < 16;a++)
    {
      b = (key & 1) ? KEY_CYCLES_BIT_1 : KEY_CYCLES_BIT_0;
      offset += b;
      key >>= 1;
#ifdef KEYBOARD_ADD_CB_DEBUG
      cerr.form("Keyboard: addCallback(): %8Ld, %8ld, %8ld\n",
		z80->getCounter() + offset, a, offset);
#endif
      z80->addCallback(offset, this, (void *)a);
    }

  offset += KEY_CYCLES_T_DW;
#ifdef KEYBOARD_ADD_CB_DEBUG
  cerr.form("Keyboard: addCallback(): %8Ld, %8ld, %8ld\n",
	    z80->getCounter() + offset, 64, offset);
#endif
  z80->addCallback(offset, this, (void *)64);
}

int
Keyboard3::checkReplay(void)
{
  return 0;
#if 0
  int key;

  if (_replay_text == NULL)
    return -1;

  key = _replay_text->str[_replay_idx++];
  if (_replay_idx >= _replay_text->len)
    {
      g_string_free(_replay_text, TRUE);
      _replay_idx = 0;
      _replay_text = NULL;
    }

  /*
   *  invert case
   */
  if ((key >= 0x41) && (key <= 0x5a))
    key += 0x20;
  else if ((key >= 0x61) && (key <= 0x7a))
    key -= 0x20;

  //cout.form("Keyboard3::checkReplay() ret = %d (%02xh)\n", key, key);

  switch (key)
    {
    case 0x0a: key = 0x0d; break;
    }

  /*
   *  FIXME:
   *  we need a return value for keyPressed to learn if the key
   *  was accepted!
   */
  keyPressed(key, 0);
  keyReleased(key, 0);

  return key;
#endif
}

void
Keyboard3::callback(void *data)
{
  long val = (long)data;

#ifdef KEYBOARD_CB_DEBUG
  static long long last = 0;

  cerr.form("Keyboard:    callback(): %8Ld, %8ld, %8ld\n",
	    z80->getCounter(), val, z80->getCounter() - last);
  //switch (val) {
  //case 0:  cerr.form(" ### vvv\n"); break;
  //case 64: cerr.form(" ### ^^^\n"); break;
  //default: cerr.form("\n");         break;
  //}
  last = z80->getCounter();
#endif
  // if (val == 0) cerr.form("\n");
  // cerr.form("[%02x] ", val);
  switch (val)
    {
    default:
      pio->strobe_B();
      break;
    case 64:
      if (_lock != -1)
	{
	  _key = _lock;
	  _lock = -1;
	}

      if (_release > 0)
	{
	  _release--;
	  if (_release == 0)
	    _key = -1;
	}

      if (_key != -1)
	sendKey();
      else
	{
	  /*
	   *  triggering strobe here is most likely wrong!
           */
	  //pio->strobe_B();

#ifdef KEYBOARD_ADD_CB_DEBUG
	  cerr.form("Keyboard: addCallback(): %8Ld, %8ld, %8ld\n",
		    z80->getCounter() + _replay_offset, 128, _replay_offset);
#endif
	  z80->addCallback(_replay_offset, this, (void *)128);
	}
      break;
    case 128:
      /*
       *  we need an additional delay after normal end of key handling
       *  which is marked by val == 64 to separate the automatically
       *  generated key presses
       *  otherwise we could trigger the kc internal key repeat function
       *  FIXME:
       *    check against the U807 spec if the above statement is true
       *
       *  an other problem is the screen scrolling that will cause some
       *  characters to be ignored :-(
       */
      if (checkReplay() == 0x0d)
	_replay_offset = 800 * 256;
      else
	_replay_offset = 100 * 256;
      break;
    }
}

void
Keyboard3::reset(bool power_on)
{
  init();
}

void
Keyboard3::reti(void)
{
}
