/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: ctc1.cc,v 1.4 2001/04/14 15:15:56 tp Exp $
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

#include <iostream.h>
#include <iomanip.h>

#include "kc/config.h"
#include "kc/system.h"

#include "kc/z80.h"
#include "kc/ctc1.h"
#include "kc/tape.h"

#include "libdbg/dbg.h"

CTC1::CTC1(void)
{
  _last_irq_0 = 0;
}

CTC1::~CTC1(void)
{
}

byte_t
CTC1::in(word_t addr)
{
  DBG(2, form("KCemu/CTC/1/in",
              "CTC1::in(): addr = %04x\n",
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
}

void
CTC1::out(word_t addr, byte_t val)
{
  DBG(2, form("KCemu/CTC/1/out",
              "CTC1::out(): addr = %04x, val = %02x\n",
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
CTC1::irq_0(void)
{
  unsigned long diff;
  unsigned long long c;

  c = z80->getCounter();
  diff = (unsigned long)(c - _last_irq_0);
  _last_irq_0 = c;
  if (diff > 3000)
    return false;

  tape->ctcSignal();

  _last_irq_0 = c;

  return false;
}

bool
CTC1::irq_1(void)
{
  return false;
}

bool
CTC1::irq_2(void)
{
  return false;
}

bool
CTC1::irq_3(void)
{
  return false;
}
