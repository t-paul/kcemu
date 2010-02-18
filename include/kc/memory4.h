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

#ifndef __kc_memory4_h
#define __kc_memory4_h

#include "kc/kc.h"
#include "kc/memory.h"

class Memory4 : public Memory
{
 protected:
  byte_t *_ram;
  int _ram_size;

  byte_t *_rom_caosc;
  byte_t *_rom_caose;
  byte_t *_rom_basic;

  byte_t _irm0[0x8000];
  byte_t _irm1[0x8000];

  byte_t _block_ram_8;
  byte_t _block_rom_c;

  bool _caos_c;
  bool _caos_e;
  bool _enable_irm;
  bool _access_color;
  bool _access_screen1;
  bool _display_screen1;

  MemAreaGroup *_m_scr;       /* scratch memory */
  MemAreaGroup *_m_caos_c;    /* CAOS  c000h - dfffh */
  MemAreaGroup *_m_caos_e;    /* CAOS  e000h - ffffh */
  MemAreaGroup *_m_basic;     /* BASIC c000h - dfffh (for KC85/4) */
  MemAreaGroup *_m_user[4];   /* BASIC c000h - dfffh (and USER0-2 for KC85/5) */
  MemAreaGroup *_m_ram_0;     /* RAM   0000h - 3fffh */
  MemAreaGroup *_m_ram_4;     /* RAM   4000h - 7fffh */
  MemAreaGroup *_m_ram_8[16]; /* RAM   8000h - bfffh (pages 0-15) */
  MemAreaGroup *_m_irm_0p;    /* IRM   8000h - a7ffh (screen 0/ pixel) */
  MemAreaGroup *_m_irm_0c;    /* IRM   8000h - a7ffh (screen 0/ color) */
  MemAreaGroup *_m_irm_1p;    /* IRM   8000h - a7ffh (screen 1/ pixel) */
  MemAreaGroup *_m_irm_1c;    /* IRM   8000h - a7ffh (screen 1/ color) */
  MemAreaGroup *_m_irm_0px;   /* IRM   a800h - bfffh (screen 0 / pixel - shared) */
  MemAreaGroup *_m_irm_0cx;   /* IRM   a800h - bfffh (screen 0 / color - when CAOS-C on, CAOS-E off) */
  MemAreaGroup *_m_irm_1px;   /* IRM   a800h - bfffh (screen 1 / pixel - when CAOS-C on, CAOS-E off) */
  MemAreaGroup *_m_irm_1cx;   /* IRM   a800h - bfffh (screen 1 / color - when CAOS-C on, CAOS-E off) */

 protected:
  virtual void init_4(void);
  virtual void init_5(void);
  virtual void configureRAM_8(void);
  virtual void configureROM_C(void);
  virtual void configureIRM(void);

 public:
  Memory4(void);
  ~Memory4(void);
  void dumpCore(void);

  byte_t memRead8(word_t addr);
  void memWrite8(word_t addr, byte_t val);

  byte_t * get_irm(void);
  byte_t * get_char_rom(void);

  void enableRAM_0(int v);
  void enableRAM_4(int v);
  void enableRAM_8(int v);
  void enableCAOS_C(int v);
  void enableCAOS_E(int v);
  void enableBASIC_C(int v);
  void enableIRM(int v);
  void enableCOLOR(int v);
  void enableSCREEN_1(int v);

  void selectRAM_8(int segment);
  void selectROM_C(int segment);

  void displaySCREEN_1(int v);

  void protectRAM_0(int v);
  void protectRAM_4(int v);
  void protectRAM_8(int v);

  virtual void reset(bool power_on = false);
};

#endif /* __kc_memory4_h */
