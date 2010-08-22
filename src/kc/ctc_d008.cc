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

#include "kc/tape.h"
#include "kc/ctc_d008.h"

#include "ui/ui.h"

#include "libdbg/dbg.h"

CTC_D008::CTC_D008(void)
{
}

CTC_D008::~CTC_D008(void)
{
}

byte_t
CTC_D008::in(word_t addr)
{
  DBG(2, form("KCemu/CTC/D008/in",
              "CTC_D008::in(): addr = %04x\n",
              addr));

  switch (addr & 3)
    {
    case 0:
      return c_in(0);
    case 1:
      return c_in(1);
    case 2:
      return c_in(2);
    case 3:
      return c_in(3);
    }

  return 0; // shouldn't be reached
}

void
CTC_D008::out(word_t addr, byte_t val)
{
  DBG(2, form("KCemu/CTC/D008/out",
              "CTC_D008::out(): addr = %04x, val = %02x\n",
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
CTC_D008::irq_0(void)
{
  return true;
}

bool
CTC_D008::irq_1(void)
{
  return true;
}

bool
CTC_D008::irq_2(void)
{
  return true;
}

bool
CTC_D008::irq_3(void)
{
  return true;
}

long
CTC_D008::counter_value_0(void)
{
  return 0;
}

long
CTC_D008::counter_value_1(void)
{
  return 0;
}

long
CTC_D008::counter_value_2(void)
{
  return 0;
}

long
CTC_D008::counter_value_3(void)
{
  return 0;
}
