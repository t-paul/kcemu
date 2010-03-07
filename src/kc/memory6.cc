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
#include "kc/memory6.h"

using namespace std;

Memory6::Memory6(void) : Memory()
{
  if (Preferences::instance()->get_kc_variant() == KC_VARIANT_POLY880_SC1)
    {
      load_rom(SystemROM::ROM_KEY_SYSTEM1, &_rom0);
    }
  else
    {
      load_rom(SystemROM::ROM_KEY_SYSTEM1, &_rom0);
      load_rom(SystemROM::ROM_KEY_SYSTEM2, &_rom1);

      if (!load_rom(SystemROM::ROM_KEY_SYSTEM3, &_rom2))
        memset(_rom2, 0, 0x0400);
      if (!load_rom(SystemROM::ROM_KEY_SYSTEM4, &_rom3))
        memset(_rom3, 0, 0x0400);

      /*
       *  The content of the poly880 rom is stored inverted. We use
       *  the original rom and restore the machine readable code here.
       */
      for (int a = 0;a < 0x0400;a++)
        {
          _rom0[a] = ~_rom0[a];
          _rom1[a] = ~_rom1[a];
          _rom2[a] = ~_rom2[a];
          _rom3[a] = ~_rom3[a];
        }
    }

  memory_group_t mem[] = {
    { &_m_scr,      "-",     0x0000, 0x10000, 0,               256, 0, 1, -1 },
    { &_m_rom0,    "ROM0", 0x0000,  0x0400, &_rom0[0x0000],      0, 1, 1, KC_VARIANT_POLY880 },
    { &_m_rom_sc1, "ROM",  0x0000,  0x1000, &_rom0[0x0000],      0, 1, 1, KC_VARIANT_POLY880_SC1 },
    { &_m_rom1,    "ROM1", 0x1000,  0x0400, &_rom1[0x0000],      0, 1, 1, KC_VARIANT_POLY880 },
    { &_m_rom2,    "ROM2", 0x2000,  0x0400, &_rom2[0x0000],      0, 1, 1, -1 },
    { &_m_rom3,    "ROM3", 0x3000,  0x0400, &_rom3[0x0000],      0, 1, 1, -1 },
    { &_m_ram,     "RAM",  0x4000,  0x0400, &_ram[0x0000],       0, 0, 1, -1 },
    { 0, },
  };
  init_memory_groups(mem);

  reset(true);
  z80->register_ic(this);
}

Memory6::~Memory6(void)
{
  z80->unregister_ic(this);
}

byte_t *
Memory6::get_irm(void)
{
  return (byte_t *)0;
}

byte_t *
Memory6::get_char_rom(void)
{
  return (byte_t *)0;
}

void
Memory6::reset(bool power_on)
{
  if (!power_on)
    return;

  memset(&_ram[0], 0, 0x0400);
}

void
Memory6::dumpCore(void)
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
