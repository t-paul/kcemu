/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: memory3.cc,v 1.12 2002/06/09 14:24:33 torsten_paul Exp $
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
#include <fstream.h>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/z80.h"
#include "kc/memory3.h"

#include "ui/ui.h"

Memory3::Memory3(void) : Memory()
{
  int l;
  char *ptr;
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
    { &_m_scr,   "-",     0x0000, 0x10000, 0,            256, 0, 1 },
    { &_m_ram,   "RAM 0", 0x0000,  0x4000, &_ram[0],       0, 0, 1 },
    { &_m_irm,   "IRM",   0x8000,  0x4000, &_irm[0],       1, 0, 1 },
    { &_m_caos,  "CAOS",  0xe000,  0x2000, &_rom_caos[0],  2, 1, 1 },
    { &_m_basic, "BASIC", 0xc000,  0x2000, &_rom_basic[0], 2, 1, 1 },
    { 0, },
  };
  
  l = strlen(kcemu_datadir);
  ptr = new char[l + 14];
  strcpy(ptr, kcemu_datadir);
  
  strcpy(ptr + l, "/caos__e0.853");
  loadROM(ptr, &_rom_caos, 0x2000, 1);
  strcpy(ptr + l, "/basic_c0.853");
  loadROM(ptr, &_rom_basic, 0x2000, 1);
  
  _access_color = false;
  
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
}

Memory3::~Memory3(void)
{
  z80->unregister_ic(this);
}

#ifdef MEMORY_SLOW_ACCESS
byte_t
Memory3::memRead8(word_t addr)
{
  return _memrptr[addr >> PAGE_SHIFT][addr & PAGE_MASK];
}

void
Memory3::memWrite8(word_t addr, byte_t val)
{
  _memwptr[addr >> PAGE_SHIFT][addr & PAGE_MASK] = val;
}
#endif /* MEMORY_SLOW_ACCESS */

byte_t *
Memory3::getIRM(void)
{
  return (byte_t *)_irm;
}

void
Memory3::dumpCore(void)
{
#if 0
  ofstream os;
    
  os.open("core.z80");

  cout.form("Memory: dumping core...\n");
  if (!os)
    {
      cerr << "can't write 'core.z80'\n";
      exit(0);
    }
  
  os.write(&_ram[0], 0x4000);
  os.write(&_irm[0],   0x4000);

  os.close();
  cout.form("Memory: done.\n");
#endif
}

void
Memory3::enableCAOS_E(int v)
{
  _m_caos->set_active(v);
  reload_mem_ptr();
}

void
Memory3::enableBASIC_C(int v)
{
  _m_basic->set_active(v);
  reload_mem_ptr();
}

void
Memory3::enableIRM(int v)
{
  _m_irm->set_active(v);
  reload_mem_ptr();
}

void
Memory3::reset(bool power_on)
{
  if (!power_on)
    return;

  scratch_mem(&_ram[0],  0x4000);
  scratch_mem(&_irm[0],  0x4000);
}
