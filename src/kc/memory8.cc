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
#include "kc/memory8.h"

#include "ui/ui.h"

#include "libdbg/dbg.h"

using namespace std;

Memory8::Memory8(void) : Memory()
{
  switch (Preferences::instance()->get_kc_variant())
    {
    case KC_VARIANT_LC80_1k:
      load_rom(SystemROM::ROM_KEY_SYSTEM1, &_rom1);
      load_rom(SystemROM::ROM_KEY_SYSTEM2, &_rom2);
      break;
    case KC_VARIANT_LC80_2k:
      load_rom(SystemROM::ROM_KEY_SYSTEM, &_rom);
      break;
    case KC_VARIANT_LC80e:
      load_rom(SystemROM::ROM_KEY_SYSTEM1, &_rome[0x0000]);
      load_rom(SystemROM::ROM_KEY_SYSTEM2, &_rome[0x1000]);
      load_rom(SystemROM::ROM_KEY_SYSTEM3, &_rome[0x2000]);
      break;
    default:
      DBG(0, form("KCemu/internal_error",
                  "Memory8: invalid kc variant value: %d\n",
                  Preferences::instance()->get_kc_variant()));
      break;
   }

  memory_group_t mem[] = {
    { &_m_scr,   "-",     0x0000, 0x10000, 0,            256, 0, 1, -1                 },
    { &_m_rom1,  "ROM1",  0x0000,  0x0400, &_rom1[0],      0, 1, 1, KC_VARIANT_LC80_1k },
    { &_m_rom2,  "ROM2",  0x0800,  0x0400, &_rom2[0],      0, 1, 1, KC_VARIANT_LC80_1k },
    { &_m_rom,   "ROM",   0x0000,  0x0800, &_rom[0],       0, 1, 1, KC_VARIANT_LC80_2k },
    { &_m_rome1, "ROM1",  0x0000,  0x1000, &_rome[0],      0, 1, 1, KC_VARIANT_LC80e   },
    { &_m_rome2, "ROM2",  0x1000,  0x1000, &_rome[0x1000], 0, 1, 1, KC_VARIANT_LC80e   },
    { &_m_rome5, "ROM5",  0xc000,  0x1000, &_rome[0x2000], 0, 1, 1, KC_VARIANT_LC80e   },
    { &_m_ram,   "RAM",   0x2000,  0x0400, &_ram[0],       0, 0, 1, KC_VARIANT_LC80_1k },
    { &_m_ram,   "RAM",   0x2000,  0x0400, &_ram[0],       0, 0, 1, KC_VARIANT_LC80_2k },
    { &_m_ram,   "RAM",   0x2000,  0x1000, &_ram[0],       0, 0, 1, KC_VARIANT_LC80e   },
    { 0, },
  };
  init_memory_groups(mem);

  reset(true);
  z80->register_ic(this);
}

Memory8::~Memory8(void)
{
  z80->unregister_ic(this);
}

byte_t
Memory8::memRead8(word_t addr)
{
  return _memrptr[addr >> MemArea::PAGE_SHIFT][addr & MemArea::PAGE_MASK];
}

void
Memory8::memWrite8(word_t addr, byte_t val)
{
  _memwptr[addr >> MemArea::PAGE_SHIFT][addr & MemArea::PAGE_MASK] = val;
}

byte_t *
Memory8::get_irm(void)
{
  return (byte_t *)0;
}

byte_t *
Memory8::get_char_rom(void)
{
  return (byte_t *)0;
}

void
Memory8::reset(bool power_on)
{
  if (!power_on)
    return;

  scratch_mem(&_ram[0], 0x1000);
}

void
Memory8::dumpCore(void)
{
}
