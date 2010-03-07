/*
 *  KCemu -- The emulator for the KC85 homecomputer series and much more.
 *  Copyright (C) 1997-2010 Torsten Paul
 *  Copyright (C) 2005 Alexander Schön
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

#ifndef __kc_mod_320k_h
#define __kc_mod_320k_h

#include "kc/romdi.h"
#include "kc/module.h"
#include "kc/memory.h"

class Module320k : public ModuleInterface,
  public PortInterface,
  public InterfaceCircuit,
  public ROMDIInterface
{
 private:
  bool _master;
  byte_t _bank;
  byte_t *_rom;
  PortGroup *_portg;
  MemAreaGroup *_group[5];

 protected:
  void register_memory_bank(byte_t val);
  void unregister_memory_bank(void);

 public:
  Module320k(Module320k &tmpl);
  Module320k(const char *filename, const char *name);
  virtual ~Module320k(void);

  virtual void m_out(word_t addr, byte_t val);
  virtual ModuleInterface * clone(void);

  /*
   *  PortInterface
   */
  virtual byte_t in(word_t addr);
  virtual void out(word_t addr, byte_t val);

  /*
   *  InterfaceCircuit
   */
  virtual void reti(void) {}
  virtual void irqreq(void) {}
  virtual word_t irqack() { return IRQ_NOT_ACK; }
  virtual void reset(bool power_on = false);

  /*
   *  ROMDIInterface
   */
  void romdi(bool val);
};

#endif /* __kc_mod_320k_h */
