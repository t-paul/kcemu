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

#ifndef __kc_kramermc_memory_h
#define __kc_kramermc_memory_h

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/memory.h"

class MemoryKramerMC : public Memory
{
private:
  byte_t _irm[0x0400];
  byte_t _ram_0c00h[0x0400];
  byte_t _ram_1000h[0x3000];
  byte_t _ram_4000h[0x4000];
  byte_t _rom_io_monitor[0x0400];
  byte_t _rom_debugger[0x0400];
  byte_t _rom_reassembler[0x0400];
  byte_t _rom_basic[0x3000];
  byte_t _rom_editor[0x0400];
  byte_t _rom_assembler[0x1c00];
  byte_t _rom_chargen[0x0800];

  MemAreaGroup *_m_scr;               /* scratch memory */
  MemAreaGroup *_m_rom_io_monitor;    /* ROM IO-Monitor  0000h - 03ffh */
  MemAreaGroup *_m_rom_debugger;      /* ROM Debugger    0400h - 07ffh */
  MemAreaGroup *_m_rom_reassembler;   /* ROM Reassembler 0800h - 0bffh */
  MemAreaGroup *_m_ram_0c00h;         /* RAM system      0c00h - 0fffh */
  MemAreaGroup *_m_ram_1000h;         /* RAM statisch    1000h - 3fffh */
  MemAreaGroup *_m_ram_4000h;         /* RAM dynamisch   4000h - 7fffh */
  MemAreaGroup *_m_rom_basic;         /* ROM BASIC       8000h - afffh */
  MemAreaGroup *_m_rom_editor;        /* ROM Editor      c000h - c3ffh */
  MemAreaGroup *_m_rom_assembler;     /* ROM Assembler   c400h - dfffh */
  MemAreaGroup *_m_irm;               /* IRM             fc00h - ffffh */

public:
  MemoryKramerMC(void);
  virtual ~MemoryKramerMC(void);
  void dumpCore(void);

  byte_t memRead8(word_t addr);
  void memWrite8(word_t addr, byte_t val);
  
  byte_t * get_irm(void);
  byte_t * get_char_rom(void);

  virtual void reset(bool power_on = false);
};

#endif /* __kc_kramermc_memory_h */
