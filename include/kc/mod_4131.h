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

#ifndef __kc_mod_4131_h
#define __kc_mod_4131_h

#include "kc/pio.h"
#include "kc/module.h"

class ModuleXY4131 : public ModuleInterface, public PIOCallbackInterface
{
 private:
  bool ready;

 public:
  ModuleXY4131(ModuleXY4131 &tmpl);
  ModuleXY4131(const char *name);
  virtual ~ModuleXY4131(void);

  void callback_A_in(void);
  void callback_A_out(byte_t val);

  void callback_B_in(void);
  void callback_B_out(byte_t val);

  virtual void m_out(word_t addr, byte_t val);
  virtual ModuleInterface * clone(void);
};

#endif /* __kc_mod_4131_h */
