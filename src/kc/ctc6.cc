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
#include "kc/ctc6.h"
#include "kc/poly880.h"

#include "libdbg/dbg.h"

CTC6::CTC6(void)
{
}

CTC6::~CTC6(void)
{
}

byte_t
CTC6::in(word_t addr)
{
  byte_t val = 0xff;

  DBG(2, form("KCemu/CTC/6/in",
              "CTC6::in(): addr = %04x\n",
              addr));

  switch (addr & 3)
    {
    case 0:
      val = c_in(0);
      break;
    case 1:
      val = c_in(1);
      break;
    case 2:
      val = c_in(2);
      break;
    case 3:
      val = c_in(3);
      break;
    }

  DBG(2, form("KCemu/CTC/6/in",
              "CTC6::in():  addr = %04x, val = %02x\n",
              addr, val));
  return val;
}

void
CTC6::out(word_t addr, byte_t val)
{
  DBG(2, form("KCemu/CTC/6/out",
              "CTC6::out(): addr = %04x, val = %02x\n",
              addr, val));

  switch (addr & 3)
    {
    case 0:
      c_out(0, val);
      break;
    case 1:
      c_out(1, val);
      break;
    case 2:
      c_out(2, val);
      break;
    case 3:
      c_out(3, val);
      break;
    }
}

bool
CTC6::irq_0(void)
{
  poly880->trigger_send();
  return false;
}

bool
CTC6::irq_1(void)
{
  return false;
}

bool
CTC6::irq_2(void)
{
  return false;
}

bool
CTC6::irq_3(void)
{
  return false;
}

long
CTC6::counter_value_0(void)
{
  return 0;
}

long
CTC6::counter_value_1(void)
{
  return 0;
}

long
CTC6::counter_value_2(void)
{
  return 0;
}

long
CTC6::counter_value_3(void)
{
  return 0;
}
