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
#include <ctype.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <sys/time.h>

#include <z80ex/z80ex_dasm.h>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/ports.h"
#include "kc/cb_list.h"
#include "kc/z80_fdc.h"

#include "libdbg/dbg.h"

#define MEM_SIZE (65536)
byte_t fdc_mem[MEM_SIZE];

static Z80_FDC *self; // for the signal handler
static void signalHandler(int sig);

Z80_FDC::Z80_FDC(void)
{
  self = this;
  
  _context = z80ex_create(z80ex_mread_cb, this, z80ex_mwrite_cb, this,
                          z80ex_pread_cb, this, z80ex_pwrite_cb, this,
                          z80ex_intread_cb, this);

  z80ex_reset(_context);

  _debug = false;
  signal(SIGINT, signalHandler);
}

Z80_FDC::~Z80_FDC(void) { }

void
Z80_FDC::do_execute(void)
{
  if (_debug)
    {
      int addr = getPC();
      char buf[80];
      int t, t2;
      int base_addr = addr;

      printf("FDC: %04X: ", addr);
      addr += z80ex_dasm(buf, 80, 0, &t, &t2, z80ex_dasm_readbyte_cb, addr, &base_addr);
      printf("%-15s  t=%d", buf, t);
      if (t2) printf("/%d", t2);
      printf("\n");
    }
  _counter += z80ex_step(_context);
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
  _next_irq = irq_vector;
  z80ex_int(_context);
  return 0;
}

void
Z80_FDC::add_callback(unsigned long long offset, Callback *cb, void *data)
{
  _cb_list.add_callback(_counter + offset, cb, data);
}

void
Z80_FDC::register_ic(InterfaceCircuit *h)
{
  _ic_list.push_back(h);
}

void
Z80_FDC::unregister_ic(InterfaceCircuit *h)
{
  _ic_list.remove(h);
}

void
Z80_FDC::reset(bool power_on)
{
  if (power_on)
    memset(fdc_mem, 0, MEM_SIZE);

  z80ex_reset(_context);
  z80ex_set_reg(_context, regPC, 0xfc00);

  _cb_list.clear();

  for (ic_list_t::iterator it = _ic_list.begin(); it != _ic_list.end(); it++)
    (*it)->reset(power_on);
}

void
Z80_FDC::power_on()
{
  reset(true);
}

bool
Z80_FDC::trace(void)
{
  return false;
}

void
Z80_FDC::trace(bool value) { }

Z80EX_BYTE
Z80_FDC::z80ex_dasm_readbyte_cb(Z80EX_WORD addr, void *user_data)
{
  return fdc_mem[(addr & 0xffff)];
}

Z80EX_BYTE
Z80_FDC::z80ex_mread_cb(Z80EX_CONTEXT *cpu, Z80EX_WORD addr, int m1_state, void *user_data)
{
  return fdc_mem[(addr & 0xffff)];
}

void
Z80_FDC::z80ex_mwrite_cb(Z80EX_CONTEXT *cpu, Z80EX_WORD addr, Z80EX_BYTE value, void *user_data)
{
  fdc_mem[(addr & 0xffff)] = value;
}

Z80EX_BYTE
Z80_FDC::z80ex_pread_cb(Z80EX_CONTEXT *cpu, Z80EX_WORD port, void *user_data)
{
  byte_t value = fdc_ports->in(port);
  DBG(3, form("KCemu/Z80FDC/InZ80",
              "InZ80():  %04x: %04x -> %02x\n",
              Z80_GetPC(), port, value));
  return value;
}

void
Z80_FDC::z80ex_pwrite_cb(Z80EX_CONTEXT *cpu, Z80EX_WORD port, Z80EX_BYTE value, void *user_data)
{
  DBG(3, form("KCemu/Z80FDC/OutZ80",
              "OutZ80(): %04x: %04x -> %02x\n",
              Z80_GetPC(), port, value));
  fdc_ports->out(port, value);
}

Z80EX_BYTE
Z80_FDC::z80ex_intread_cb(Z80EX_CONTEXT *cpu, void *user_data)
{
  Z80_FDC *z80 = (Z80_FDC *) user_data;
  return z80->_next_irq;
}

static void
signalHandler(int sig)
{
  static bool flag = false;
  std::cout << "\n *** signal caught (" << sig << ") ***\n\n";
  signal(sig, signalHandler);
  flag = !flag;
  self->_debug = flag;
}
