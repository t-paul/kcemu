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
#include "kc/pio6.h"
#include "kc/poly880.h"

#include "libdbg/dbg.h"

PIO6_1::PIO6_1(void)
{
  _led_value = 0;
  set_B_EXT(0xff, 0x00);
}

PIO6_1::~PIO6_1(void)
{
}

byte_t
PIO6_1::in(word_t addr)
{
  byte_t val = 0xff;

  switch (addr & 3)
    {
    case 0:
      val = in_A_DATA();
      break;
    case 1:
      val = in_A_CTRL();
      break;
    case 2:
      val = in_B_DATA();
      break;
    case 3:
      val = in_B_CTRL();
      break;
    }

  DBG(2, form("KCemu/PIO/6a/in",
              "PIO6_1::in():  addr = %04x, val = %02x\n",
              addr, val));

  return val;
}

void
PIO6_1::out(word_t addr, byte_t val)
{
  DBG(2, form("KCemu/PIO/6a/out",
              "PIO6_1::out(): addr = %04x, val = %02x\n",
              addr, val));

  switch (addr & 3)
    {
    case 0:
      out_A_DATA(val);
      _led_value = val;
      break;
    case 1:
      out_A_CTRL(val);
      break;
    case 2:
      out_B_DATA(val);
      break;
    case 3:
      out_B_CTRL(val);
      break;
    }
}

void
PIO6_1::change_A(byte_t changed, byte_t val)
{
}

void
PIO6_1::change_B(byte_t changed, byte_t val)
{
  poly880->set_scon((val & 64) == 64);
  //printf("port B: %02x %s %s\n", val, (val & 4)  ? "MOUT" : "mout", (val & 64) ? "SCON" : "scon");
}

byte_t
PIO6_1::get_led_value(void)
{
  return _led_value;
}

PIO6_2::PIO6_2(void)
{
}

PIO6_2::~PIO6_2(void)
{
}

byte_t
PIO6_2::in(word_t addr)
{
  byte_t val = 0xff;

  switch (addr & 3)
    {
    case 0:
      val = in_A_DATA();
      break;
    case 1:
      val = in_A_CTRL();
      break;
    case 2:
      val = in_B_DATA();
      break;
    case 3:
      val = in_B_CTRL();
      break;
    }

  DBG(2, form("KCemu/PIO/6b/in",
              "PIO6_2::in():  addr = %04x, val = %02x\n",
              addr, val));

  return val;
}

void
PIO6_2::out(word_t addr, byte_t val)
{
  DBG(2, form("KCemu/PIO/6b/out",
              "PIO6_2::out(): addr = %04x, val = %02x\n",
              addr, val));

  switch (addr & 3)
    {
    case 0:
      out_A_DATA(val);
      break;
    case 1:
      out_A_CTRL(val);
      break;
    case 2:
      out_B_DATA(val);
      break;
    case 3:
      out_B_CTRL(val);
      break;
    }
}

void
PIO6_2::change_A(byte_t changed, byte_t val)
{
}

void
PIO6_2::change_B(byte_t changed, byte_t val)
{
}
