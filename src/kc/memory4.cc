/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: memory4.cc,v 1.18 2002/10/31 01:46:35 torsten_paul Exp $
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
#include "kc/rc.h"
#include "kc/z80.h"
#include "kc/memory4.h"
#include "kc/mod_rom.h"

#include "ui/ui.h"

#include "libdbg/dbg.h"

Memory4::Memory4(void) : Memory()
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
    { &_m_scr,    "-",           0x0000, 0x10000, 0,            256, 0, 1 },
    { &_m_ram_0,  "RAM 0",       0x0000,  0x4000, &_ram_0[0],     0, 0, 1 },
    { &_m_ram_4,  "RAM 4",       0x4000,  0x4000, &_ram_4[0],     4, 0, 1 },
    { &_m_ram_8a, "RAM 8 (0)",   0x8000,  0x4000, &_ram_8a[0],    3, 0, 1 },
    { &_m_ram_8b, "RAM 8 (1)",   0x8000,  0x4000, &_ram_8b[0],    3, 0, 0 },

    { &_m_irm_x0, "IRM a800h/0", 0xa800,  0x1800, &_irm0[0x2800], 1, 0, 1 },
    { &_m_irm_x1, "IRM a800h/1", 0xa800,  0x1800, &_irm1[0x2800], 1, 0, 0 },
    { &_m_irm_0p, "IRM 0 Pixel", 0x8000,  0x2800, &_irm0[0],      1, 0, 1 },
    { &_m_irm_0c, "IRM 0 Color", 0x8000,  0x2800, &_irm0[0x4000], 1, 0, 0 },
    { &_m_irm_1p, "IRM 1 Pixel", 0x8000,  0x2800, &_irm1[0],      1, 0, 0 },
    { &_m_irm_1c, "IRM 1 Color", 0x8000,  0x2800, &_irm1[0x4000], 1, 0, 0 },

    { &_m_caos_c, "CAOS c000h",  0xc000,  0x1000, &_rom_caosc[0], 2, 1, 0 },
    { &_m_caos_e, "CAOS e000h",  0xe000,  0x2000, &_rom_caose[0], 2, 1, 1 },
    { &_m_basic,  "BASIC",       0xc000,  0x2000, &_rom_basic[0], 2, 1, 0 },
    { 0, },
  };
  
  l = strlen(kcemu_datadir);
  ptr = new char[l + 14];
  strcpy(ptr, kcemu_datadir);
  
  strcpy(ptr + l, "/caos__c0.854");
  loadROM(ptr, &_rom_caosc, 0x1000, 1);
  strcpy(ptr + l, "/caos__e0.854");
  loadROM(ptr, &_rom_caose, 0x2000, 1);
  strcpy(ptr + l, "/basic_c0.854");
  loadROM(ptr, &_rom_basic, 0x2000, 1);

  delete[] ptr;

  _caos_c = false;
  _caos_e = true;
  _enable_irm = true;
  _access_ram_8_1 = false;
  _access_color = false;
  _access_screen1 = false;
  _display_screen1 = false;
  
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

  loadRAM(RC::instance()->get_string("Load On Init", NULL));
}

Memory4::~Memory4(void)
{
  z80->unregister_ic(this);
}

#ifdef MEMORY_SLOW_ACCESS
byte_t
Memory4::memRead8(word_t addr)
{
  return _memrptr[addr >> PAGE_SHIFT][addr & PAGE_MASK];
}

void
Memory4::memWrite8(word_t addr, byte_t val)
{
  _memwptr[addr >> PAGE_SHIFT][addr & PAGE_MASK] = val;
}
#endif /* MEMORY_SLOW_ACCESS */

byte_t *
Memory4::getIRM(void)
{
  return _display_screen1 ? &_irm1[0] : &_irm0[0];
}

void
Memory4::dumpCore(void)
{
  ofstream os;
    
  os.open("core.z80");
  
  DBG(0, form("KCemu/Memory/4/core",
              "Memory: dumping core...\n"));
  if (!os)
    {
      cerr << "can't write 'core.z80'\n";
      return;
    }
    
  os.write((char *)_ram_0,  0x4000);
  os.write((char *)_ram_4,  0x4000);
  os.write((char *)_ram_8a, 0x4000);
  os.write((char *)_ram_8b, 0x4000);
  os.write((char *)_irm0,   0x8000);
  os.write((char *)_irm1,   0x8000);
  os.close();
}

void
Memory4::enableCAOS_C(int v)
{
  DBG(0, form("KCemu/Memory/4/switch",
              "Memory: CAOS C %s\n",
	      v ? "enabled" : "disabled"));

  _caos_c = v;
  _m_caos_c->set_active(v);
  if (_enable_irm)
    {
      bool x = !_caos_e && _caos_c && _access_screen1;
      _m_irm_x0->set_active(!x);
      _m_irm_x1->set_active(x);
    }

  reload_mem_ptr();
}

void
Memory4::enableCAOS_E(int v)
{
  DBG(0, form("KCemu/Memory/4/switch",
              "Memory: CAOS E %s\n",
	      v ? "enabled" : "disabled"));

  _caos_e = v;
  _m_caos_e->set_active(v);
  if (_enable_irm)
    {
      bool x = !_caos_e && _caos_c && _access_screen1;
      _m_irm_x0->set_active(!x);
      _m_irm_x1->set_active(x);
    }

  reload_mem_ptr();
}

