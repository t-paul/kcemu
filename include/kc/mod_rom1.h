/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: mod_rom1.h,v 1.1 2002/10/31 01:46:33 torsten_paul Exp $
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
  
public:
  ModuleROM1(ModuleROM1 &tmpl);
  ModuleROM1(const char *filename,
	     const char *name,
	     word_t addr,
	     dword_t size = 0x4000,
	     bool set_romdi = false);
  virtual ~ModuleROM1(void);

  virtual void m_out(word_t addr, byte_t val);
  virtual ModuleInterface * clone(void);

  /*
   *  ROMDIInterface
   */
  void romdi(bool val);
};

#endif /* __kc_mod_rom1_h */
