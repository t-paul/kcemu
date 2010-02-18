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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/z80.h"
#include "kc/disk_io.h"

#include "libdbg/dbg.h"

FloppyIO::FloppyIO(void)
{
  _bit[0] = _bit[1] = _bit[2] = _bit[3] = 0;
}

FloppyIO::~FloppyIO(void)
{
}

/*
 * BIT   7 6 5 4 3 2 1 0
 *               | | | |
 *               | | |  - Freigabe des Prozessors
 *               | |  --- Setzen des Prozessors in Dauer-RESET
 *               |  ----- Ausgabe RESET-Impuls
 *                ------- Ausgabe NMI-Impuls
 */
byte_t
FloppyIO::in(word_t addr)
{
  byte_t val = _val;

  DBG(2, form("KCemu/FloppyIO/in",
              "FloppyIO::in(): addr = %04x, val = %02x\n",
              addr, val));

  return val;
}

void
FloppyIO::out(word_t addr, byte_t val)
{
  int a;

  DBG(2, form("KCemu/FloppyIO/out",
              "FloppyIO::out(): addr = %04x, val = %02x\n",
              addr, val));

  for (a = 0;a < 3;a++)
    {
      if (val & (1 << a))
        {
          _bit[a] = true;
          /*
           *  Hmm, the manual says activation is done after a 1/0
           *  transition but this doen't really works :-(
           */
          activate(a);
        }
      else
        {
          _bit[a] = false;
        }
    }

  _val = val;
}

void
FloppyIO::activate(int bit_nr)
{
  switch (bit_nr)
    {
    case 0:
      DBG(2, form("KCemu/FloppyIO/activate",
                  "FloppyIO::activate(): bit = %d, start processor\n",
                  bit_nr));
      z80->start_floppy_cpu();
      break;
    case 1:
      DBG(2, form("KCemu/FloppyIO/activate",
                  "FloppyIO::activate(): bit = %d, halt processor\n",
                  bit_nr));
      z80->halt_floppy_cpu(false);
      break;
    case 2:
      DBG(2, form("KCemu/FloppyIO/activate",
                  "FloppyIO::activate(): bit = %d, trigger RESET\n",
                  bit_nr));
      break;
    case 3:
      DBG(2, form("KCemu/FloppyIO/activate",
                  "FloppyIO::activate(): bit = %d, trigger NMI\n",
                  bit_nr));
      break;
    default:
      DBG(0, form("KCemu/warning",
                  "FloppyIO::activate(): bit = %d ???\n",
                  bit_nr));
      break;
    }
}
