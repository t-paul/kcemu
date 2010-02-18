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

#ifndef __kc_memory1_h
#define __kc_memory1_h

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/romdi.h"
#include "kc/memory.h"
#include "kc/memoryif.h"

class Memory1 : public Memory, public ROMDIInterface
{
private:
  byte_t _ram[0x4000];
  byte_t _irm[0x0800];
  byte_t _rom_os[0x1000];
  byte_t _rom_chargen[0x0800];

  MemAreaGroup *_m_scr;    /* scratch memory */
  MemAreaGroup *_m_ram;    /* RAM   0000h - 3fffh */
  MemAreaGroup *_m_irm_e8; /* IRM   e800h - bfffh (color, readonly) */
  MemAreaGroup *_m_irm_ec; /* IRM   ec00h - efffh (text) */
  MemAreaGroup *_m_os;     /* OS    f000h - ffffh */

  typedef std::list<ROMDIInterface *> romdi_list_t;
  romdi_list_t _romdi_list;
  bool _romdi;

  typedef std::list<MemoryInterface *> memory_list_t;
  memory_list_t _memory_list;

public:
  Memory1(void);
  virtual ~Memory1(void);
  void dumpCore(void);

  byte_t memRead8(word_t addr);
  void memWrite8(word_t addr, byte_t val);
  
  byte_t * get_irm(void);
  byte_t * get_char_rom(void);

  void set_romdi(bool val);
  void register_romdi_handler(ROMDIInterface *handler);
  void unregister_romdi_handler(ROMDIInterface *handler);

  void register_memory_handler(MemoryInterface *handler);
  void unregister_memory_handler(MemoryInterface *handler);

  virtual void reset(bool power_on = false);

  /*
   *  ROMDIInterface
   */
  void romdi(bool val);
};

#endif /* __kc_memory1_h */
