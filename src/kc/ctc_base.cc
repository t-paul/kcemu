/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: ctc_base.cc,v 1.2 2001/12/31 14:11:53 torsten_paul Exp $
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
#include "kc/ctc_base.h"

#include "libdbg/dbg.h"

CTC_Base::CTC_Base(void) : CTC()
{
  z80->register_ic(this);
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

byte_t
CTC_Base::trigger_irq(byte_t irq_vector)
{
  if (z80->triggerIrq(irq_vector))
    {
      z80->handleIrq(irq_vector);
      return 1;
    }
  return 0;
}

void
CTC_Base::add_callback(unsigned long long offset, Callback *cb, void *data)
{
  z80->addCallback(offset, cb, data);
}
