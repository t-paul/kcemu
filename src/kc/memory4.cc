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
#include "kc/prefs/prefs.h"

#include "kc/z80.h"
#include "kc/memory4.h"
#include "kc/mod_rom.h"

#include "ui/ui.h"

#include "libdbg/dbg.h"

using namespace std;

Memory4::Memory4(void)
{
  if (Preferences::instance()->get_kc_type() == KC_TYPE_85_4)
    init_4();
  else
    init_5();
}

void
Memory4::init_4(void)
{
  _ram_size = 0x10000;
  _ram = new byte_t[_ram_size];
  _rom_caosc = new byte_t[0x1000];
  _rom_caose = new byte_t[0x2000];
  _rom_basic = new byte_t[0x2000];

  load_rom(SystemROM::ROM_KEY_CAOSC, &_rom_caosc[0]);
  load_rom(SystemROM::ROM_KEY_CAOSE, &_rom_caose[0]);
  load_rom(SystemROM::ROM_KEY_BASIC, &_rom_basic[0]);

  memory_group_t mem[] = {
    { &_m_scr,       "-",             0x0000, 0x10000, 0,            256, 0, 1, -1 },
    { &_m_ram_0,     "RAM 0",         0x0000,  0x4000, &_ram[0x8000],  0, 0, 1, -1 },
    { &_m_ram_4,     "RAM 4",         0x4000,  0x4000, &_ram[0xc000],  4, 0, 1, -1 },
    { &_m_ram_8[ 0], "RAM 8 (0)",     0x8000,  0x4000, &_ram[0x0000],  3, 0, 1, -1 },
    { &_m_ram_8[ 1], "RAM 8 (1)",     0x8000,  0x4000, &_ram[0x4000],  3, 0, 0, -1 },

    { &_m_irm_0p,    "IRM 0 Pixel",   0x8000,  0x2800, &_irm0[0],      1, 0, 1, -1 },
    { &_m_irm_0c,    "IRM 0 Color",   0x8000,  0x2800, &_irm0[0x4000], 1, 0, 0, -1 },
    { &_m_irm_1p,    "IRM 1 Pixel",   0x8000,  0x2800, &_irm1[0],      1, 0, 0, -1 },
    { &_m_irm_1c,    "IRM 1 Color",   0x8000,  0x2800, &_irm1[0x4000], 1, 0, 0, -1 },

    { &_m_irm_0px,   "IRM 0 Pixel *", 0xa800,  0x1800, &_irm0[0x2800], 1, 0, 1, -1 },
    { &_m_irm_0cx,   "IRM 0 Color *", 0xa800,  0x1800, &_irm0[0x6800], 1, 0, 0, -1 },
    { &_m_irm_1px,   "IRM 1 Pixel *", 0xa800,  0x1800, &_irm1[0x2800], 1, 0, 0, -1 },
    { &_m_irm_1cx,   "IRM 1 Color *", 0xa800,  0x1800, &_irm1[0x6800], 1, 0, 0, -1 },

    { &_m_caos_c,    "CAOS c000h",    0xc000,  0x1000, &_rom_caosc[0], 2, 1, 0, -1 },
    { &_m_caos_e,    "CAOS e000h",    0xe000,  0x2000, &_rom_caose[0], 2, 1, 1, -1 },
    { &_m_basic,     "BASIC",         0xc000,  0x2000, &_rom_basic[0], 2, 1, 0, -1 },
    { 0, },
  };
  init_memory_groups(mem);

  for (int a = 2;a < 16;a++)
    _m_ram_8[a] = NULL;

  _m_user[0] = _m_basic;
  _m_user[1] = _m_basic;
  _m_user[2] = _m_basic;
  _m_user[3] = _m_basic;

  reset(true);
  z80->register_ic(this);
}

