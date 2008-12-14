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
#include <fstream>
#include <iostream>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/mod_disk.h"

#include "libdbg/dbg.h"

ModuleDisk::ModuleDisk(ModuleDisk &tmpl) : ModuleROM(tmpl)
{
  _val = 0;
  _slot = tmpl._slot;
}

ModuleDisk::ModuleDisk(const char *rom_key, const char *name, byte_t id, int slot) :
  ModuleROM(rom_key, name, id)
{
  _val = 0;
  _slot = slot;
}

ModuleDisk::~ModuleDisk(void)
{
}

word_t
ModuleDisk::get_addr(byte_t val)
{
  return (val & 0x20) ? 0xe000 : 0xc000;
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
ModuleDisk::m_out(word_t addr, byte_t val)
{
  bool unreg, reg;

  if (((_val & 0x25) ^ (val & 0x25)) == 0)
    return;

  word_t map_addr = get_addr(val);

  DBG(2, form("KCemu/ModuleDisk/out",
	      "ModuleDisk::out(): %s: addr = %04x, val = %02x, old val = %02x\n",
              get_name(), addr, val, _val));

  // The module interface allows us to snoop on slots F8 and FC at
  // the same time. So we can disable the memory of the not accessed
  // slot.
  reg = unreg = false;
  if (((addr >> 8) & 0xfc) != _slot)
    {
      DBG(2, form("KCemu/ModuleDisk/out",
                  "ModuleDisk::out(): %s: I/O to module on other port detected!\n",
                  get_name()));
      reg = false;
      unreg = true;
    }
  else
    {
      if ((_val & 0x20) ^ (val & 0x20))
        {
          DBG(2, form("KCemu/ModuleDisk/out",
                      "ModuleDisk::out(): %s: new map address is %04x\n",
                      get_name(), map_addr));
          reg = unreg = true;
        }

      if ((_val & 0x04) ^ (val & 0x04))
        {
          DBG(2, form("KCemu/ModuleDisk/out",
                      "ModuleDisk::out(): %s: shared memory is now %s\n",
                      get_name(), (val & 0x04) ? "on" : "off"));
        }

      if ((_val & 0x01) ^ (val & 0x01))
        {
          if (val & 1)
            reg = true;
          else
            reg = false;
        }

    }

  if (unreg)
    if (_group)
      {
        if (_group) memory->unregister_memory(_group);
        _group = 0;
      }

  if (reg)
    _group = memory->register_memory(get_name(), map_addr, _size,
                                     _rom, (addr >> 8), true);

  _val = val;
}

ModuleInterface *
ModuleDisk::clone(void)
{
  return new ModuleDisk(*this);
}
