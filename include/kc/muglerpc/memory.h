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

#ifndef __kc_muglerpc_memory_h
#define __kc_muglerpc_memory_h

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/memory.h"

class MemoryMuglerPC : public Memory
{
private:
  byte_t _irm[0x0800];
  byte_t _rom[0x2000];
  byte_t _rom_chargen[0x0800];
  byte_t _ram_block0[0x10000];
  byte_t _ram_block1[0x10000];
  byte_t _ram_block2[0x10000];

  MemAreaGroup *_m_scr;         /* scratch memory */
  MemAreaGroup *_m_rom;         /* ROM             0000h - 1fffh */
  MemAreaGroup *_m_ram0_lo;     /* RAM Block 0     0000h - ffffh */
  MemAreaGroup *_m_ram0_hi;     /* RAM Block 0     0000h - ffffh */
  MemAreaGroup *_m_ram1;        /* RAM Block 1     0000h - ffffh */
  MemAreaGroup *_m_ram2;        /* RAM Block 2     0000h - ffffh */
  MemAreaGroup *_m_irm;         /* IRM             f800h - ffffh */

public:
  MemoryMuglerPC(void);
  virtual ~MemoryMuglerPC(void);
  void dumpCore(void);

  byte_t memRead8(word_t addr);
  void memWrite8(word_t addr, byte_t val);
  
  byte_t * get_irm(void);
  byte_t * get_char_rom(void);

  void select_eprom(bool active);
  void select_ram_block(int block, bool force_ram0_at_c000h);

  virtual void reset(bool power_on = false);
};

#endif /* __kc_muglerpc_memory_h */
