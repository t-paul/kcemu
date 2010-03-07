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

#ifndef __kc_mod_ramf_h
#define __kc_mod_ramf_h

#include "kc/ports.h"
#include "kc/module.h"

class ModuleRAMFloppy : public ModuleInterface, public PortInterface
{
 private:
  byte_t          *_ram;
  PortGroup       *_portg;
  ModuleRAMFloppy *_master;

  int             _addr;
  int             _port;
  int             _disk_a;
  int             _disk_b;

 protected:
  void init(void);
  int  allocate_port(void);
  void release_port(int port);
  void read_file(const char *filename, long size);

 public:
  ModuleRAMFloppy(ModuleRAMFloppy &tmpl);
  ModuleRAMFloppy(const char *name);
  virtual ~ModuleRAMFloppy(void);

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

#endif /* __kc_mod_ramf_h */
