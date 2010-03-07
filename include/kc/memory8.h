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

#ifndef __kc_memory8_h
#define __kc_memory8_h

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/memory.h"

class Memory8 : public Memory
{
private:
  byte_t _ram[0x1000]; /* 0x400 for 1k & 2k, 0x1000 for lc80e */
  byte_t _rom[0x0800];
  byte_t _rom1[0x0400];
  byte_t _rom2[0x0400];
  byte_t _rome[0x3000];

  MemAreaGroup *_m_scr;     /* scratch memory */
  MemAreaGroup *_m_ram;     /* RAM   2000h - 23ffh */
  MemAreaGroup *_m_rom;     /* ROM   0000h - 07ffh (2k variant) */
  MemAreaGroup *_m_rom1;    /* ROM   0000h - 03ffh (1k variant) */
  MemAreaGroup *_m_rom2;    /* ROM   0800h - 0bffh (1k variant) */
  MemAreaGroup *_m_rome1;   /* ROM   0000h - 0fffh (LC80e) */
  MemAreaGroup *_m_rome2;   /* ROM   1000h - 1fffh (LC80e) */
  MemAreaGroup *_m_rome5;   /* ROM   c000h - cfffh (LC80e) */
            
public:
  Memory8(void);
  virtual ~Memory8(void);
  void dumpCore(void);

  byte_t memRead8(word_t addr);
  void memWrite8(word_t addr, byte_t val);

  byte_t * get_irm(void);
  byte_t * get_char_rom(void);

  virtual void reset(bool power_on = false);
};

#endif /* __kc_memory8_h */
