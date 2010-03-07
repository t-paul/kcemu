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
#include "kc/pio4.h"
#include "kc/tape.h"
#include "kc/memory4.h"

#include "libdbg/dbg.h"

#define memory ((Memory4 *)memory)

PIO4::PIO4(void)
{
}

PIO4::~PIO4(void)
{
}

byte_t
PIO4::in(word_t addr)
{
  DBG(2, form("KCemu/PIO/4/in",
              "PIO4::in(): addr = %04x\n",
              addr));

  switch (addr & 3)
    {
    case 0:
      return in_A_DATA();
    case 1:
      return in_B_DATA();
    case 2:
      return in_A_CTRL();
    case 3:
      return in_B_CTRL();
    }

  return 0; // shouldn't be reached
}

void
PIO4::out(word_t addr, byte_t val)
{
  DBG(2, form("KCemu/PIO/4/out",
              "PIO4::out(): addr = %04x, val = %02x\n",
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
PIO4::change_A(byte_t changed, byte_t val)
{
  if (changed & 0x01)
    {
      DBG(2, form("KCemu/PIO/4/change/A",
                  "PIO A: CAOS ROM E [%d]\n",
                  (val & 1)));
      memory->enableCAOS_E(val & 0x01);
    }
  if (changed & 0x02)
    {
      DBG(2, form("KCemu/PIO/4/change/A",
                  "PIO A: RAM 0 [%d]\n",
                  ((val >> 1) & 1)));
      memory->enableRAM_0(val & 0x02);
    }
  if (changed & 0x04)
    {
      DBG(2, form("KCemu/PIO/4/change/A",
                  "PIO A: IRM [%d]\n",
                  ((val >> 2) & 1)));
      memory->enableIRM(val & 0x04);
    }
  if (changed & 0x08)
    {
      DBG(2, form("KCemu/PIO/4/change/A",
                  "PIO A: write protect RAM 0 [%d]\n",
                  ((val >> 3) & 1)));
      memory->protectRAM_0(val & 0x08);
    }
  if (changed & 0x10)
    {
      DBG(2, form("KCemu/PIO/4/change/A",
                  "PIO A: K OUT (unused) [%d]\n",
                  ((val >> 4) & 1)));
    }
  if (changed & 0x20)
    {
      DBG(2, form("KCemu/PIO/4/change/A",
                  "PIO A: LED [%d]\n",
                  ((val >> 5) & 1)));
    }
  if (changed & 0x40)
    {
      DBG(2, form("KCemu/PIO/4/change/A",
                  "PIO A: TAPE Power [%d]\n",
                  ((val >> 6) & 1)));
      tape->power((val >> 6) & 1);
    }
  if (changed & 0x80)
    {
      DBG(2, form("KCemu/PIO/4/change/A",
                  "PIO A: ROM C (BASIC) [%d]\n",
                  ((val >> 7) & 1)));
      memory->enableBASIC_C(val & 0x80);
    }
}

void
PIO4::change_B(byte_t changed, byte_t val)
{
  if (changed & 0x01)
    DBG(2, form("KCemu/PIO/4/change/B",
                "PIO B: flip-flop [%d]\n",
                (val & 1)));
  if (changed & 0x0e)
    DBG(2, form("KCemu/PIO/4/change/B",
                "PIO B: volume [%d]\n",
                ((val >> 1) & 7)));
  if (changed & 0x10)
    DBG(2, form("KCemu/PIO/4/change/B",
                "PIO B: unused 0x10 [%d]\n",
                ((val >> 4) & 1)));
  if (changed & 0x20)
    {
      DBG(2, form("KCemu/PIO/4/change/B",
                  "PIO B: RAM 8 [%d]\n",
                  ((val >> 5) & 1)));
      memory->enableRAM_8(val & 0x20);
    }
  if (changed & 0x40)
    {
      DBG(2, form("KCemu/PIO/4/change/B",
                  "PIO B: RAM 8 write protect [%d]\n",
                  ((val >> 6) & 1)));
      memory->protectRAM_8(val & 0x40);
    }
  if (changed & 0x80)
    DBG(2, form("KCemu/PIO/4/change/B",
                "PIO B: foreground blink [%d]\n",
                ((val >> 6) & 1)));
}

void
PIO4::tape_callback(byte_t val)
{
  strobe_A();
}
