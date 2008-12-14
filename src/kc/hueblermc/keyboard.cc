/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2004 Torsten Paul
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
#include "kc/hueblermc/keydef.h"
#include "kc/hueblermc/keyboard.h"

#include "libdbg/dbg.h"

KeyboardHueblerMC::KeyboardHueblerMC(void)
{
  init();
  z80->register_ic(this);
}


KeyboardHueblerMC::~KeyboardHueblerMC(void)
{
  z80->unregister_ic(this);
}

void
KeyboardHueblerMC::init(void)
{
  _key = 0;
}

int
KeyboardHueblerMC::decode_key(int keysym, bool press)
{
  return __keys[keysym];
}

void
KeyboardHueblerMC::keyPressed(int keysym, int keycode)
{
  int k = decode_key(keysym, true);
  DBG(2, form("KCemu/keyboard/hueblermc/key_press",
	      "##### Keyboard: keyPressed  [%03x/%02x] => %04x\n",
	      keysym, keycode, k));

  if (k == 0)
    return;

  _key = k;
}

void
KeyboardHueblerMC::keyReleased(int keysym, int keycode)
{
  DBG(2, form("KCemu/keyboard/hueblermc/key_released",
	      "##### Keyboard: keyPressed  [%03x/%02x]\n",
	      keysym, keycode));

  _key = 0;
}

void
KeyboardHueblerMC::replayString(const char *text)
{
}

void
KeyboardHueblerMC::callback(void *data)
{
}

byte_t
KeyboardHueblerMC::in(word_t addr)
{
  byte_t val;

  switch (addr & 0xff)
    {
    case 0x08:
      val = _key;
      break;
    case 0x09:
      val = _key == 0 ? 0 : 0xff;
      break;
    default:
      val = 0xff;
      break;
    }
  
  DBG(2, form("KCemu/keyboard/hueblermc/in",
	      "KeyboardHueblerMC::in():  addr = %04x <= val = %02x\n",
	      addr, val));

  return val;
}

void
KeyboardHueblerMC::out(word_t addr, byte_t val)
{
  DBG(2, form("KCemu/keyboard/hueblermc/out",
	      "KeyboardHueblerMC::out(): addr = %04x => val = %02x\n",
	      addr, val));
}

void
KeyboardHueblerMC::reti(void)
{
}

void
KeyboardHueblerMC::irqreq(void)
{
}

word_t
KeyboardHueblerMC::irqack()
{
  return IRQ_NOT_ACK;
}

void
KeyboardHueblerMC::reset(bool power_on)
{
  init();
}
