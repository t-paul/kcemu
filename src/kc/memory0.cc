/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2002 Torsten Paul
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

#include <stdlib.h>

#include <fstream>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/z80.h"
#include "kc/memory0.h"

#include "ui/ui.h"
#include "ui/generic/ui_0.h"

using namespace std;

Memory0::Memory0(void) : Memory()
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
    int            model;
  } *mptr, m[] = {
    { &_m_scr,      "-",    0x0000, 0x10000, 0,             256, 0, 1, -1                    },
    { &_m_ram,    "RAM",    0x0000, 0x04000, &_ram[0x0000],  10, 0, 1, KC_VARIANT_Z1013_01   },
    { &_m_ram,    "RAM",    0x0000, 0x00400, &_ram[0x0000],  10, 0, 1, KC_VARIANT_Z1013_12   },
    { &_m_ram,    "RAM",    0x0000, 0x04000, &_ram[0x0000],  10, 0, 1, KC_VARIANT_Z1013_16   },
    { &_m_ram,    "RAM",    0x0000, 0x10000, &_ram[0x0000],  10, 0, 1, KC_VARIANT_Z1013_64   },
    { &_m_ram,    "RAM",    0x0000, 0x10000, &_ram[0x0000],  10, 0, 1, KC_VARIANT_Z1013_A2   },
    { &_m_ram,    "RAM",    0x0000, 0x10000, &_ram[0x0000],  10, 0, 1, KC_VARIANT_Z1013_RB   },
    { &_m_ram,    "RAM",    0x0000, 0x0ec00, &_ram[0x0000],  10, 0, 1, KC_VARIANT_Z1013_SURL },
    { &_m_ram_f0, "RAM-F0", 0xf000, 0x00800, &_ram[0xf000],  10, 0, 1, KC_VARIANT_Z1013_SURL },
    { &_m_ram_f8, "RAM-F8", 0xf800, 0x00800, &_ram[0xf800],  10, 0, 1, KC_VARIANT_Z1013_SURL },
    { &_m_ram,    "RAM",    0x0000, 0x0ec00, &_ram[0x0000],  10, 0, 1, KC_VARIANT_Z1013_BL4  },
    { &_m_ram_f0, "RAM-F0", 0xf000, 0x00800, &_ram[0xf000],  10, 0, 1, KC_VARIANT_Z1013_BL4  },
    { &_m_ram_f8, "RAM-F8", 0xf800, 0x00800, &_ram[0xf800],  10, 0, 1, KC_VARIANT_Z1013_BL4  },
    { &_m_irm,    "IRM",    0xec00, 0x00400, &_irm[0x0000],   0, 0, 1, -1                    },
    { &_m_rom,    "ROM",    0xf000, 0x00800, &_rom[0x0000],   0, 1, 1, -1                    },
    { &_m_rom_f8, "ROM",    0xf800, 0x00800, &_rom[0x0800],   0, 1, 1, KC_VARIANT_Z1013_RB   },
    { &_m_rom_f8, "ROM",    0xf800, 0x00800, &_rom[0x0800],   0, 1, 1, KC_VARIANT_Z1013_BL4  },
    { 0, },
  };

  _portg = NULL;

  int romsize = 0x0800;
  string datadir(kcemu_datadir);
  string z1013_romdir = datadir + "/roms/z1013";
  string z1013_char_rom = z1013_romdir + "/z1013_zg.rom";
  string z1013_system_rom;

  switch (get_kc_variant())
    {
    case KC_VARIANT_Z1013_A2:
      z1013_system_rom = z1013_romdir + "/z1013_a2.rom";
      break;
    case KC_VARIANT_Z1013_RB:
      romsize = 0x1000;
      z1013_system_rom = z1013_romdir + "/z1013_rb.rom";
      break;
    case KC_VARIANT_Z1013_SURL:
      z1013_system_rom = z1013_romdir + "/z1013_ul.rom";
      _portg = ports->register_ports("MEMORY0", 4, 1, this, 0);
      break;
    case KC_VARIANT_Z1013_BL4:
      romsize = 2432;
      z1013_system_rom = z1013_romdir + "/z1013_bl.rom";
      _portg = ports->register_ports("MEMORY0", 4, 1, this, 0);
      break;
    default:
      z1013_system_rom = z1013_romdir + "/z1013_20.rom";
      break;
    }

  load_rom(z1013_system_rom.c_str(), &_rom, romsize, true);
  load_rom(z1013_char_rom.c_str(), &_chr, 0x1000, true);

  for (mptr = &m[0];mptr->name;mptr++)
    {
      *(mptr->group) = NULL;

      if ((mptr->model >= 0) && (mptr->model != get_kc_variant()))
	continue;

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

Memory0::~Memory0(void)
{
  if (_portg)
    ports->unregister_ports(_portg);

  z80->unregister_ic(this);
}

byte_t
Memory0::memRead8(word_t addr)
{
  return _memrptr[addr >> MemArea::PAGE_SHIFT][addr & MemArea::PAGE_MASK];
}

void
Memory0::memWrite8(word_t addr, byte_t val)
{
  _memwptr[addr >> MemArea::PAGE_SHIFT][addr & MemArea::PAGE_MASK] = val;
}

byte_t *
Memory0::get_irm(void)
{
  return (byte_t *)_irm;
}

byte_t *
Memory0::get_char_rom(void)
{
  if (_val & 0x20)
    return _chr + 2048;

  return _chr;
}

void
Memory0::reset(bool power_on)
{
  out(4, 0);

  if (!power_on)
    return;

  memset(&_ram[0], 0, 0x10000);
  //scratch_mem(&_ram[0], 0x10000);
}

void
Memory0::dumpCore(void)
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

byte_t
Memory0::in(word_t addr)
{
  _val &= 0x90;

  return _val;
}

void
Memory0::out(word_t addr, byte_t val)
{
  port_04(_val ^ val, val);
  _val = val;

}

/*
 *  bit 7 - switch screen mode 32x32 (0) / 64x16 (1)
 *  bit 6 - switch clock frequency 2 MHz (0) / 4 MHz (1)
 *  bit 5 - switch character rom normal (0) / with umlauts, invers (1)
 *  bit 4 - switch monitor rom on (0) / off (1)
 */
void
Memory0::port_04(byte_t changed, byte_t val)
{
  if (changed & 0x80)
    {
      if (ui->get_mode() != UI_0::UI_GENERIC_MODE_GDC)
	{
	  if (val & 0x80)
	    ui->set_mode(UI_0::UI_GENERIC_MODE_Z1013_64x16);
	  else
	    ui->set_mode(UI_0::UI_GENERIC_MODE_Z1013_32x32);
	}
    }

  if (changed & 0x10)
    {
      bool rom = val & 0x10;

      if (_m_rom)
	_m_rom->set_active(!rom);

      if (_m_rom_f8)
	_m_rom_f8->set_active(!rom);

      reload_mem_ptr();
    }

#if 0
  bool ram_f0 = val & 0x04;
  bool ram_f8 = val & 0x02;

  if (_m_ram_f0)
    _m_ram_f0->set_readonly(!ram_f0);

  if (_m_ram_f8)
    _m_ram_f8->set_readonly(!ram_f8);
#endif

}
