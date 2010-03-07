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

#ifndef __kc_c80_memory_h
#define __kc_c80_memory_h

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/memory.h"

class MemoryC80 : public Memory
{
private:
  byte_t _rom_monitor[0x0400];
  byte_t _rom_user[0x0400];
  byte_t _ram[0x0400];

  MemAreaGroup *_m_scr;         /* scratch memory               */
  MemAreaGroup *_m_rom_monitor; /* ROM Monitor    0000h - 03ffh */
  MemAreaGroup *_m_rom_user;    /* ROM Monitor    0400h - 07ffh */
  MemAreaGroup *_m_ram_mirror;  /* RAM (mirrored) 0800h - 0bffh */
  MemAreaGroup *_m_ram;         /* RAM            0c00h - 0fffh */

public:
  MemoryC80(void);
  virtual ~MemoryC80(void);
  void dumpCore(void);

  byte_t memRead8(word_t addr);
  void memWrite8(word_t addr, byte_t val);
  
  byte_t * get_irm(void);
  byte_t * get_char_rom(void);

  virtual void reset(bool power_on = false);
};

#endif /* __kc_c80_memory_h */
