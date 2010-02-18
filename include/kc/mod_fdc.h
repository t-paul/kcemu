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

#ifndef __kc_mod_fdc_h
#define __kc_mod_fdc_h

#include "kc/ports.h"
#include "kc/module.h"

class ModuleFDC : public ModuleInterface
{
 public:
  typedef enum {
    FDC_INTERFACE_SCHNEIDER = 0,
    FDC_INTERFACE_KRAMER    = 1,
  } fdc_interface_type_t;

 private:
  int _count;
  PortGroup *_portg;
  ModuleFDC *_master;
  fdc_interface_type_t _fdc_type;

 protected:
  void init(void);
  int get_count(void);
  void set_count(int count);

 public:
  ModuleFDC(ModuleFDC &tmpl);
  ModuleFDC(const char *name, fdc_interface_type_t fdc_type);
  virtual ~ModuleFDC(void);

  /*
   *  ModuleInterface
   */
  virtual ModuleInterface * clone(void);
  virtual void reset(bool power_on = false);
  virtual void m_out(word_t addr, byte_t val);
};

#endif /* __kc_mod_fdc_h */
