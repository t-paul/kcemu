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

#ifndef __ui_module_h
#define __ui_module_h

#include "kc/system.h"

#include "kc/module.h"

class UI_ModuleInterface
{
 public:
  UI_ModuleInterface(void) {}
  virtual ~UI_ModuleInterface(void) {}

  /**
   *  insert a module into the specified slot
   *  the ModuleInterface that is given here has to be the
   *  template module which is stored in the ModuleList
   *  class (kc/mod_list.c) not the cloned module!
   *
   *  if the ModuleInterface parameter is 0 the currently
   *  active module is removed
   */
  virtual void insert(int slot, ModuleInterface *m) = 0;
  /**
   *  the value given for this activate function is the
   *  module specific byte which is written to port 80h
   *  it comes directly from the CAOS SWITCH command
   */
  virtual void activate(int slot, byte_t value) = 0;
};

#endif /* __ui_module_h */
