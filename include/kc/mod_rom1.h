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

#ifndef __kc_mod_rom1_h
#define __kc_mod_rom1_h

#include "kc/romdi.h"
#include "kc/module.h"
#include "kc/memory.h"

class ModuleROM1 : public ModuleInterface, public ROMDIInterface
{
 private:
  byte_t       *_rom;
  word_t        _addr;
  dword_t       _size;
  MemAreaGroup *_group;
  bool          _set_romdi;
  
 protected:
  void set_romdi(bool val);

 public:
  ModuleROM1(ModuleROM1 &tmpl);
  ModuleROM1(const char *filename,
	     const char *name,
	     word_t addr,
	     dword_t size = 0x4000,
	     bool set_romdi = false);
  virtual ~ModuleROM1(void);

  virtual bool is_active(void);
  virtual void set_active(bool active);

  virtual void m_out(word_t addr, byte_t val);
  virtual ModuleInterface * clone(void);
  virtual void reset(bool power_on = false);

  /*
   *  ROMDIInterface
   */
  void romdi(bool val);
};

#endif /* __kc_mod_rom1_h */
