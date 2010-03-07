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

  byte_t memRead8(word_t addr);
  void memWrite8(word_t addr, byte_t val);
  
  byte_t * get_irm(void);
  byte_t * get_char_rom(void);

  void enableCAOS_E(int v);
  void enableBASIC_C(int v);
  void enableRAM(int v);
  void protectRAM(int v);
  void enableIRM(int v);

  virtual void reset(bool power_on = false);
};

#endif /* __kc_memory3_h */
