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

#include <string.h>
#include <stdlib.h>
#include <fstream>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/z80.h"
#include "kc/memory7.h"

#include "ui/ui.h"

using namespace std;

Memory7::Memory7(void) : Memory()
{
  int l;
  char *ptr;
  kc_variant_t v;
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
    { &_m_scr,   "-",     0x0000, 0x10000, 0,              256, 0, 1 },
    { &_m_ram,   "RAM",   0x0000,  0x4000, &_ram[0],         0, 0, 1 },
    { &_m_basic, "BASIC", 0xc000,  0x2800, &_rom_basic[0],   0, 1, 1 },
    { &_m_os,    "OS",    0xf000,  0x1000, &_rom_os[0],      0, 1, 1 },
    { &_m_irm,   "IRM",   0xec00,  0x0400, &_irm[0x400],     1, 0, 1 },
    /* dummy entry needed for getIRM() */
    { &_m_irm,   "IRM -", 0xe800,  0x0400, &_irm[0],         1, 1, 1 },
    { 0, },
  };
  
  l = strlen(kcemu_datadir);
  ptr = new char[l + 14];
  strcpy(ptr, kcemu_datadir);

  v = get_kc_variant();
  if ((v == KC_VARIANT_87_20) || (v == KC_VARIANT_87_21))
    {
      strcpy(ptr + l, "/os____f0.87b");
      loadROM(ptr, &_rom_os, 0x1000, 1);
      strcpy(ptr + l, "/basic_c0.87b");
      loadROM(ptr, &_rom_basic, 0x2800, 1);
    }
  else
    {
      strcpy(ptr + l, "/os____f0.851");
      loadROM(ptr, &_rom_os, 0x1000, 1);
      strcpy(ptr + l, "/basic_c0.87a");
      loadROM(ptr, &_rom_basic, 0x2800, 1);
    }
  
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
  set_romdi(false);
  register_romdi_handler(this);
  z80->register_ic(this);
}

Memory7::~Memory7(void)
{
  z80->unregister_ic(this);
  unregister_romdi_handler(this);
}

#ifdef MEMORY_SLOW_ACCESS
byte_t
Memory7::memRead8(word_t addr)
{
  return _memrptr[addr >> PAGE_SHIFT][addr & PAGE_MASK];
}

void
Memory7::memWrite8(word_t addr, byte_t val)
{
  _memwptr[addr >> PAGE_SHIFT][addr & PAGE_MASK] = val;
}
#endif /* MEMORY_SLOW_ACCESS */

byte_t *
Memory7::getIRM(void)
{
  return (byte_t *)get_page_addr_r(0xe800);
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
Memory7::reset(bool power_on)
{
  if (!power_on)
    return;

  //scratch_mem(&_ram[0], 0x4000);
  memset(&_ram[0], 0, 0x4000);
  scratch_mem(&_irm[0x0400], 0x0400);
  if (getIRM() != _irm)
    scratch_mem(&_irm[0x0], 0x0400);
  else
    memset(&_irm[0], 0x70, 0x400);
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

void
Memory7::romdi(bool val)
{
  _m_basic->set_active(!val);
}
