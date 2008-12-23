/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2008 Torsten Paul
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

#ifndef __kc_mod_z9gr_h
#define __kc_mod_z9gr_h

#include "kc/ports.h"
#include "kc/module.h"

class ModuleZ9001Graphic : public ModuleInterface, PortInterface
{
 private:
  int _count;
  PortGroup *_portg;
  ModuleZ9001Graphic *_master;

  byte_t _addr_low;
  const char *_color_names[8];

 protected:
  void init(void);
  int get_count(void);
  void set_count(int count);

 public:
  ModuleZ9001Graphic(ModuleZ9001Graphic &tmpl);
  ModuleZ9001Graphic(const char *name);
  virtual ~ModuleZ9001Graphic(void);

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

#endif /* __kc_mod_z9gr_h */
