/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: z80_fdc.cc,v 1.5 2002/10/31 01:46:36 torsten_paul Exp $
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

#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <sys/time.h>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/ports.h"
#include "kc/cb_list.h"
#include "kc/z80_fdc.h"
#include "libdbg/dbg.h"

byte_t fdc_mem[65536];
int Z80_IRQ = Z80_IGNORE_INT;
static int Z80_IRQ_VECTOR = Z80_IGNORE_INT;

unsigned int
Z80_RDMEM(dword A)
{
  return fdc_mem[(A & 0xffff)];
}

void
Z80_WRMEM(dword A, byte V)
{
  fdc_mem[(A & 0xffff)] = V;

#if 0
  Z80_Regs r;
  Z80_GetRegs(&r);

  cout << "W: "
       << hex << setw(4) << Z80_GetPC()
       << " - HL = " << r.HL.D
       << " (HL) = " << (int)fdc_mem[r.HL.D]
       << endl;
#endif
}

void
Z80_Out(byte Port, byte Value)
{
  DBG(3, form("KCemu/Z80FDC/OutZ80",
	      "OutZ80(): %04x: %04x -> %02x\n",
	      Z80_GetPC(), Port, Value));
  fdc_ports->out(Port, Value);
}

byte
Z80_In(byte Port)
{
  byte_t Value;

  Value = fdc_ports->in(Port);
  DBG(3, form("KCemu/Z80FDC/InZ80",
	      "InZ80():  %04x: %04x -> %02x\n",
	      Z80_GetPC(), Port, Value));
  return Value;
}

void
Z80_Patch(Z80_Regs * /* Regs */)
{
}

int
Z80_Interrupt(void)
{
  int irq = Z80_IRQ_VECTOR;

  if (Z80_IRQ_VECTOR != Z80_IGNORE_INT)
    {
      Z80_IRQ_VECTOR = Z80_IGNORE_INT;
    }

  return irq;
}

void
Z80_Reti(void)
{
}

void
Z80_Retn(void)
{
}

#if 0
static void dump_core(void)
{
  int ok;
  FILE *f;

  printf("dumping floppy cpu memory... ");
  ok = 0;
  f = fopen("core-floppy.z80", "wb");
  if (f)
    {
      if (fwrite(fdc_mem, 0x10000, 1, f) == 1)
	ok = 1;
    }
  if (ok)
    printf("done.\n");
  else
    printf("failed!\n");
}
#endif

Z80_FDC::Z80_FDC(void)
{
  //atexit(dump_core);
  Z80_InitTables();
}

Z80_FDC::~Z80_FDC(void)
{
}

void
Z80_FDC::do_execute(void)
{
#if 0
  Z80_Regs r;
  Z80_GetRegs(&r);

  if (r.PC.D == 0xf4c8)
    {
      cout << "PC: " << hex << setw(4) << setfill('0') << r.PC.D << endl;
      Z80_RegisterDump();
    }
#endif

  Z80_ICount = 0;
  Z80_ExecuteSingle();
  _counter -= Z80_ICount; // ICount is negative!
}

void
Z80_FDC::execute(void)
{
  static int x = 40000;
  static int calls = 8;

  do_execute();
  do_execute();

  if (--calls == 0)
    {
      calls = 8;
      do_execute();
    }

  _cb_list.run_callbacks(_counter);

  if (--x == 0)
    {
      x = 40000;
      if (DBG_check("KCemu/Z80core2/trace"))
	{
	  x = 500;
	  Z80_Trace = 1;
	}
    }
}

long long
Z80_FDC::get_counter()
{
  return _counter;
}

byte_t
Z80_FDC::trigger_irq(byte_t irq_vector)
{
  if (Z80_IRQ_VECTOR == Z80_IGNORE_INT)
    Z80_IRQ_VECTOR = irq_vector;

  return 0;
}

void
Z80_FDC::add_callback(unsigned long long offset, Callback *cb, void *data)
{
  _cb_list.add_callback(_counter + offset, cb, data);
}

void
Z80_FDC::reset(bool power_on)
{
  Z80_Regs r;

  Z80_IPeriod = 0;
  Z80_IRQ = Z80_IGNORE_INT;

  if (power_on)
    memset(fdc_mem, 0, 0xfc00);

  Z80_Reset();
  Z80_GetRegs(&r);
  r.PC.D = 0xfc00;
  Z80_SetRegs(&r);
  Z80_Trace = 0;

  _cb_list.clear();
}

void
Z80_FDC::power_on()
{
  reset(true);
}

bool
Z80_FDC::trace(void)
{
  return Z80_Trace;
}

void
Z80_FDC::trace(bool value)
{
  Z80_Trace = value;
}
