/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: module.h,v 1.3 2001/04/14 15:15:09 tp Exp $
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

#ifndef __ui_module_h
#define __ui_module_h

#include "kc/config.h"
#include "kc/system.h"

#include "kc/module.h"

class UI_ModuleInterface
{
 public:
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
