/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: memory8.h,v 1.1 2002/03/23 20:04:39 torsten_paul Exp $
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

#ifndef __kc_memory8_h
#define __kc_memory8_h

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/memory.h"

class Memory8 : public Memory
{
private:
  byte_t _ram[0x0400];
  byte_t _rom1[0x0400];
  byte_t _rom2[0x0400];

  MemAreaGroup *_m_scr;     /* scratch memory */
  MemAreaGroup *_m_ram;     /* RAM   2000h - 23ffh */
  MemAreaGroup *_m_rom1;    /* ROM   0000h - 03ffh */
  MemAreaGroup *_m_rom2;    /* ROM   0800h - 0bffh */

public:
  Memory8(void);
  virtual ~Memory8(void);
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

  byte_t * get_irm(void);
  byte_t * get_char_rom(void);

  virtual void reset(bool power_on = false);
};

#endif /* __kc_memory8_h */
