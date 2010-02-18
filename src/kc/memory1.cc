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

#include <stdlib.h>
#include <fstream>

#include "kc/system.h"
#include "kc/prefs/types.h"

#include "kc/kc.h"
#include "kc/z80.h"
#include "kc/memory1.h"

#include "libdbg/dbg.h"

using namespace std;

Memory1::Memory1(void) : Memory()
{
  load_rom(SystemROM::ROM_KEY_SYSTEM, &_rom_os);
  load_rom(SystemROM::ROM_KEY_CHARGEN, &_rom_chargen);

  memset(&_irm[0], 0x70, 0x400);

  memory_group_t mem[] = {
    { &_m_scr,   "-",     	0x0000, 0x10000, 0,              256, 0, 1, -1 },
    { &_m_ram,   "RAM",   	0x0000,  0x4000, &_ram[0],         0, 0, 1, -1 },
    { &_m_os,    "OS",    	0xf000,  0x1000, &_rom_os[0],      0, 1, 1, -1 },
    { &_m_irm_ec,"IRM (text)",  0xec00,  0x0400, &_irm[0x400],     1, 0, 1, -1 },
    /*
     *  dummy entry needed for get_irm() if color
     *  expansion is not installed
     */
    { &_m_irm_e8,"IRM (color)", 0xe800,  0x0400, &_irm[0],         1, 1, 1, -1 },
    { 0, },
  };
  init_memory_groups(mem);

  reset(true);
  z80->register_ic(this);

  register_romdi_handler(this);
  set_romdi(false);
}

Memory1::~Memory1(void)
{
  z80->unregister_ic(this);
  unregister_romdi_handler(this);

  delete _m_scr;
  delete _m_ram;
  delete _m_os;
  delete _m_irm_e8;
  delete _m_irm_ec;
}

byte_t
Memory1::memRead8(word_t addr)
{
  for (memory_list_t::iterator it = _memory_list.begin();it != _memory_list.end();it++)
    (*it)->memory_read_byte(addr);

  return _memrptr[addr >> MemArea::PAGE_SHIFT][addr & MemArea::PAGE_MASK];
}

void
Memory1::memWrite8(word_t addr, byte_t val)
{
  for (memory_list_t::iterator it = _memory_list.begin();it != _memory_list.end();it++)
    (*it)->memory_write_byte(addr, val);

  _memwptr[addr >> MemArea::PAGE_SHIFT][addr & MemArea::PAGE_MASK] = val;
}

byte_t *
Memory1::get_irm(void)
{
  return (byte_t *)get_page_addr_r(0xe800);
}

byte_t *
Memory1::get_char_rom(void)
{
  return (byte_t *)_rom_chargen;
}

void
Memory1::register_romdi_handler(ROMDIInterface *handler)
{
  DBG(1, form("KCemu/Memory1/romdi",
              "Memory1::register_romdi_handler(): %p\n",
              handler));

  _romdi_list.push_back(handler);
}

void
Memory1::unregister_romdi_handler(ROMDIInterface *handler)
{
  DBG(1, form("KCemu/Memory1/romdi",
              "Memory1::unregister_romdi_handler(): %p\n",
              handler));

  _romdi_list.remove(handler);
}

void
Memory1::set_romdi(bool val)
{
  DBG(1, form("KCemu/Memory1/romdi",
              "Memory1::set_romdi(): %s\n",
              val ? "on" : "off"));

  _romdi = val;
  for (romdi_list_t::iterator it = _romdi_list.begin();it != _romdi_list.end();it++)
    (*it)->romdi(val);

  reload_mem_ptr();
}

void
Memory1::romdi(bool val)
{
  DBG(1, form("KCemu/Memory1/romdi",
              "Memory1::romdi(): OS ROM %s\n",
              val ? "off" : "on"));

  _m_os->set_active(!val);
}

void
Memory1::register_memory_handler(MemoryInterface *handler)
{
  _memory_list.push_back(handler);
}

void
Memory1::unregister_memory_handler(MemoryInterface *handler)
{
  _memory_list.remove(handler);
}

void
Memory1::reset(bool power_on)
{
  if (!power_on)
    return;

  scratch_mem(&_ram[0], 0x4000);
  scratch_mem(&_irm[0x0400], 0x0400);
  if (get_irm() != _irm)
    scratch_mem(&_irm[0x0], 0x0400);
  else
    memset(&_irm[0], 0x70, 0x400);

  /*
   *  Clear the first 1k of ram with the system variables.  This saves
   *  some trouble with the initialization.
   */
  memset(&_ram[0], 0, 0x400);

  /*
   *  The CPM-Z9 boot module is enabled/disabled by writing to address
   *  ranges f800h-fbffh/fc00h-ffffh. The delete cursor routine at
   *  fa33h uses the cursor address (2dh/2eh) which is initialized by
   *  using cursor row/column from 2bh/2ch (although after using the
   *  cursor address first).
   *
   *  If the cursor address holds random values we may get memory
   *  writes at addresses that disable the boot module at power on.
   *
   *  To prevent the following initialization to be overwritten in the
   *  startup routine we also need to initialize the EOR (end of ram)
   *  pointer.
   *
   *  How is this supposed to work on the real machine?
   */
  _ram[0x2b] = 0x01; /* column */
  _ram[0x2c] = 0x01; /* row */
  _ram[0x2d] = 0x55; /* cursor address low */
  _ram[0x2e] = 0x55; /* cursor address high */

  _ram[0x36] = 0x00; /* logical ram end low */
  _ram[0x37] = 0xc0; /* logical ram end high */
}

void
Memory1::dumpCore(void)
{
  ofstream os;

  os.open("core.z80");

  cerr << "Memory: dumping core..." << endl;
  if (!os)
    {
      cerr << "Memory: can't write 'core.z80'" << endl;
      return;
    }

  for (int a = 0;a < 0x10000;a++)
    os.put(memRead8(a));

  os.close();
  cerr << "Memory: done." << endl;
}
