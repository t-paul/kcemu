/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
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

#ifndef __kc_mod_192k_h
#define __kc_mod_192k_h

#include "kc/module.h"
#include "kc/memory.h"

class Module192k : public ModuleInterface, public PortInterface, public InterfaceCircuit
{
 private:
  byte_t _val;
  bool _master;

  byte_t *_ram_D1; /* 32k SRAM  62256 */
  byte_t *_rom_D2; /* 64k EPROM 27512 */
  byte_t *_rom_D3; /* 32k EPROM 27256 */
  byte_t *_ram_D4; /* 32k SRAM  62256 */
  byte_t *_rom_D5; /*  8k EPROM 2764  */
  byte_t *_ram_D6; /*  8k SRAM  5164  */

  PortGroup *_portg;

  MemAreaGroup *_m_4000; /* 4k RAM (D1) switchable by S1 */
  MemAreaGroup *_m_6000; /* 4k RAM (D1) switchable by S2 */
  MemAreaGroup *_m_8000; /* 4k RAM (D1) switchable by S3 */
  MemAreaGroup *_m_a000; /* 4k RAM (D1) switchable by S4 */
  MemAreaGroup *_m_c000[16];
  MemAreaGroup *_m_e000[16];

 protected:
  void init(void);
  bool load_rom(const char *filename, byte_t *buf, int size);

 public:
  Module192k(Module192k &tmpl);
  Module192k(const char *d2, const char *d3, const char *d5, const char *name);
  virtual ~Module192k(void);

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
};

#endif /* __kc_mod_192k_h */
