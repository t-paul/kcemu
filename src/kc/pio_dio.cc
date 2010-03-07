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
#include "kc/pio_dio.h"

#include "libdbg/dbg.h"

PIO_DIO::PIO_DIO(void)
{
}

PIO_DIO::~PIO_DIO(void)
{
}

byte_t
PIO_DIO::in(word_t addr)
{
  DBG(2, form("KCemu/PIO/DIO/in",
              "PIO_DIO::in(): addr = %04x\n",
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
PIO_DIO::out(word_t addr, byte_t val)
{
  DBG(2, form("KCemu/PIO/DIO/out",
              "PIO_DIO::out(): addr = %04x, val = %02x\n",
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
PIO_DIO::change_A(byte_t changed, byte_t val)
{
}

void
PIO_DIO::change_B(byte_t changed, byte_t val)
{
}
