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

#ifndef __kc_memory6_h
#define __kc_memory6_h

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/memory.h"

class Memory6 : public Memory
{
 private:
  byte_t _rom0[0x1000]; // holds the first poly880 rom or the sc1 chess rom
  byte_t _rom1[0x0400];
  byte_t _rom2[0x0400];
  byte_t _rom3[0x0400];
  byte_t _ram[0x0400];

  MemAreaGroup *_m_scr;     /* scratch memory */
  MemAreaGroup *_m_rom0;    /* ROM   0000h - 03ffh */
  MemAreaGroup *_m_rom1;    /* ROM   1000h - 13ffh */
  MemAreaGroup *_m_rom2;    /* ROM   2000h - 23ffh */
  MemAreaGroup *_m_rom3;    /* ROM   3000h - 33ffh */
  MemAreaGroup *_m_rom_sc1; /* ROM   0000h - 0fffh (SC1 chess rom) */
  MemAreaGroup *_m_ram;     /* RAM   4000h - 43ffh */

 public:
  Memory6(void);
  virtual ~Memory6(void);
  void dumpCore(void);

  inline byte_t memRead8(word_t addr)
    {
      return _memrptr[addr >> MemArea::PAGE_SHIFT][addr & MemArea::PAGE_MASK];
    }

  inline void memWrite8(word_t addr, byte_t val)
    {
      _memwptr[addr >> MemArea::PAGE_SHIFT][addr & MemArea::PAGE_MASK] = val;
    }

  byte_t * get_irm(void);
  byte_t * get_char_rom(void);

  virtual void reset(bool power_on = false);
};

#endif /* __kc_memory6_h */
