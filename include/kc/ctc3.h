/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: ctc3.h,v 1.5 2001/12/31 14:11:53 torsten_paul Exp $
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

#ifndef __kc_ctc3_h
#define __kc_ctc3_h

#include "kc/system.h"

#include "kc/ctc_base.h"

class CTC3 : public CTC_Base
{
 private:
  enum {
    CHANNEL_2_CLK  = 70000 // 25 Hz
  };

 public:
  CTC3(void);
  virtual ~CTC3(void);

  virtual bool irq_0(void);
  virtual bool irq_1(void);
  virtual bool irq_2(void);
  virtual bool irq_3(void);

  virtual long counter_value_0(void);
  virtual long counter_value_1(void);
  virtual long counter_value_2(void);
  virtual long counter_value_3(void);

  void info(void);
  
  /* PortInterface */
  byte_t in(word_t addr);
  void out(word_t addr, byte_t val);
};

#endif /* __kc_ctc3_h */
