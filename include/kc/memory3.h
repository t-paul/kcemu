/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-1998 Torsten Paul
 *
 *  $Id: memory3.h,v 1.7 2000/05/21 17:34:00 tp Exp $
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

#ifndef __kc_memory3_h
#define __kc_memory3_h

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/memory.h"

class Memory3 : public Memory
{
private:
  byte_t _ram[0x4000];

  byte_t _rom_caos[0x2000];
  byte_t _rom_basic[0x2000];

  byte_t _irm[0x4000];

  bool _access_color;
  
  MemAreaGroup *_m_scr;    /* scratch memory */
  MemAreaGroup *_m_caos;   /* CAOS  e000h - ffffh */
  MemAreaGroup *_m_basic;  /* BASIC c000h - dfffh */
  MemAreaGroup *_m_ram;    /* RAM   0000h - 3fffh */
  MemAreaGroup *_m_irm;    /* IRM   8000h - bfffh */

public:
  Memory3(void);
  virtual ~Memory3(void);
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

  void enableCAOS_E(int v);
  void enableBASIC_C(int v);
  void enableIRM(int v);

  virtual void reset(bool power_on = false);
};

#endif /* __kc_memory3_h */
