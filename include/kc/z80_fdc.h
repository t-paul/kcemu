/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: z80_fdc.h,v 1.4 2002/10/31 01:46:33 torsten_paul Exp $
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

#ifndef __kc_z80_fdc_h
#define __kc_z80_fdc_h

#include "kc/system.h"

extern "C" {
#include "z80core2/z80.h"
}

#include "kc/ic.h"
#include "kc/cb.h"
#include "kc/cb_list.h"

class Z80_FDC
{
 public:
  unsigned long long _counter;

  CallbackList _cb_list;

 private:
  void do_execute(void);

 public:
  Z80_FDC(void);
  virtual ~Z80_FDC(void);

  void execute(void);
  long long get_counter();
  byte_t trigger_irq(byte_t irq_vector);
  void add_callback(unsigned long long offset, Callback *cb, void *data);

  void reset(bool power_on = false);
  void power_on();
};

#endif /* __kc_z80_fdc_h */
