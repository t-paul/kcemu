/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id$
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

#ifndef __kc_memory0_h
#define __kc_memory0_h

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/ports.h"
#include "kc/memory.h"

class Memory0 : public Memory, public PortInterface
{
private:
  byte_t _val;
  byte_t _irm[0x00400];
  byte_t _ram[0x10000];
  byte_t _rom[0x01000];
  byte_t _chr[0x01000];

  MemAreaGroup *_m_scr;    /* scratch memory */
  MemAreaGroup *_m_ram;    /* RAM   0000h - efffh */
  MemAreaGroup *_m_ram_f0; /* RAM   f000h - f7ffh */
  MemAreaGroup *_m_ram_f8; /* RAM   f800h - ffffh */
  MemAreaGroup *_m_irm;    /* IRM   ec00h - efffh */
  MemAreaGroup *_m_rom;    /* ROM   f000h - f7ffh */
  MemAreaGroup *_m_rom_f8; /* ROM   f800h - ffffh */

  PortGroup    *_portg;
public:
  Memory0(void);
  virtual ~Memory0(void);
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
  
  void port_04(byte_t changed, byte_t val);

  virtual void reset(bool power_on = false);

  /*
   *  PortInterface
   */
  virtual byte_t in(word_t addr);
  virtual void out(word_t addr, byte_t val);
};

#endif /* __kc_memory0_h */