void
Memory4::init_5(void)
{
  _ram_size = 0x40000;
  _ram = new byte_t[_ram_size];
  _rom_caosc = new byte_t[0x2000];
  _rom_caose = new byte_t[0x2000];
  _rom_basic = new byte_t[0x8000];

  load_rom(SystemROM::ROM_KEY_CAOSC, &_rom_caosc[0]);
  load_rom(SystemROM::ROM_KEY_CAOSE, &_rom_caose[0]);
  load_rom(SystemROM::ROM_KEY_BASIC, &_rom_basic[0]);

  memory_group_t mem[] = {
    { &_m_scr,       "-",             0x0000, 0x10000, 0,                 256, 0, 1, -1 },
    { &_m_ram_0,     "RAM 0",         0x0000,  0x4000, &_ram[0x38000],	    0, 0, 1, -1 },
    { &_m_ram_4,     "RAM 4",         0x4000,  0x4000, &_ram[0x3c000],	    4, 0, 1, -1 },
    { &_m_ram_8[ 0], "RAM 8 (0)",     0x8000,  0x4000, &_ram[0x00000],	    3, 0, 1, -1 },
    { &_m_ram_8[ 1], "RAM 8 (1)",     0x8000,  0x4000, &_ram[0x04000],	    3, 0, 0, -1 },
    { &_m_ram_8[ 2], "RAM 8 (2)",     0x8000,  0x4000, &_ram[0x08000],	    3, 0, 0, -1 },
    { &_m_ram_8[ 3], "RAM 8 (3)",     0x8000,  0x4000, &_ram[0x0c000],	    3, 0, 0, -1 },
    { &_m_ram_8[ 4], "RAM 8 (4)",     0x8000,  0x4000, &_ram[0x10000],	    3, 0, 0, -1 },
    { &_m_ram_8[ 5], "RAM 8 (5)",     0x8000,  0x4000, &_ram[0x14000],	    3, 0, 0, -1 },
    { &_m_ram_8[ 6], "RAM 8 (6)",     0x8000,  0x4000, &_ram[0x18000],	    3, 0, 0, -1 },
    { &_m_ram_8[ 7], "RAM 8 (7)",     0x8000,  0x4000, &_ram[0x1c000],	    3, 0, 0, -1 },
    { &_m_ram_8[ 8], "RAM 8 (8)",     0x8000,  0x4000, &_ram[0x20000],	    3, 0, 0, -1 },
    { &_m_ram_8[ 9], "RAM 8 (9)",     0x8000,  0x4000, &_ram[0x24000],	    3, 0, 0, -1 },
    { &_m_ram_8[10], "RAM 8 (10)",    0x8000,  0x4000, &_ram[0x28000],	    3, 0, 0, -1 },
    { &_m_ram_8[11], "RAM 8 (11)",    0x8000,  0x4000, &_ram[0x2c000],	    3, 0, 0, -1 },
    { &_m_ram_8[12], "RAM 8 (12)",    0x8000,  0x4000, &_ram[0x30000],	    3, 0, 0, -1 },
    { &_m_ram_8[13], "RAM 8 (13)",    0x8000,  0x4000, &_ram[0x34000],	    3, 0, 0, -1 },
    { &_m_ram_8[14], "RAM 8 (14)",    0x8000,  0x4000, &_ram[0x38000],	    3, 0, 0, -1 },
    { &_m_ram_8[15], "RAM 8 (15)",    0x8000,  0x4000, &_ram[0x3c000],	    3, 0, 0, -1 },

    { &_m_irm_0p,    "IRM 0 Pixel",   0x8000,  0x2800, &_irm0[0],           1, 0, 1, -1 },
    { &_m_irm_0c,    "IRM 0 Color",   0x8000,  0x2800, &_irm0[0x4000],	    1, 0, 0, -1 },
    { &_m_irm_1p,    "IRM 1 Pixel",   0x8000,  0x2800, &_irm1[0],           1, 0, 0, -1 },
    { &_m_irm_1c,    "IRM 1 Color",   0x8000,  0x2800, &_irm1[0x4000],	    1, 0, 0, -1 },

    { &_m_irm_0px,   "IRM 0 Pixel *", 0xa800,  0x1800, &_irm0[0x2800],	    1, 0, 1, -1 },
    { &_m_irm_0cx,   "IRM 0 Color *", 0xa800,  0x1800, &_irm0[0x6800],	    1, 0, 0, -1 },
    { &_m_irm_1px,   "IRM 1 Pixel *", 0xa800,  0x1800, &_irm1[0x2800],	    1, 0, 0, -1 },
    { &_m_irm_1cx,   "IRM 1 Color *", 0xa800,  0x1800, &_irm1[0x6800],	    1, 0, 0, -1 },

    { &_m_caos_c,    "CAOS c000h",    0xc000,  0x2000, &_rom_caosc[0],	    2, 1, 0, -1 },
    { &_m_caos_e,    "CAOS e000h",    0xe000,  0x2000, &_rom_caose[0],	    2, 1, 1, -1 },
    { &_m_user[3],   "BASIC",         0xc000,  0x2000, &_rom_basic[0x6000], 2, 1, 0, -1 },
    { &_m_user[2],   "USER 0",        0xc000,  0x2000, &_rom_basic[0x4000], 2, 1, 0, -1 },
    { &_m_user[1],   "USER 1",        0xc000,  0x2000, &_rom_basic[0x2000], 2, 1, 0, -1 },
    { &_m_user[0],   "USER 2",        0xc000,  0x2000, &_rom_basic[0x0000], 2, 1, 0, -1 },
    { 0, },
  };
  init_memory_groups(mem);

  _m_basic = _m_user[3];

  reset(true);
  z80->register_ic(this);
}