void
Memory4::enableBASIC_C(int v)
{
  DBG(0, form("KCemu/Memory/4/switch",
              "Memory: BASIC C %s\n",
	      v ? "enabled" : "disabled"));
  _m_basic->set_active(v);
  reload_mem_ptr();
}

void
Memory4::enableIRM(int v)
{
  _enable_irm = v;
  if (v)
    {
      if (_access_screen1)
	{
	  _m_irm_0p->set_active(false);
	  _m_irm_0c->set_active(false);
	  _m_irm_1p->set_active(!_access_color);
	  _m_irm_1c->set_active(_access_color);
	  _m_irm_x0->set_active(!(!_caos_e && _caos_c));
	  _m_irm_x1->set_active(!_caos_e && _caos_c);
	}
      else
	{
	  _m_irm_0p->set_active(!_access_color);
	  _m_irm_0c->set_active(_access_color);
	  _m_irm_1p->set_active(false);
	  _m_irm_1c->set_active(false);
	  _m_irm_x0->set_active(true);
	  _m_irm_x1->set_active(false);
	}
    }
  else
    {
      _m_irm_0p->set_active(false);
      _m_irm_0c->set_active(false);
      _m_irm_1p->set_active(false);
      _m_irm_1c->set_active(false);
      _m_irm_x0->set_active(false);
      _m_irm_x1->set_active(false);
    }
  reload_mem_ptr();
}

void
Memory4::enableCOLOR(int v)
{
  _access_color = v;
  enableIRM(_enable_irm);
}

void
Memory4::displaySCREEN_1(int v)
{
  DBG(0, form("KCemu/Memory/4/switch",
              "Memory: display SCREEN %d\n",
	      v));

  _display_screen1 = (v != 0);
}

void
Memory4::enableSCREEN_1(int v)
{
  DBG(0, form("KCemu/Memory/4/switch",
              "Memory: enable SCREEN %d\n",
	      v ? 1 : 0));

  _access_screen1 = v;
  enableIRM(_enable_irm);
}

void
Memory4::enableRAM_0(int v)
{
  DBG(0, form("KCemu/Memory/4/switch",
              "Memory: RAM 0 %s\n",
	      v ? "enabled" : "disabled"));

  _m_ram_0->set_active(v);
  reload_mem_ptr();
}

void
Memory4::protectRAM_0(int v)
{
  DBG(0, form("KCemu/Memory/4/switch",
              "Memory: RAM 0 %s\n",
	      v ? "read/write" : "readonly"));

  _m_ram_0->set_readonly(!v);
  reload_mem_ptr();
}

void
Memory4::enableRAM_4(int v)
{
  DBG(0, form("KCemu/Memory/4/switch",
              "Memory: RAM 4 %s\n",
	      v ? "enabled" : "disabled"));

  _m_ram_4->set_active(v);
  reload_mem_ptr();
}

void
Memory4::protectRAM_4(int v)
{
  DBG(0, form("KCemu/Memory/4/switch",
              "Memory: RAM 4 %s\n",
	      v ? "read/write" : "readonly"));

  _m_ram_4->set_readonly(!v);
  reload_mem_ptr();
}

void
Memory4::enableRAM_8(int v)
{
  DBG(0, form("KCemu/Memory/4/switch",
              "Memory: RAM 8 %s\n",
	      v ? "enabled" : "disabled"));

  if (v)
    {
      _m_ram_8a->set_active(!_access_ram_8_1);
      _m_ram_8b->set_active(_access_ram_8_1);
    }
  else
    {
      _m_ram_8a->set_active(false);
      _m_ram_8b->set_active(false);
    }
  reload_mem_ptr();
}

void
Memory4::enableRAM_8_1(int v)
{
  DBG(0, form("KCemu/Memory/4/switch",
              "Memory: RAM 8 BANK %d\n",
	      v ? 1 : 0));

  _access_ram_8_1 = v;
  _m_ram_8a->set_active(!v);
  _m_ram_8b->set_active(v);
  reload_mem_ptr();
}

void
Memory4::protectRAM_8(int v)
{
  DBG(0, form("KCemu/Memory/4/switch",
              "Memory: RAM 8 %s\n",
	      v ? "read/write" : "readonly"));

  _m_ram_8a->set_readonly(!v);
  _m_ram_8b->set_readonly(!v);
  reload_mem_ptr();
}

void
Memory4::reset(bool power_on)
{
  _m_ram_0->set_active(true);
  _m_ram_4->set_active(true);
  _m_ram_8a->set_active(true);
  _m_ram_8b->set_active(false);
  _m_irm_x0->set_active(true);
  _m_irm_x1->set_active(true);
  _m_irm_0p->set_active(true);
  _m_irm_0c->set_active(false);
  _m_irm_1p->set_active(false);
  _m_irm_1c->set_active(false);
  _m_caos_c->set_active(false);
  _m_caos_e->set_active(true);
  _m_basic->set_active(false);

  if (!power_on)
    return;
  
  scratch_mem(&_ram_0[0],  0x4000);
  scratch_mem(&_ram_4[0],  0x4000);
  scratch_mem(&_ram_8a[0], 0x4000);
  scratch_mem(&_ram_8b[0], 0x4000);
  scratch_mem(&_irm0[0],   0x8000);
  scratch_mem(&_irm1[0],   0x8000);
}
