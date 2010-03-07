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

#include "kc/z80.h"
#include "kc/ctc_base.h"

#include "libdbg/dbg.h"

CTC_Base::CTC_Base(void) : CTC("CTC")
{
  z80->register_ic(this);
  _z80_irq_mask = z80->get_irq_mask();
}

CTC_Base::~CTC_Base(void)
{
  z80->unregister_ic(this);
}

long long
CTC_Base::get_counter()
{
  return z80->getCounter();
}

void
CTC_Base::irqreq(void)
{
  DBG(2, form("KCemu/CTC/reti",
	      "CTC_Base::irqreq()\n"));
  z80->set_irq_line(_z80_irq_mask);
}

word_t
CTC_Base::irqack(void)
{
  int a, b;
  int vector = IRQ_NOT_ACK;
  
  DBG(2, form("KCemu/CTC/reti",
	      "CTC_Base::irqack(): active: %d %d %d %d - pending: %d %d %d %d\n",
	      _irq_active[0],
	      _irq_active[1],
	      _irq_active[2],
	      _irq_active[3],
	      _irq_pending[0],
	      _irq_pending[1],
	      _irq_pending[2],
	      _irq_pending[3]));

  for (a = 0;a < 4;a++)
    {
      if (_irq_pending[a])
	{
	  _irq_active[a] = 1;
	  _irq_pending[a] = 0;
	  vector = getIRQVector(a);
	  DBG(2, form("KCemu/CTC/reti",
		      "CTC_Base::irqack(): channel = %d, irq_vector = %02xh\n",
		      a, vector));
	  break;
	}
    }

  b = 0;
  for (a = 0;a < 4;a++)
    if (_irq_pending[a])
      b++;

  if (b == 0)
    z80->reset_irq_line(_z80_irq_mask);

  return vector;
}

void
CTC_Base::trigger_irq(int channel)
{
  int a;

  for (a = 0;a < channel;a++)
    if (_irq_active[a])
      return;

  irq();
}

void
CTC_Base::add_callback(unsigned long long offset, Callback *cb, void *data)
{
  z80->addCallback(offset, cb, data);
}
