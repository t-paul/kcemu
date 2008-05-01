/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2004 Torsten Paul
 *
 *  $Id$
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

#ifndef __kc_mod_boot_h
#define __kc_mod_boot_h

#include "kc/mod_rom1.h"
#include "kc/memoryif.h"

class ModuleBOOT : public ModuleROM1, public MemoryInterface
{
 private:
  bool _romdi_handling;

 protected:
  virtual void set_active(bool active);

 public:
  ModuleBOOT(ModuleBOOT &tmpl);
  ModuleBOOT(const char *filename, const char *name,
             word_t addr, dword_t size = 0x4000,
             bool set_romdi = false);
  virtual ~ModuleBOOT(void);

  virtual ModuleInterface * clone(void);
  virtual void reset(bool power_on = false);

  /*
   *  MemoryInterface
   */
  byte_t memory_read_byte(word_t addr);
  void memory_write_byte(word_t addr, byte_t val);
};

#endif /* __kc_mod_boot_h */
