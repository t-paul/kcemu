/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: memory1.h,v 1.4 2002/10/31 01:46:33 torsten_paul Exp $
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

#ifndef __kc_memory1_h
#define __kc_memory1_h

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/romdi.h"
#include "kc/memory.h"

class Memory1 : public Memory, public ROMDIInterface
{
private:
  byte_t _ram[0x4000];
  byte_t _irm[0x0800];
  byte_t _rom_os[0x1000];

  MemAreaGroup *_m_scr;    /* scratch memory */
  MemAreaGroup *_m_ram;    /* RAM   0000h - 3fffh */
  MemAreaGroup *_m_irm;    /* IRM   e800h - efffh */
  MemAreaGroup *_m_os;     /* OS    f000h - ffffh */

  typedef std::list<ROMDIInterface *> romdi_list_t;
  romdi_list_t _romdi_list;
  bool _romdi;

public:
  Memory1(void);
  virtual ~Memory1(void);
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

  void set_romdi(bool val);
  void register_romdi_handler(ROMDIInterface *handler);
  void unregister_romdi_handler(ROMDIInterface *handler);

  virtual void reset(bool power_on = false);

  /*
   *  ROMDIInterface
   */
  void romdi(bool val);
};

#endif /* __kc_memory1_h */
