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

#ifndef __kc_mod_4131_h
#define __kc_mod_4131_h

#include "kc/pio.h"
#include "kc/module.h"

class ModuleXY4131 : public ModuleInterface, public PIOCallbackInterface
{
private:
  bool _master;
  byte_t _val;

public:
  ModuleXY4131(ModuleXY4131 &tmpl);
  ModuleXY4131(const char *name);
  virtual ~ModuleXY4131(void);

  /*
   *  PIOCallbackInterface
   */
  virtual int callback_A_in(void);
  virtual int callback_B_in(void);
  virtual void callback_A_out(byte_t val);
  virtual void callback_B_out(byte_t val);

  /*
   *  ModuleInterface
   */
  virtual void m_out(word_t addr, byte_t val);
  virtual ModuleInterface * clone(void);
  virtual void reset(bool power_on = false);
};

#endif /* __kc_mod_4131_h */
