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

#ifndef __kc_memory9_h
#define __kc_memory9_h

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/memory.h"

class Memory9 : public Memory
{
 private:
  byte_t _rom_slot0[0x0a000];
  byte_t _rom_slot1[0x04000];
  byte_t _ram_slot2[0x10000];

  MemAreaGroup *_m_scr;                /* scratch memory */
  MemAreaGroup *_m_rom_slot0_page0;    /* ROM Slot 1 0000h - 9fffh */
  MemAreaGroup *_m_rom_slot0_page1;    /* ROM Slot 1 0000h - 9fffh */
  MemAreaGroup *_m_rom_slot0_page2;    /* ROM Slot 1 0000h - 9fffh */
  MemAreaGroup *_m_rom_slot1_page1;    /* ROM Slot 1 4000h - 7fffh */
  MemAreaGroup *_m_ram_slot2_page0;    /* RAM Slot 2 0000h - 3fffh */
  MemAreaGroup *_m_ram_slot2_page1;    /* RAM Slot 2 4000h - 7fffh */
  MemAreaGroup *_m_ram_slot2_page2;    /* RAM Slot 2 8000h - bfffh */
  MemAreaGroup *_m_ram_slot2_page3;    /* RAM Slot 2 c000h - ffffh */

 public:
  Memory9(void);
  virtual ~Memory9(void);
  void dumpCore(void);

  byte_t memRead8(word_t addr);
  void memWrite8(word_t addr, byte_t val);

  byte_t * get_irm(void);
  byte_t * get_char_rom(void);

  virtual void reset(bool power_on = false);
  void set_page(int page, int slot);
};

#endif /* __kc_memory9_h */
