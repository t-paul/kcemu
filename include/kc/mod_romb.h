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

#ifndef __kc_mod_romb_h
#define __kc_mod_romb_h

#include "kc/romdi.h"
#include "kc/ports.h"
#include "kc/memory.h"
#include "kc/module.h"

class ModuleROMBank : public ModuleInterface, public PortInterface
{
 private:
  enum {
    ROM_BANK_SIZE = 0x20000,
  };

 private:
  int           _bank;
  byte_t       *_rom;
  MemAreaGroup *_group;
  PortGroup    *_portg;
  
 protected:
  virtual void activate_bank(void);

 public:
  ModuleROMBank(ModuleROMBank &tmpl);
  ModuleROMBank(const char *filename, const char *name);
  virtual ~ModuleROMBank(void);

  /*
   *  ModuleInterface
   */
  virtual ModuleInterface * clone(void);
  virtual void reset(bool power_on = false);
  virtual void m_out(word_t addr, byte_t val);

  /*
   *  PortInterface
   */
  virtual byte_t in(word_t addr);
  virtual void out(word_t addr, byte_t val);
};

#endif /* __kc_mod_romb_h */