Memory4::~Memory4(void)
{
  z80->unregister_ic(this);
}

byte_t
Memory4::memRead8(word_t addr)
{
  return _memrptr[addr >> MemArea::PAGE_SHIFT][addr & MemArea::PAGE_MASK];
}

void
Memory4::memWrite8(word_t addr, byte_t val)
{
  _memwptr[addr >> MemArea::PAGE_SHIFT][addr & MemArea::PAGE_MASK] = val;
}

byte_t *
Memory4::get_irm(void)
{
  return _display_screen1 ? &_irm1[0] : &_irm0[0];
}

byte_t *
Memory4::get_char_rom(void)
{
  return (byte_t *)0;
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

  os.write((char *)_ram,  0x10000);
  os.write((char *)_irm0,   0x8000);
  os.write((char *)_irm1,   0x8000);
  os.close();
}

void
Memory4::configureIRM(void)
{
  if (!_enable_irm)
    {
      _m_irm_0px->set_active(false);
      _m_irm_0cx->set_active(false);
      _m_irm_1px->set_active(false);
      _m_irm_1cx->set_active(false);
      return;
    }

  if (_caos_e || !_caos_c)
    {
      /* default, shared segment in screen 0 / pixel */
      _m_irm_0px->set_active(true);
      _m_irm_0cx->set_active(false);
      _m_irm_1px->set_active(false);
      _m_irm_1cx->set_active(false);
    }
  else
    {
      if (_access_screen1)
	{
	  _m_irm_0px->set_active(false);
	  _m_irm_0cx->set_active(false);
	  _m_irm_1px->set_active(!_access_color);
	  _m_irm_1cx->set_active(_access_color);
	}
      else
	{
	  _m_irm_0px->set_active(!_access_color);
	  _m_irm_0cx->set_active(_access_color);
	  _m_irm_1px->set_active(false);
	  _m_irm_1cx->set_active(false);
	}
    }
}

void
Memory4::enableCAOS_C(int v)
{
  DBG(0, form("KCemu/Memory/4/switch",
              "Memory: CAOS C %s\n",
	      v ? "enabled" : "disabled"));

  _caos_c = v;
  _m_caos_c->set_active(v);
  configureIRM();
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
  configureIRM();
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
	}
      else
	{
	  _m_irm_0p->set_active(!_access_color);
	  _m_irm_0c->set_active(_access_color);
	  _m_irm_1p->set_active(false);
	  _m_irm_1c->set_active(false);
	}
    }
  else
    {
      _m_irm_0p->set_active(false);
      _m_irm_0c->set_active(false);
      _m_irm_1p->set_active(false);
      _m_irm_1c->set_active(false);
    }

  configureIRM();
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
Memory4::configureRAM_8(void)
{
  /*
   *  The lower 4 bits in _block_ram_8 configure the
   *  ram block that is accessed (which includes the
   *  prepared internal extension to 256k RAM.
   *
   *  Bit 8 is set if the RAM is disabled
   *  (PIO port B bit 5)
   *
   */

  for (int a = 0;a < 16;a++)
    if (_m_ram_8[a])
      _m_ram_8[a]->set_active(_block_ram_8 == a);

  reload_mem_ptr();
}

void
Memory4::enableRAM_8(int v)
{
  if (v)
    _block_ram_8 &= 0x0f;
  else
    _block_ram_8 |= 0x80;

  DBG(1, form("KCemu/Memory/4/switch",
              "Memory: RAM 8 BLOCK %s (0x%02x)\n",
	      v ? "enabled" : "disabled", _block_ram_8));

  configureRAM_8();
}

