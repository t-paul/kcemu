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

#ifndef __kc_vcs80_memory_h
#define __kc_vcs80_memory_h

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/memory.h"

class MemoryVCS80 : public Memory
{
private:
  byte_t _rom[0x0200];
  byte_t _ram[0x0400];

  MemAreaGroup *_m_scr; /* scratch memory */
  MemAreaGroup *_m_rom; /* ROM Monitor  0000h - 01ffh */
  MemAreaGroup *_m_ram; /* RAM          0400h - 07ffh */

public:
  MemoryVCS80(void);
  virtual ~MemoryVCS80(void);
  void dumpCore(void);

  byte_t memRead8(word_t addr);
  void memWrite8(word_t addr, byte_t val);
  
  byte_t * get_irm(void);
  byte_t * get_char_rom(void);

  virtual void reset(bool power_on = false);
};

#endif /* __kc_vcs80_memory_h */
