/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: mod_disk.cc,v 1.3 2002/06/09 14:24:33 torsten_paul Exp $
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
#include <fstream.h>
#include <iostream.h>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/mod_disk.h"

#include "libdbg/dbg.h"

ModuleDisk::ModuleDisk(ModuleDisk &tmpl) : ModuleROM(tmpl)
{
  _addr = 0;
}

ModuleDisk::ModuleDisk(const char *filename, const char *name,
                     dword_t size, byte_t id) :
  ModuleROM(filename, name, size, id, 0)
{
  _addr = 0;
}

ModuleDisk::~ModuleDisk(void)
{
}

/*
 *  SWITCH FC kk-------------------
 *                                |
 *                                |
 *          ---------------------------------
 *   Bit    | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 *          ---------------------------------
 *                    |           |       |          Bit  1/0
 *                    |           |       ---- ROM       on/off
 *                    |           ----------- Kopplung   on/off
 *                    ------------------- Basisadresse E000/C000
 *  
 */
void
ModuleDisk::out(word_t addr, byte_t val)
{
  bool unreg, reg;

  if (((_val & 0x25) ^ (val & 0x25)) == 0)
    return;

  if (val & 0x20)
    _addr = 0xe000;
  else
    _addr = 0xc000;

  DBG(1, form("KCemu/ModuleDisk/out",
	      "ModuleDisk::out(): addr = %04x, val = %02x, old val = %02x\n",
              addr, val, _val));

  reg = unreg = false;
  if ((_val & 0x20) ^ (val & 0x20))
    {
      DBG(1, form("KCemu/ModuleDisk/out",
                  "ModuleDisk::out(): new map address is %04x\n",
                  _addr));
      reg = unreg = true;
    }

  if ((_val & 0x04) ^ (val & 0x04))
    {
      DBG(1, form("KCemu/ModuleDisk/out",
                  "ModuleDisk::out(): shared memory is now %s\n",
                  (val & 0x04) ? "on" : "off"));
    }

  if ((_val & 0x01) ^ (val & 0x01))
    {
      if (val & 1)
        reg = true;
      else
        reg = false;
    }

  if (unreg)
    if (_group)
      {
        if (_group) memory->unregister_memory(_group);
        _group = 0;
      }

  if (reg)
    _group = memory->register_memory(get_name(), _addr, _size,
                                     _rom, (addr >> 8), true);

  _val = val;
}

ModuleInterface *
ModuleDisk::clone(void)
{
  return new ModuleDisk(*this);
}
