/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-1998 Torsten Paul
 *
 *  $Id: memory1.h,v 1.2 2000/05/21 17:33:59 tp Exp $
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

#ifndef __kc_memory1_h
#define __kc_memory1_h

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/memory.h"

class Memory1 : public Memory
{
private:
  byte_t _ram[0x4000];
  byte_t _irm[0x0800];
  byte_t _rom_os[0x1000];
  byte_t _rom_basic[0x2800];

  MemAreaGroup *_m_scr;    /* scratch memory */
  MemAreaGroup *_m_ram;    /* RAM   0000h - 3fffh */
  MemAreaGroup *_m_basic;  /* BASIC c000h - e7ffh */
  MemAreaGroup *_m_irm;    /* IRM   e800h - efffh */
  MemAreaGroup *_m_os;     /* OS    f000h - ffffh */

public:
  Memory1(void);
  virtual ~Memory1(void);
  void dumpCore(void);

#ifndef MEMORY_SLOW_ACCESS
  inline byte_t memRead8(word_t addr)
    {
      return _memrptr[addr >> MemArea::PAGE_SHIFT][addr & MemArea::PAGE_MASK];
    }
  
  inline void memWrite8(word_t addr, byte_t val)
    {
      _memwptr[addr >> MemArea::PAGE_SHIFT][addr & MemArea::PAGE_MASK] = val;
    }
#else /* MEMORY_SLOW_ACCESS */
  byte_t memRead8(word_t addr);
  void memWrite8(word_t addr, byte_t val);
#endif /* MEMORY_SLOW_ACCESS */
  
  byte_t * getIRM(void);

  virtual void reset(bool power_on = false);
};

#endif /* __kc_memory1_h */
