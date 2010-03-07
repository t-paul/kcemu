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
#include "kc/z80.h" //DEBUG
#include "kc/sh_mem.h"

#include "libdbg/dbg.h"

#ifdef ENABLE_COREDUMP
static byte_t *__ptr = 0;
static void dump_core(void)
{
  int ok;
  FILE *f;

  if (__ptr == 0)
    {
      printf("can't dump shared memory!\n");
      return;
    }

  printf("dumping shared memory... ");
  ok = 0;
  f = fopen("core-shmem.z80", "wb");
  if (f)
    {
      if (fwrite(__ptr, 0x0400, 1, f) == 1)
        ok = 1;
    }
  if (ok)
    printf("done.\n");
  else
    printf("failed!\n");
}
#endif /* ENABLE_COREDUMP */

FloppySharedMem::FloppySharedMem(void)
{
#ifdef ENABLE_COREDUMP
  atexit(dump_core);
#endif /* ENABLE_COREDUMP */
}

FloppySharedMem::~FloppySharedMem(void)
{
}

byte_t
FloppySharedMem::in(word_t addr)
{
  byte_t val;

  val = 0xff;
  switch (addr & 0xff)
    {
    case 0xf0:
      val = _mem[((addr >> 8) & 0xff)];
      break;
    case 0xf1:
      val = _mem[((addr >> 8) & 0xff) + 0x0100];
      break;
    case 0xf2:
      val = _mem[((addr >> 8) & 0xff) + 0x0200];
      break;
    case 0xf3:
      val = _mem[((addr >> 8) & 0xff) + 0x0300];
      break;
    case 0xf4:
      break;
    default:
      break;
    }

  DBG(2, form("KCemu/FloppySharedMem/in",
              "FloppySharedMem::in(): addr = %04x, val = %02x\n",
              addr, val));

  return val;
}

void
FloppySharedMem::out(word_t addr, byte_t val)
{
  int idx;

  switch (addr & 0xff)
    {
    case 0xf0:
      idx = ((addr >> 8) & 0xff);
      break;
    case 0xf1:
      idx = ((addr >> 8) & 0xff) | 0x0100;
      break;
    case 0xf2:
      idx = ((addr >> 8) & 0xff) | 0x0200;
      break;
    case 0xf3:
      idx = ((addr >> 8) & 0xff) | 0x0300;
      break;
    default:
      return;
    }

  DBG(2, form("KCemu/FloppySharedMem/out",
	      "FloppySharedMem::out(): %04xh: addr = %04x [=%04x], val = %02x\n",
	      z80->getPC(), addr, idx + 0xfc00, val));

  _mem[idx] = val;
}

void
FloppySharedMem::set_memory(byte_t *mem)
{
  _mem = mem;
#ifdef ENABLE_COREDUMP
  __ptr = mem;
#endif /* ENABLE_COREDUMP */
}
