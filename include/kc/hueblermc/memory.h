/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2004 Torsten Paul
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

#ifndef __kc_hueblermc_memory_h
#define __kc_hueblermc_memory_h

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/memory.h"

class MemoryHueblerMC : public Memory
{
private:
  byte_t _rom_monitor[0x0c00];
  byte_t _ram[0xe800];
  byte_t _ram_cpu[0x0400];
  byte_t _irm[0x0800];
  byte_t _chargen[0x0c00];

  MemAreaGroup *_m_scr;         /* scratch memory               */
  MemAreaGroup *_m_ram;         /* RAM             0000h - e7ffh */
  MemAreaGroup *_m_irm;         /* IRM             e800h - efffh */
  MemAreaGroup *_m_rom_monitor; /* ROM Monitor     f000h - fbffh */
  MemAreaGroup *_m_ram_cpu;     /* RAM on CPU card fc00h - ffffh */

public:
  MemoryHueblerMC(void);
  virtual ~MemoryHueblerMC(void);
  void dumpCore(void);

  byte_t memRead8(word_t addr);
  void memWrite8(word_t addr, byte_t val);
  
  byte_t * get_irm(void);
  byte_t * get_char_rom(void);

  virtual void reset(bool power_on = false);
};

#endif /* __kc_hueblermc_memory_h */
