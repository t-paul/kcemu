/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: module.h,v 1.1 2002/06/09 14:24:32 torsten_paul Exp $
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

#ifndef __ui_generic_module_h
#define __ui_generic_module_h

#include "kc/system.h"

#include "ui/module.h"

class DummyModuleHandler : public UI_ModuleInterface
{
 public:
  DummyModuleHandler(void);
  virtual ~DummyModuleHandler(void);

  /*
   *  UI_ModuleInterface
   */
  virtual void insert(int slot, ModuleInterface *m);
  virtual void activate(int slot, byte_t value);
};

#endif /* __ui_generic_module_h */
