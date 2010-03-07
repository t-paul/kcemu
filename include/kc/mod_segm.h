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

#ifndef __kc_mod_segm_h
#define __kc_mod_segm_h

#include "kc/module.h"
#include "kc/memory.h"

class ModuleSegmentedMemory : public ModuleInterface
{
 private:
  byte_t _val;
  byte_t *_mem;
  bool _is_rom;
  bool _master;
  int _segments;
  word_t _segment_size;
  MemAreaGroup *_group;

 protected:
  virtual bool is_rom(void);

  virtual int get_segment_count(void);
  virtual int get_segment_size(void);

  virtual int      get_segment_index(word_t addr, byte_t val) = 0;
  virtual word_t   get_base_address(word_t addr, byte_t val) = 0;

 public:
  ModuleSegmentedMemory(ModuleSegmentedMemory &tmpl);
  ModuleSegmentedMemory(const char *name, byte_t id, int segments, int segment_size);
  ModuleSegmentedMemory(const char *name, byte_t id, int segments, int segment_size, const char *filename);
  virtual ~ModuleSegmentedMemory(void);

  virtual void m_out(word_t addr, byte_t val);
  virtual ModuleInterface * clone(void) = 0;
  virtual void reset(bool power_on = false);
};

#endif /* __kc_mod_segm_h */
