/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: memory7.cc,v 1.1 2002/10/31 01:46:35 torsten_paul Exp $
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

#include <stdlib.h>
#include <fstream>

#include "kc/system.h"
#include "kc/prefs/prefs.h"

#include "kc/z80.h"
#include "kc/memory7.h"

#include "ui/ui.h"

using namespace std;

Memory7::Memory7(void) : Memory()
{
  struct {
    MemAreaGroup **group;
    const char    *name;
    word_t         addr;
    dword_t        size;
    byte_t        *mem;
    int            prio;
    bool           ro;
    bool           active;
  } *mptr, m[] = {
    { &_m_scr,   "-",     	0x0000, 0x10000, 0,              256, 0, 1 },
    { &_m_ram,   "RAM",   	0x0000,  0x4000, &_ram[0],         0, 0, 1 },
    { &_m_basic, "BASIC", 	0xc000,  0x2800, &_rom_basic[0],   0, 1, 1 },
    { &_m_os,    "OS",    	0xf000,  0x1000, &_rom_os[0],      0, 1, 1 },
    { &_m_irm_ec,"IRM (text)",  0xec00,  0x0400, &_irm[0x400],     1, 0, 1 },
    /*
     *  dummy entry needed for get_irm() if color
     *  expansion is not installed
     */
    { &_m_irm_e8,"IRM (color)", 0xe800,  0x0400, &_irm[0],         1, 1, 1 },
    { 0, },
  };

  string datadir(kcemu_datadir);
  string z9001_romdir = datadir + "/roms/z9001";

  string z9001_os_rom;
  string z9001_basic_rom;
  kc_variant_t v = Preferences::instance()->get_kc_variant();
  if ((v == KC_VARIANT_87_20) || (v == KC_VARIANT_87_21))
    {
      z9001_os_rom = z9001_romdir + "/os____f0.87b";
      z9001_basic_rom = z9001_romdir + "/basic_c0.87b";
    }
  else
    {
      z9001_os_rom = z9001_romdir + "/os____f0.851";
      z9001_basic_rom = z9001_romdir + "/basic_c0.87a";
    }

  load_rom(z9001_os_rom.c_str(), &_rom_os, 0x1000, true);
  load_rom(z9001_basic_rom.c_str(), &_rom_basic, 0x2800, true);

  string z9001_chargen_rom = z9001_romdir + "/chargen.851";
  load_rom(z9001_chargen_rom.c_str(), &_rom_chargen, 0x0800, true);

  memset(&_irm[0], 0x70, 0x400);

  for (mptr = &m[0];mptr->name;mptr++)
    {
      *(mptr->group) = new MemAreaGroup(mptr->name,
					mptr->addr,
					mptr->size,
					mptr->mem,
					mptr->prio,
					mptr->ro);
      (*(mptr->group))->add(get_mem_ptr());
      if (mptr->active)
	(*(mptr->group))->set_active(true);
    }

  reload_mem_ptr();

  reset(true);
  z80->register_ic(this);

  register_romdi_handler(this);
  set_romdi(false);
}

Memory7::~Memory7(void)
{
  z80->unregister_ic(this);
  unregister_romdi_handler(this);

  delete _m_scr;
  delete _m_ram;
  delete _m_basic;
  delete _m_os;
  delete _m_irm_e8;
  delete _m_irm_ec;
}

byte_t
Memory7::memRead8(word_t addr)
{
  for (memory_list_t::iterator it = _memory_list.begin();it != _memory_list.end();it++)
    (*it)->memory_read_byte(addr);

  return _memrptr[addr >> MemArea::PAGE_SHIFT][addr & MemArea::PAGE_MASK];
}

void
Memory7::memWrite8(word_t addr, byte_t val)
{
  for (memory_list_t::iterator it = _memory_list.begin();it != _memory_list.end();it++)
    (*it)->memory_write_byte(addr, val);

  _memwptr[addr >> MemArea::PAGE_SHIFT][addr & MemArea::PAGE_MASK] = val;
}

byte_t *
Memory7::get_irm(void)
{
  return (byte_t *)get_page_addr_r(0xe800);
}

byte_t *
Memory7::get_char_rom(void)
{
  return (byte_t *)_rom_chargen;
}

void
Memory7::register_romdi_handler(ROMDIInterface *handler)
{
  _romdi_list.push_back(handler);
}

void
Memory7::unregister_romdi_handler(ROMDIInterface *handler)
{
  _romdi_list.remove(handler);
}

void
Memory7::set_romdi(bool val)
{
  _romdi = val;
  for (romdi_list_t::iterator it = _romdi_list.begin();it != _romdi_list.end();it++)
    (*it)->romdi(val);

  reload_mem_ptr();
}

void
Memory7::romdi(bool val)
{
  _m_basic->set_active(!val);
}

void
Memory7::register_memory_handler(MemoryInterface *handler)
{
  _memory_list.push_back(handler);
}

void
Memory7::unregister_memory_handler(MemoryInterface *handler)
{
  _memory_list.remove(handler);
}

void
Memory7::reset(bool power_on)
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
Memory7::dumpCore(void)
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
