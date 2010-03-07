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
#include "kc/prefs/prefs.h"

#include "kc/z80.h"
#include "kc/daisy.h"
#include "kc/timer.h"
#include "kc/ports.h"
#include "kc/memory.h"
#include "kc/z80_fdc.h"
#include "kc/cb_list.h"

#include "sys/sysdep.h"

#include "cmd/cmd.h"

#include "ui/ui.h"

#include "libdbg/dbg.h"

using namespace std;

static Z80 *self; // for the signal handler
static void signalHandler(int sig);

#if 0
unsigned long long rdtsc(void)
{
  union {
    struct {
      unsigned long lo;
      unsigned long hi;
    } l;
    unsigned long long ll;
  } ret;

  __asm__ volatile ("rdtsc" : "=a" (ret.l.lo), "=d" (ret.l.hi));

  return ret.ll;
}
#endif

class CMD_single_step : public CMD
{
private:
  Z80 *_z80;
public:
  CMD_single_step(Z80 *z80) : CMD("z80-single-step")
    {
      _z80 = z80;
      register_cmd("z80-single-step-on", 0);
      register_cmd("z80-single-step-off", 1);
      register_cmd("z80-single-step-toggle", 2);
      register_cmd("z80-execute-step", 3);
      register_cmd("z80-trace-on", 4);
      register_cmd("z80-trace-off", 5);
      register_cmd("z80-trace-toggle", 6);
      register_cmd("z80-trace-set-delay", 7);
    }

  void execute(CMD_Args *args, CMD_Context context)
    {
      switch (context)
        {
        case 0:
	  z80->singlestep(true);
	  break;
        case 1:
	  z80->singlestep(false);
	  break;
        case 2:
	  z80->singlestep(!z80->singlestep());
	  break;
	case 3:
	  z80->executestep();
	  break;
	case 4:
	  z80->trace(true);
	  break;
	case 5:
	  z80->trace(false);
	  break;
	case 6:
	  z80->trace(!z80->trace());
	  break;
	case 7:
	  if (args)
	    {
	      long delay = 1000 * args->get_long_arg("delay");
	      z80->tracedelay(delay);
	    }
	  break;
	}
    }
};

Z80::Z80(void)
{
  self = this;

  _context = z80ex_create(z80ex_mread_cb, this, z80ex_mwrite_cb, this,
                           z80ex_pread_cb, this, z80ex_pwrite_cb, this,
                           z80ex_intread_cb, this);

  z80ex_set_reti_callback(_context, z80ex_reti_cb, this);
  
  z80ex_reset(_context);

  const EmulationType &emulation_type = Preferences::instance()->get_system_type()->get_emulation_type();
  z80ex_set_reg(_context, regPC, emulation_type.get_power_on_addr());

  /*
   *  FIXME: at least z1013 emulation breaks with the stackpointer
   *  FIXME: initialized with 0xf000; the CP/M bootlader BL4 will
   *  FIXME: overwrite it's own stack when clearing the screen :-(
   */
  z80ex_set_reg(_context, regSP, 0x0000);

  _counter = 0;

  _debug = false;
  _trace = false;
  _singlestep = false;
  _executestep = false;
  _enable_floppy_cpu = false;

  _tracedelay = 100000;

  _irq_line = 0;
  _irq_mask = 1;

  _do_quit = false;
}

#if 0
static void
print_regs(_Z80 *r)
{
  int c1, c2;

  z80->printPC();

  cout << "a="    << setw(2) << setfill('0') << hex << r->AF.B.l
       << ", bc=" << setw(4) << setfill('0') << hex << r->BC.W
       << ", de=" << setw(4) << setfill('0') << hex << r->DE.W
       << ", hl=" << setw(4) << setfill('0') << hex << r->HL.W
       << endl;

  c1 = RdZ80(r->DE.W) & 0xff;
  c2 = RdZ80(r->HL.W) & 0xff;

  cout << "(de)=" << setw(2) << setfill('0') << hex << c1
       << " '" << (isprint(c1) ? c1 : '.') << "' "
       << "(hl)=" << setw(2) << setfill('0') << hex << c2
       << " '" << (isprint(c2) ? c2 : '.') << "'"
       << endl;
}
#endif