void
Memory4::selectRAM_8(int segment)
{
  /*
   *  http://www.iee.et.tu-dresden.de/~kc-club/02/KCN94-04/KCN94-04-07.HTML
   *
   *  256 kByte on Board ... der erste Schritt zum KC 85/5?
   *  von Mario Leubner
   *
   *  Segment   Block           SWITCH 3 .. (f�r wr/on)
   *     0      RAM8 E (RAM0)   3B (legt RAM0 zus�tzlich auf 8000H)
   *     1      RAM8 F (RAM4)   3F (legt RAM4 zus�tzlich auf 8000H)
   *     2      RAM8 0          03 (bisher auch Block 0)
   *     3      RAM8 1          07 (bisher auch Block 1)
   *     4      RAM8 2          0B (ab hier neu)
   *     5      RAM8 3          0F
   *     6      RAM8 4          13
   *     7      RAM8 5          17
   *     8      RAM8 6          1B
   *     9      RAM8 7          1F
   *    10      RAM8 8          23
   *    11      RAM8 9          27
   *    12      RAM8 A          2B
   *    13      RAM8 B          2F
   *    14      RAM8 C          33
   *    15      RAM8 D          37
   *
   */

  int block = segment - 2;
  if (block < 0)
    block += 16;

  _block_ram_8 = (_block_ram_8 & 0xf0) | (block & 0x0f);

  DBG(1, form("KCemu/Memory/4/switch",
              "Memory: RAM 8 SEGMENT %d BLOCK %d (0x%02x)\n",
	      segment, block, _block_ram_8));

  configureRAM_8();
}

void
Memory4::protectRAM_8(int v)
{
  DBG(0, form("KCemu/Memory/4/switch",
              "Memory: RAM 8 %s\n",
	      v ? "read/write" : "readonly"));

  for (int a = 0;a < 16;a++)
    if (_m_ram_8[a])
      _m_ram_8[a]->set_readonly(!v);

  reload_mem_ptr();
}

void
Memory4::configureROM_C(void)
{
  /*
   *  The lower 2 bits in _block_rom_c configure the
   *  rom block that is accessed.
   *
   *  Bit 8 is set if the ROM is disabled
   *
   */

  for (int a = 0;a < 4;a++)
    if (_m_user[a])
      _m_user[a]->set_active(_block_rom_c == a);

  reload_mem_ptr();
}

void
Memory4::enableBASIC_C(int v)
{
  if (v)
    _block_rom_c &= 0x03;
  else
    _block_rom_c |= 0x80;

  DBG(0, form("KCemu/Memory/4/switch",
              "Memory: BASIC C %s (0x%02x)\n",
	      v ? "enabled" : "disabled", _block_rom_c));

  configureROM_C();
}

void
Memory4::selectROM_C(int segment)
{
  /*
   *
   *  Segment  ROM Block  Program in default ROM
   *    3       BASIC      KC-BASIC
   *    2       USER 1     KC-Debugger (TEMO/REASS)
   *    1       USER 2     EDAS V1.6
   *    0       USER 3     KC-FORTH
   */

  _block_rom_c = (_block_rom_c & 0xfc) | (segment & 0x03);

  DBG(1, form("KCemu/Memory/4/switch",
              "Memory: BASIC C SEGMENT %d (0x%02x)\n",
	      segment, _block_rom_c));

  configureROM_C();
}

void
Memory4::reset(bool power_on)
{
  _caos_c = false;
  _caos_e = true;
  _enable_irm = true;
  _block_ram_8 = 0;
  _block_rom_c = 0x83; // disabled, segment 3 (BASIC)
  _access_color = false;
  _access_screen1 = false;
  _display_screen1 = false;

  _m_ram_0->set_active(true);
  _m_ram_4->set_active(true);
  _m_irm_0p->set_active(true);
  _m_irm_0c->set_active(false);
  _m_irm_1p->set_active(false);
  _m_irm_1c->set_active(false);
  _m_irm_0px->set_active(true);
  _m_irm_0cx->set_active(false);
  _m_irm_1px->set_active(false);
  _m_irm_1cx->set_active(false);
  _m_caos_c->set_active(false);
  _m_caos_e->set_active(true);

  for (int a = 0;a < 16;a++)
    if (_m_ram_8[a])
      _m_ram_8[a]->set_active(a == 0);

  _m_basic->set_active(false);
  for (int a = 0;a < 4;a++)
    if (_m_user[a])
      _m_user[a]->set_active(0);

  reload_mem_ptr();

  if (!power_on)
    return;

  scratch_mem(&_ram[0],_ram_size);
  scratch_mem(&_irm0[0],  0x8000);
  scratch_mem(&_irm1[0],  0x8000);
}
