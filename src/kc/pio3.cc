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
#include "kc/pio3.h"
#include "kc/tape.h"
#include "kc/memory3.h"

#include "libdbg/dbg.h"

#define memory ((Memory3 *)memory)

PIO3::PIO3(void)
{
}

PIO3::~PIO3(void)
{
}

byte_t
PIO3::in(word_t addr)
{
  byte_t ret = 0xff;

  switch (addr & 3)
    {
    case 0:
      ret = in_A_DATA();
      break;
    case 1:
      ret = in_B_DATA();
      break;
    case 2:
      ret = in_A_CTRL();
      break;
    case 3:
      ret = in_B_CTRL();
      break;
    }

  DBG(2, form("KCemu/PIO/3/in",
              "PIO3::in():  addr = %04x, val = %02x\n",
              addr, ret));

  return ret;
}

void
PIO3::out(word_t addr, byte_t val)
{
  DBG(2, form("KCemu/PIO/3/out",
              "PIO3::out(): addr = %04x, val = %02x\n",
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

byte_t
PIO3::in_A_DATA(void)
{
  byte_t ret = PIO::in_A_DATA();

  if (_mode[A] == MODE_INPUT)
    ret = 0x07;

  return ret;
}

void
PIO3::change_A(byte_t changed, byte_t val)
{
  if (changed & 0x01)
    {
      DBG(2, form("KCemu/PIO/3/change/A",
                  "PIO A: CAOS ROM [%d]\n",
                  (val & 1)));

      memory->enableCAOS_E(val & 1);
    }
  if (changed & 0x02)
    {
      DBG(2, form("KCemu/PIO/3/change/A",
                  "PIO A: RAM [%d]\n",
                  ((val >> 1) & 1)));
      memory->enableRAM((val >> 1) & 1);
    }
  if (changed & 0x04)
    {
      DBG(2, form("KCemu/PIO/3/change/A",
                  "PIO A: IRM [%d]\n",
                  ((val >> 2) & 1)));
      memory->enableIRM((val >> 2) & 1);
    }
  if (changed & 0x08)
    {
      DBG(2, form("KCemu/PIO/3/change/A",
                  "PIO A: RAM PROTECT [%d]\n",
                  ((val >> 3) & 1)));
      memory->protectRAM((val >> 3) & 1);
    }
  if (changed & 0x10)
    {
      DBG(2, form("KCemu/PIO/3/change/A",
                  "PIO A: - [%d]\n",
                  ((val >> 4) & 1)));
    }
  if (changed & 0x20)
    {
      DBG(2, form("KCemu/PIO/3/change/A",
                  "PIO A: LED TAPE [%d]\n",
                  ((val >> 5) & 1)));
    }
  if (changed & 0x40)
     {
       DBG(2, form("KCemu/PIO/3/change/A",
                  "PIO A: TAPE Power [%d]\n",
		   ((val >> 6) & 1)));
       tape->power((val >> 6) & 1);
     }
  if (changed & 0x80)
    {
      DBG(2, form("KCemu/PIO/3/change/A",
                  "PIO A: BASIC ROM [%d]\n",
                  ((val >> 7) & 1)));
      memory->enableBASIC_C(val & 0x80);
    }
}

void PIO3::change_B(byte_t changed, byte_t val)
{
  if (changed & 0x1f)
    {
      DBG(2, form("KCemu/PIO/3/change/B",
		  "PIO B: volume [%d]\n",
		  (val & 0x1f)));
    }
  if (changed & 0x20)
    {
      DBG(2, form("KCemu/PIO/3/change/B",
                  "PIO B: unused [%d]\n",
                  ((val >> 5) & 1)));
    }
  if (changed & 0x40)
    {
      DBG(2, form("KCemu/PIO/3/change/B",
                  "PIO B: unused [%d]\n",
                  ((val >> 6) & 1)));
    }
  if (changed & 0x80)
    {
      DBG(2, form("KCemu/PIO/3/change/B",
		  "PIO B: foreground blink [%d]\n",
		  ((val >> 6) & 1)));
    }
}

void
PIO3::tape_callback(byte_t val)
{
  strobe_A();
}
