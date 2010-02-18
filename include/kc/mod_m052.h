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

#ifndef __kc_mod_m052_h
#define __kc_mod_m052_h

#include "kc/mod_segm.h"

class ModuleNetworkUSB : public ModuleSegmentedMemory
{
 private:
  PIO *_pio_net;
  PIO *_pio_vdip;
  PortGroup *_portg_pio_net;
  PortGroup *_portg_pio_vdip;

 protected:
  virtual word_t get_base_address(word_t addr, byte_t val);
  virtual int get_segment_index(word_t addr, byte_t val);

 public:
  ModuleNetworkUSB(ModuleNetworkUSB &tmpl);
  ModuleNetworkUSB(const char *filename, const char *name, byte_t id);
  virtual ~ModuleNetworkUSB(void);

  byte_t m_in(word_t addr);
  void m_out(word_t addr, byte_t val);

  virtual ModuleInterface * clone(void);
};

#endif /* __kc_mod_m052_h */