void
Z80::executestep(void)
{
  _executestep = true;
}

void
Z80::singlestep(bool value)
{
  _singlestep = value;
}

bool
Z80::singlestep()
{
  return _singlestep;
}

bool
Z80::run(void)
{
  int a;
  CMD *cmd;

  signal(SIGINT, signalHandler);

  if (timer)
    timer->start();

  cmd = new CMD_single_step(this);

  a = 0;
  while (!_do_quit)
    {
//      if (DBG_check("KCemu/Z80core/trace"))
//	debug(true);

      //if (_regs.PC.W <= 0x8000)
      //z80->printPC(); cout << endl;

      //if (_regs.PC.W == 0x0170)
      //z80->debug(true);

      if (_singlestep)
	{
	  ui->update(true);
	  if (!_executestep)
	    {
	      sys_usleep(100000);
	      continue;
	    }
	  CMD_EXEC("single-step-executed");
	  _executestep = false;

	}
      else
	if (_trace)
	  {
	    ui->update();
	    CMD_EXEC("single-step-executed");
	    sys_usleep(_tracedelay);
	  }

      if (_debug)
        {
          int addr = getPC();
          char buf[80];
          int t, t2;
          int base_addr;
          
          printf("%04X: ", addr);
          addr += z80ex_dasm(buf, 80, 0, &t, &t2, z80ex_dasm_readbyte_cb, addr, &base_addr);
          printf("%-15s  t=%d", buf, t);
          if (t2) printf("/%d", t2);
          printf("\n");
	}

      int tstates = z80ex_step(_context);

      if (_irq_line && z80ex_int_possible(_context))
        {
          word_t irq = daisy->irq_ack();
          if (irq != IRQ_NOT_ACK)
            {
              _next_irq = (byte_t)irq;
              z80ex_int(_context);
            }
        }

      if (_enable_floppy_cpu && fdc_z80)
	fdc_z80->execute();

      _counter += tstates;

      _cb_list.run_callbacks(_counter);
    }

  return false;
}

void
Z80::quit(void)
{
  _do_quit = true;
}

void
Z80::addCallback(unsigned long long offset, Callback *cb, void *data)
{
  _cb_list.add_callback(getCounter() + offset, cb, data);
}

void
Z80::remove_callback_listener(Callback *cb)
{
  _cb_list.remove_callback_listener(cb);
}

bool
Z80::debug(void)
{
  return _debug;
}

void
Z80::debug(bool value)
{
  _debug = value;
  if (_enable_floppy_cpu && fdc_z80)
    fdc_z80->trace(value);
}

bool
Z80::trace(void)
{
  return _trace;
  //return _regs.Trace;
}

void
Z80::trace(bool value)
{
  _trace = value;
  //if (level < 0) level = 0;
  //_regs.Trace = level;
}

void
Z80::tracedelay(long delay)
{
  _tracedelay = delay;
}

void
Z80::reset(word_t pc, bool power_on)
{
  _cb_list.clear();

  for (ic_list_t::iterator it = _ic_list.begin();it != _ic_list.end();it++)
    (*it)->reset(power_on);

  module->reset(power_on);

  z80ex_reset(_context);
  z80ex_set_reg(_context, regPC, pc);

  /*
   *  FIXME: at least z1013 emulation breaks with the stackpointer
   *  FIXME: initialized with 0xf000; the CP/M bootlader BL4 will
   *  FIXME: overwrite it's own stack when clearing the screen :-(
   */
  z80ex_set_reg(_context, regSP, 0x0000);

  halt_floppy_cpu(power_on);

  if (timer)
    timer->start();
}

void
Z80::reset(void)
{
  const EmulationType &emulation_type = Preferences::instance()->get_system_type()->get_emulation_type();
  reset(emulation_type.get_reset_addr(), false);
}

void
Z80::power_on(void)
{
  const EmulationType &emulation_type = Preferences::instance()->get_system_type()->get_emulation_type();
  reset(emulation_type.get_power_on_addr(), true);
}

