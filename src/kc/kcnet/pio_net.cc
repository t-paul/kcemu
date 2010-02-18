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
#include "kc/kcnet/kcnet.h"
#include "kc/kcnet/pio_net.h"

#include "libdbg/dbg.h"

PIO_NET::PIO_NET(void)
{
}

PIO_NET::~PIO_NET(void)
{
}

byte_t
PIO_NET::in(word_t addr)
{
  byte_t val = 0xff;

  switch (addr & 3)
    {
    case 0:
      val = kcnet->read();
      break;
    case 1:
      val = kcnet->get_status();
      break;
    case 2:
      val = in_A_CTRL();
      break;
    case 3:
      val = in_B_CTRL();
      break;
    }

  DBG(2, form("KCemu/PIO/NET/in",
              "PIO_NET::in(): %04xh: addr = %04x, val = %02x\n",
              z80->getPC(), addr, val));

  return val;
}

void
PIO_NET::out(word_t addr, byte_t val)
{
  DBG(2, form("KCemu/PIO/NET/out",
              "PIO_NET::out(): addr = %04x, val = %02x\n",
              addr, val));

  switch (addr & 3)
    {
    case 0:
      DBG(2, form("KCemu/PIO/NET/data_out",
                  "PIO_NET::out(): addr = %04x, val = %02x ('%c')\n",
                  addr, val, isprint(val) ? val : '.'));
      kcnet->write(val);
      break;
    case 1:
      DBG(2, form("KCemu/PIO/NET/ctrl_out",
                  "PIO_NET::out(): addr = %04x, val = %02x, RXF# = %d, TXE# = %d, RD# = %d, WR# = %d, RESET = %d\n",
                  addr, val, val & 1, (val >> 1) & 1, (val >> 2) & 1, (val >> 3) & 1, (val >> 6) & 1));
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
PIO_NET::change_A(byte_t changed, byte_t val)
{
}

void
PIO_NET::change_B(byte_t changed, byte_t val)
{
}
