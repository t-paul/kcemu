/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2004 Torsten Paul
 *
 *  $Id$
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
#include "kc/kramermc/memory.h"

using namespace std;

MemoryKramerMC::MemoryKramerMC(void) : Memory()
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
    { &_m_scr,             "-",               0x0000, 0x10000, 0,                   256, 0, 1 },
    { &_m_rom_io_monitor,  "IO-Monitor",      0x0000,  0x0400, &_rom_io_monitor[0],   1, 1, 1 },
    { &_m_rom_debugger,    "Debugger",        0x0400,  0x0400, &_rom_debugger[0],     1, 1, 1 },
    { &_m_rom_reassembler, "Reassembler",     0x0800,  0x0400, &_rom_reassembler[0],  1, 1, 1 },
    { &_m_ram_0c00h,       "RAM (system)",    0x0c00,  0x0400, &_ram_0c00h[0],        1, 0, 1 },
    { &_m_ram_1000h,       "RAM (statisch)",  0x1000,  0x3000, &_ram_1000h[0],        1, 0, 1 },
    { &_m_ram_4000h,       "RAM (dynamisch)", 0x4000,  0x4000, &_ram_4000h[0],        1, 0, 1 },
    { &_m_rom_basic,       "BASIC",           0x8000,  0x3000, &_rom_basic[0],        1, 1, 1 },
    { &_m_rom_editor,      "Editor",          0xc000,  0x0400, &_rom_editor[0],       1, 1, 1 },
    { &_m_rom_assembler,   "Assembler",       0xc400,  0x1c00, &_rom_assembler[0],    1, 1, 1 },
    { &_m_irm,             "IRM",             0xfc00,  0x0400, &_irm[0],              1, 0, 1 },
    { 0, },
  };

  l = strlen(kcemu_datadir);
  ptr = new char[l + 14];
  strcpy(ptr, kcemu_datadir);

  strcpy(ptr + l, "/io-mon.kmc");
  load_rom(ptr, &_rom_io_monitor, 0x0400, true);

  strcpy(ptr + l, "/debugger.kmc");
  load_rom(ptr, &_rom_debugger, 0x0400, true);

  strcpy(ptr + l, "/reass.kmc");
  load_rom(ptr, &_rom_reassembler, 0x0400, true);

  strcpy(ptr + l, "/basic.kmc");
  load_rom(ptr, &_rom_basic, 0x3000, true);

  strcpy(ptr + l, "/editor.kmc");
  load_rom(ptr, &_rom_editor, 0x0400, true);

  strcpy(ptr + l, "/ass.kmc");
  load_rom(ptr, &_rom_assembler, 0x1c00, true);

  strcpy(ptr + l, "/chargen.kmc");
  load_rom(ptr, &_rom_chargen, 0x0800, true);

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

MemoryKramerMC::~MemoryKramerMC(void)
{
  z80->unregister_ic(this);

  delete _m_scr;
  delete _m_rom_io_monitor;
  delete _m_rom_debugger;
  delete _m_rom_reassembler;
  delete _m_ram_0c00h;
  delete _m_ram_1000h;
  delete _m_ram_4000h;
  delete _m_rom_basic;
  delete _m_rom_editor;
  delete _m_rom_assembler;
  delete _m_irm;
}

byte_t
MemoryKramerMC::memRead8(word_t addr)
{
  return _memrptr[addr >> MemArea::PAGE_SHIFT][addr & MemArea::PAGE_MASK];
}

void
MemoryKramerMC::memWrite8(word_t addr, byte_t val)
{
  _memwptr[addr >> MemArea::PAGE_SHIFT][addr & MemArea::PAGE_MASK] = val;
}

byte_t *
MemoryKramerMC::get_irm(void)
{
  return _irm;
}

byte_t *
MemoryKramerMC::get_char_rom(void)
{
  return _rom_chargen;
}

void
MemoryKramerMC::reset(bool power_on)
{
  if (!power_on)
    return;

  scratch_mem(&_irm[0], 0x0400);
  scratch_mem(&_ram_0c00h[0], 0x0400);
  scratch_mem(&_ram_1000h[0], 0x3000);
  scratch_mem(&_ram_4000h[0], 0x4000);
}

void
MemoryKramerMC::dumpCore(void)
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