void
Z80::jump(word_t pc)
{
  z80ex_set_reg(_context, regPC, pc);
}

void
Z80::register_ic(InterfaceCircuit *h)
{
  _ic_list.push_back(h);
}

void
Z80::unregister_ic(InterfaceCircuit *h)
{
  _ic_list.remove(h);
}

dword_t
Z80::get_irq_mask(void)
{
  dword_t val = _irq_mask;

  if (val == 0)
    {
      DBG(0, form("KCemu/warning",
                  "get_irq_mask(): too many interrupt sources!\n"));
    }

  _irq_mask <<= 1;

  return val;
}

void
Z80::set_irq_line(dword_t mask)
{
  dword_t irq_line = _irq_line | mask;
  DBG(2, form("KCemu/Z80/irq",
              "set_irq_line():   %04x: %04x -> %04x\n",
              mask, _irq_line, irq_line));
  _irq_line = irq_line;
}

void
Z80::reset_irq_line(dword_t mask)
{
  _irq_line = _irq_line & (~mask);
  DBG(2, form("KCemu/Z80/irq",
              "reset_irq_line(): %04x -> %04x\n",
              mask, _irq_line));
}

void
Z80::reti(void)
{
  daisy->reti();
}

void
Z80::nmi(void)
{
  z80ex_nmi(_context);
}

void
Z80::printPC(void)
{
  cout << setw(4) << setfill('0') << hex << getPC() << "h: ";
}

void
Z80::start_floppy_cpu(void)
{
  if (fdc_z80)
    {
      fdc_z80->reset();
      _enable_floppy_cpu = true;
    }
}

void
Z80::halt_floppy_cpu(bool power_on)
{
  if (fdc_z80)
    {
      fdc_z80->reset(power_on);
      _enable_floppy_cpu = false;
    }
}

Z80EX_BYTE
Z80::z80ex_dasm_readbyte_cb(Z80EX_WORD addr, void *user_data)
{
  return memory->memRead8(addr);
}

Z80EX_BYTE
Z80::z80ex_mread_cb(Z80EX_CONTEXT *cpu, Z80EX_WORD addr, int m1_state, void *user_data)
{
  return memory->memRead8(addr);
}

void
Z80::z80ex_mwrite_cb(Z80EX_CONTEXT *cpu, Z80EX_WORD addr, Z80EX_BYTE value, void *user_data)
{
  memory->memWrite8(addr, value);
}

Z80EX_BYTE
Z80::z80ex_pread_cb(Z80EX_CONTEXT *cpu, Z80EX_WORD port, void *user_data)
{
  byte_t value;

  value = ports->in(port);
  DBG(2, form("KCemu/Z80/pread_cb",
              "Z80::z80ex_pread_cb():  %04x -> %02x\n",
              port, value));
  return value;
}

void
Z80::z80ex_pwrite_cb(Z80EX_CONTEXT *cpu, Z80EX_WORD port, Z80EX_BYTE value, void *user_data)
{
  DBG(2, form("KCemu/Z80/pwrite_cb",
              "Z80::z80ex_pwrite_cb(): %04x -> %02x\n",
              port, value));
  ports->out(port, value);
}

Z80EX_BYTE
Z80::z80ex_intread_cb(Z80EX_CONTEXT *cpu, void *user_data)
{
  Z80 *z80 = (Z80 *)user_data;
  DBG(2, form("KCemu/Z80/intread_cb",
              "Z80::z80ex_intread_cb(): %02x\n",
              z80->_next_irq));
  return z80->_next_irq;
}

void
Z80::z80ex_reti_cb(Z80EX_CONTEXT *cpu, void *user_data)
{
  Z80 *z80 = (Z80 *)user_data;
  DBG(2, form("KCemu/Z80/reti_cb",
              "Z80::z80ex_reti_cb(): RETI\n"));
  z80->reti();
}

static void
signalHandler(int sig) 
{
  static bool flag = false;
  cout << "\n *** signal caught (" << sig << ") ***\n\n";
  signal(sig, signalHandler);
  flag = !flag;
  self->debug(flag);
}
