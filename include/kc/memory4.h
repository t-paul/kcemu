/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: memory4.h,v 1.10 2002/10/31 01:46:33 torsten_paul Exp $
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

#ifndef __kc_memory4_h
#define __kc_memory4_h

#include "kc/kc.h"
#include "kc/memory.h"

class Memory4 : public Memory
{
private:
  byte_t _ram_0[0x4000];
  byte_t _ram_4[0x4000];
  byte_t _ram_8a[0x4000];
  byte_t _ram_8b[0x4000];

  byte_t _rom_caosc[0x1000];
  byte_t _rom_caose[0x2000];
  byte_t _rom_basic[0x2000];

  byte_t _irm0[0x8000];
  byte_t _irm1[0x8000];

  bool _caos_c;
  bool _caos_e;
  bool _enable_irm;
  bool _access_ram_8_1;
  bool _access_color;
  bool _access_screen1;
  bool _display_screen1;

  MemAreaGroup *_m_scr;     /* scratch memory */
  MemAreaGroup *_m_caos_c;  /* CAOS  c000h - cfffh */
  MemAreaGroup *_m_caos_e;  /* CAOS  e000h - ffffh */
  MemAreaGroup *_m_basic;   /* BASIC c000h - dfffh */
  MemAreaGroup *_m_ram_0;   /* RAM   0000h - 3fffh */
  MemAreaGroup *_m_ram_4;   /* RAM   4000h - 7fffh */
  MemAreaGroup *_m_ram_8a;  /* RAM   8000h - bfffh (page 0) */
  MemAreaGroup *_m_ram_8b;  /* RAM   8000h - bfffh (page 1) */
  MemAreaGroup *_m_irm_x0;  /* IRM   a800h - bfffh (shared) */
  MemAreaGroup *_m_irm_x1;  /* IRM   a800h - bfffh (available on CAOS-C on, CAOS-E off) */
  MemAreaGroup *_m_irm_0p;  /* IRM   8000h - a7ffh (screen 0/ pixel) */
  MemAreaGroup *_m_irm_0c;  /* IRM   8000h - a7ffh (screen 0/ color) */
  MemAreaGroup *_m_irm_1p;  /* IRM   8000h - a7ffh (screen 1/ pixel) */
  MemAreaGroup *_m_irm_1c;  /* IRM   8000h - a7ffh (screen 1/ color) */

public:
  Memory4(void);
  ~Memory4(void);
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

  void enableRAM_0(int v);
  void enableRAM_4(int v);
  void enableRAM_8(int v);
  void enableRAM_8_1(int v);
  void enableCAOS_C(int v);
  void enableCAOS_E(int v);
  void enableBASIC_C(int v);
  void enableIRM(int v);
  void enableCOLOR(int v);
  void enableSCREEN_1(int v);

  void displaySCREEN_1(int v);
  
  void protectRAM_0(int v);
  void protectRAM_4(int v);
  void protectRAM_8(int v);

  virtual void reset(bool power_on = false);
};

#endif /* __kc_memory4_h */
