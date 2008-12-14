/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: mod_rom.h,v 1.5 2002/10/31 01:46:33 torsten_paul Exp $
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

#ifndef __kc_mod_rom_h
#define __kc_mod_rom_h

#include "kc/module.h"
#include "kc/memory.h"

class ModuleROM : public ModuleInterface
{
protected:
  byte_t _id;
  byte_t _val;
  dword_t _size;
  byte_t *_rom;
  MemAreaGroup *_group;
  
public:
  ModuleROM(ModuleROM &tmpl);
  ModuleROM(byte_t *rom, const char *name, dword_t size, byte_t id);
  ModuleROM(const char *rom_key, const char *name, byte_t id);
  ModuleROM(const char *filename, const char *name, dword_t size, byte_t id);
  virtual ~ModuleROM(void);

  virtual word_t get_addr(byte_t val);
  virtual dword_t get_size(byte_t val);
  virtual byte_t * get_rom_ptr(byte_t val);

  virtual void m_out(word_t addr, byte_t val);
  virtual ModuleInterface * clone(void);
  virtual void reset(bool power_on = false);
};

#endif /* __kc_mod_rom_h */
