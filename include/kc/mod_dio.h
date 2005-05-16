/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2005 Torsten Paul
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

#ifndef __kc_mod_dio_h
#define __kc_mod_dio_h

#include "kc/module.h"

class ModuleDIO : public ModuleInterface
{
 private:
  CTC *_ctc;
  PIO *_pio;
  PortGroup *_portg_ctc;
  PortGroup *_portg_pio;

 public:
  ModuleDIO(ModuleDIO &tmpl);
  ModuleDIO(const char *name, byte_t id);
  virtual ~ModuleDIO(void);

  //virtual byte_t m_in(word_t addr);
  virtual void m_out(word_t addr, byte_t val);
  virtual ModuleInterface * clone(void);
  virtual void reset(bool power_on = false);
};

#endif /* __kc_mod_dio_h */
