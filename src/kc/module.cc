/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: module.cc,v 1.10 2001/04/14 15:16:21 tp Exp $
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

#include <string.h>
#include <malloc.h>
#include <iostream.h>

#include "kc/config.h"
#include "kc/system.h"

#include "kc/kc.h"
#include "kc/module.h"

#include "ui/ui.h"

#include "libdbg/dbg.h"

ModuleInterface::ModuleInterface(const char *name,
                                 byte_t id,
                                 mod_type_t mod_type)
{
  _id = id;
  _valid = false;
  _name = strdup(name);
  _mod_type = mod_type;
}

ModuleInterface::~ModuleInterface(void)
{
  free(_name);
}

byte_t
ModuleInterface::in(word_t addr)
{
  return get_id();
}

byte_t
ModuleInterface::get_id(void)
{
  return _id;
}

const char *
ModuleInterface::get_name(void)
{
  return _name;
}

mod_type_t
ModuleInterface::get_type(void)
{
  return _mod_type;
}

void
ModuleInterface::set_valid(bool valid)
{
  _valid = valid;
}

bool
ModuleInterface::is_valid(void)
{
  return _valid;
}

Module::Module(void)
{
  int a;
  
  for (a = 0;a < NR_MODULES;a++)
    _module[a] = 0;
}

Module::~Module(void)
{
}

byte_t
Module::in(word_t addr)
{
  int a;
  byte_t id;

  if ((addr & 0x300) != 0)
    return 0xff;
  a = (addr >> 8);
  if (a < 8)
    return 0xff;
  a = (a - 8) / 4;
  if (_module[a])
    id = _module[a]->in(addr);
  else
    id = 0xff;

  DBG(2, form("KCemu/Module/in",
              "Module::in():  %04x -> %02x\n",
              addr, id));
  
  return id;
}

void
Module::out(word_t addr, byte_t val)
{
  int a;
  int slot;

  if ((addr & 0x300) != 0)
    return;
  a = (addr >> 8);
  if (a < 8)
    return;
  slot = (a - 8) / 4;
  if (_module[slot])
    {
      DBG(2, form("KCemu/Module/out",
                  "Module::out():  %04x -> %02x\n",
                  addr, val));
      _module[slot]->out(addr, val);
      ui->getModuleInterface()->activate(slot, val);
    }
}

void
Module::insert(byte_t slot, ModuleInterface *module)
{
  remove(slot);
  DBG(0, form("KCemu/Module/insert",
              "inserting module '%s' into slot %X (%d)\n",
              module->get_name(), slot * 4 + 8, slot));
  _module[slot] = module;
}

void
Module::remove(byte_t slot)
{
  if (!_module[slot])
    return;

  DBG(0, form("KCemu/Module/insert",
              "removing module '%s' from slot %X (%d)\n",
              _module[slot]->get_name(), slot * 4 + 8, slot));
  delete _module[slot];
  _module[slot] = 0;
}
