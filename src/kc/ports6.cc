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

#include <stdio.h>
#include <iostream>
#include <iomanip>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/pio6.h"
#include "kc/keyb6.h"
#include "kc/ports6.h"

#include "libdbg/dbg.h"

Ports6::Ports6(void)
{
  _val = 0;
  for (int a = 0;a < 8;a++)
    _led[a] = 0;
}

Ports6::~Ports6(void)
{
}

byte_t
Ports6::in(word_t addr)
{
  byte_t val = 0xff;

  DBG(2, form("KCemu/Ports/6/in",
	      "Ports6: in() addr = %04x (returning %02x)\n",
	      addr, val));
  
  return val;
}

void
Ports6::out(word_t addr, byte_t val)
{
  int a;

  DBG(2, form("KCemu/Ports/6/out",
              "Ports6: out() addr = %04x, val = %02x\n",
              addr, val));
  
  _val = val;

  ((Keyboard6 *)keyboard)->set_line(val); // FIXME: decouple classes

  bool changed = false;
  for (a = 0;a < 8;a++)
    if (val & (128 >> a))
      {
	byte_t v = ((PIO6_1 *)pio)->get_led_value(); // FIXME: decouple classes
	if (_led[a] != v)
	  {
	    _led[a] = v;
	    changed = true;
	  }
      }

#if 0
  if (!changed)
    return;

  //printf("\x1b[H");

  for (a = 0;a < 8;a++)
    {
      if (_led[a] & 0x20)
	printf(" ---  ");
      else
	printf("      ");
    }
  printf("\n");

  for (a = 0;a < 8;a++)
    {
      if (_led[a] & 0x40)
	printf("|   ");
      else
	printf("    ");
      if (_led[a] & 0x80)
	printf("| ");
      else
	printf("  ");
    }
  printf("\n");

  for (a = 0;a < 8;a++)
    {
      if (_led[a] & 0x40)
	printf("|   ");
      else
	printf("    ");
      if (_led[a] & 0x80)
	printf("| ");
      else
	printf("  ");
    }
  printf("\n");

  for (a = 0;a < 8;a++)
    {
      if (_led[a] & 0x10)
	printf(" ---  ");
      else
	printf("      ");
    }
  printf("\n");

  for (a = 0;a < 8;a++)
    {
      if (_led[a] & 0x01)
	printf("|   ");
      else
	printf("    ");
      if (_led[a] & 0x04)
	printf("| ");
      else
	printf("  ");
    }
  printf("\n");

  for (a = 0;a < 8;a++)
    {
      if (_led[a] & 0x01)
	printf("|   ");
      else
	printf("    ");
      if (_led[a] & 0x04)
	printf("| ");
      else
	printf("  ");
    }
  printf("\n");

  for (a = 0;a < 8;a++)
    {
      if (_led[a] & 0x02)
	printf(" ---  ");
      else
	printf("      ");
    }
  printf("\n");
#endif
}

byte_t
Ports6::get_led_value(int idx)
{
  return _led[idx];
}
