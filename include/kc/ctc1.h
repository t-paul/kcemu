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

#ifndef __kc_ctc1_h
#define __kc_ctc1_h

#include "kc/system.h"

#include "kc/ctc_base.h"

class CTC1 : public CTC_Base
{
 private:
  unsigned long long _last_irq_0;
  
 public:
  CTC1(void);
  virtual ~CTC1(void);

  virtual bool irq_0(void);
  virtual bool irq_1(void);
  virtual bool irq_2(void);
  virtual bool irq_3(void);

  virtual long counter_value_0(void);
  virtual long counter_value_1(void);
  virtual long counter_value_2(void);
  virtual long counter_value_3(void);

  /* PortInterface */
  byte_t in(word_t addr);
  void out(word_t addr, byte_t val);
};

#endif /* __kc_ctc1_h */
