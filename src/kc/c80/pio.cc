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
#include "kc/z80.h"
#include "kc/c80/pio.h"

#include "libdbg/dbg.h"

PIOC80_1::PIOC80_1(void)
{
}

PIOC80_1::~PIOC80_1(void)
{
}

byte_t
PIOC80_1::in(word_t addr)
{
  byte_t val = 0xff;
  static byte_t last_key = 0x07;

  switch (addr & 3)
    {
    case 0:
      val = in_A_DATA();
      if (last_key != (val & 0x07))
	{
	  DBG(2, form("KCemu/PIO/c80/in_key",
		      "PIOC80_1::in(): addr = %04x, val = %02x, [ %d : %d : %d ]\n",
		      addr, val, (val & 4) >> 2, (val & 2) >> 1, val & 1));
	  last_key = val & 0x07;
	}
      break;
    case 1:
      val = in_B_DATA();
      break;
    case 2:
      val = in_A_CTRL();
      break;
    case 3:
      val = in_B_CTRL();
      break;
    }

  DBG(2, form("KCemu/PIO/c80/in",
              "PIOC80_1::in(): addr = %04x, val = %02x\n",
              addr, val));

  return val;
}

void
PIOC80_1::out(word_t addr, byte_t val)
{
  DBG(2, form("KCemu/PIO/c80/out",
              "PIOC80_1::out(): addr = %04x, val = %02x\n",
              addr, val));

  switch (addr & 3)
    {
    case 0:
      out_A_DATA(val);
      break;
    case 1:
      out_B_DATA(val);
      break;
    case 2:
      out_A_CTRL(val);
      break;
    case 3:
      out_B_CTRL(val);
      break;
    }
}

void
PIOC80_1::change_A(byte_t changed, byte_t val)
{
}

void
PIOC80_1::change_B(byte_t changed, byte_t val)
{
}

PIOC80_2::PIOC80_2(void)
{
}

PIOC80_2::~PIOC80_2(void)
{
}

byte_t
PIOC80_2::in(word_t addr)
{
  byte_t val = 0xff;

  switch (addr & 3)
    {
    case 0:
      val = in_A_DATA();
      break;
    case 1:
      val = in_B_DATA();
      break;
    case 2:
      val = in_A_CTRL();
      break;
    case 3:
      val = in_B_CTRL();
      break;
    }

  DBG(2, form("KCemu/PIO/c80/in",
              "PIOC80_2::in(): addr = %04x, val = %02x\n",
              addr, val));

  return val;
}

void
PIOC80_2::out(word_t addr, byte_t val)
{
  DBG(2, form("KCemu/PIO/c80/out",
              "PIOC80_2::out(): addr = %04x, val = %02x\n",
              addr, val));

  switch (addr & 3)
    {
    case 0:
      out_A_DATA(val);
      break;
    case 1:
      out_B_DATA(val);
      break;
    case 2:
      out_A_CTRL(val);
      break;
    case 3:
      out_B_CTRL(val);
      break;
    }
}

void
PIOC80_2::change_A(byte_t changed, byte_t val)
{
}

void
PIOC80_2::change_B(byte_t changed, byte_t val)
{
}
