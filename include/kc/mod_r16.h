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

#ifndef __kc_mod_r16_h
#define __kc_mod_r16_h

#include "kc/mod_segm.h"

class ModuleSegmentedROM16k : public ModuleSegmentedMemory
{
 protected:
  virtual word_t get_base_address(word_t addr, byte_t val);
  virtual int get_segment_index(word_t addr, byte_t val);

 public:
  ModuleSegmentedROM16k(ModuleSegmentedROM16k &tmpl);
  ModuleSegmentedROM16k(const char *filename, const char *name, byte_t id);
  virtual ~ModuleSegmentedROM16k(void);

  virtual ModuleInterface * clone(void);
};

#endif /* __kc_mod_r16_h */
