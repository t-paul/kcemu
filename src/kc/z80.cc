/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: z80.cc,v 1.33 2002/01/06 12:53:40 torsten_paul Exp $
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
#include <iostream.h>
#include <iomanip.h>
#include <sys/time.h>

#include "kc/config.h"
#include "kc/system.h"

#include "kc/kc.h"
#include "kc/z80.h"
#include "kc/timer.h"
#include "kc/ports.h"
#include "kc/memory.h"
#include "kc/z80_fdc.h"
#include "kc/cb_list.h"

#include "z80core/z80.h"

#include "cmd/cmd.h"

#include "ui/ui.h"

#include "libdbg/dbg.h"

extern "C" {
#include "z80core2/z80.h"
}

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
	      long delay = 1000 * args->get_int_arg("delay");
	      z80->tracedelay(delay);
	    }
	  break;
	}
    }
};

byte_t
RdZ80(word_t Addr)
{
  byte_t Value = memory->memRead8(Addr);
  return Value;
}

void
WrZ80(word_t Addr, byte_t Value)
{
  memory->memWrite8(Addr, Value);
  ui->memWrite(Addr, Value);
}

void
OutZ80(word_t Port, byte_t Value)
{
  DBG(3, form("KCemu/Z80/OutZ80",
              "OutZ80(): %04x -> %02x\n",
              Port, Value));
  ports->out(Port, Value);
}

byte_t
InZ80(word_t Port)
{
  byte_t Value;

  Value = ports->in(Port);
  DBG(3, form("KCemu/Z80/InZ80",
              "InZ80():  %04x -> %02x\n",
              Port, Value));
  return Value;
}

void
PatchZ80(_Z80 * /* R */)
{
}


/*
 *  not used
 */
word_t
LoopZ80(_Z80 * /*R*/)
{
  return INT_NONE;
}

void
RetiZ80(void)
{
  z80->reti();
}

/*
 *
 */
long
get_time(void)
{
  struct timeval tv;
  static long base = -1;

  gettimeofday(&tv, NULL);
  if (base < 0) base = tv.tv_sec;
  return 1000 * (tv.tv_sec - base) + (tv.tv_usec / 1000);
}

const int Z80::I_PERIOD = 256;

Z80::Z80(void)
{
  self = this;

  _regs.IPeriod = I_PERIOD;
  _regs.TrapBadOps = 1;
  _regs.Trace = 0;
  _regs.Trap = 0xffff;
  ResetZ80(&_regs);
  _regs.PC.W = 0xf000;
  
  _counter = 0;

  _debug = false;
  _trace = false;
  _singlestep = false;
  _executestep = false;
  _enable_floppy_cpu = false;

  _tracedelay = 100000;

  _do_quit = false;
}

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
  static int XXX = 0;

  signal(SIGINT, signalHandler);

  Z80_IPeriod = 0;
  Z80_IRQ = Z80_IGNORE_INT;

  timer->start();

  cmd = new CMD_single_step(this);

  a = 0;
  while (!_do_quit)
    {
#if 0
      if (_regs.PC.W == 0xe03e)
	XXX = 20000;
      if (XXX > 0)
	{
	  XXX--;
	  cout << "%%% " << hex << _regs.PC.W << endl;
	}
#endif
      //if ((_regs.PC.W == 0xe0d5) && (RdZ80(0xe0d5) == 0xed) && (_regs.BC.W < 5))
      //debug(true);

      if (_singlestep)
	{
	  ui->processEvents();
	  if (!_executestep)
	    {
	      usleep(100000);
	      continue;
	    }
	  CMD_EXEC("single-step-executed");
	  _executestep = false;
	}
      else
	if (_trace)
	  {
	    ui->processEvents();
	    ui->update();
	    CMD_EXEC("single-step-executed");
	    usleep(_tracedelay);
	  }
      
      if (_debug)
	{
	  _regs.Trace = 2;
	  if (!DebugZ80(&_regs))
	    break;
	  if (_regs.Trace == 0)
	    debug(false);
	}

      _regs.ICount = 0;
      ExecZ80(&_regs);

      if (_enable_floppy_cpu)
	fdc_z80->execute();

      if (_regs.IRequest != INT_NONE)
	{
	  if(_regs.IFF & 0x20)
	    {
	      /*
	       *  after EI state (set by the EI instruction)
	       *  this triggers irqs that are delayed due to a clear
	       *  interrupt flag
	       */
	      IntZ80(&_regs, _regs.IRequest);
	      _regs.IRequest = INT_NONE;
	      _regs.IFF &= 0xdf;
	    }
	  /*
	   *  trigger pending irq if IFF is set (= irqs enabled)
	   */
	  if ((_regs.IFF & 1) != 0)
	    {
	      IntZ80(&_regs, _regs.IRequest);
	      _regs.IRequest = INT_NONE;
	    }
	}

      if (_regs.IFF & 0x80)
        {
          /*
           *  the processor is executing HALT with ICount = 0!
           *  but we need to decrement period to keep the
           *  emulation running...
           */
          _regs.ICount = -4;
        }
      _counter -= _regs.ICount; // ICount is negative!

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

bool
Z80::debug(void)
{
  return _debug;
}

void
Z80::debug(bool value)
{
  _debug = value;
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

  ResetZ80(&_regs);
  _regs.PC.W = pc;

  timer->start();
}

void
Z80::power_on(word_t pc)
{
  reset(pc, true);
}

void
Z80::jump(word_t pc)
{
  _regs.PC.W = pc;
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

void
Z80::reti(void)
{
  ic_list_t::iterator it;

  for (it = _ic_list.begin();it != _ic_list.end();it++)
    (*it)->reti();
}

int
Z80::triggerIrq(int vector)
{
  /*
   *  we can't call IntZ80() here!
   *
   *  this function may be called while an instruction is
   *  not completely decoded; that can cause corrupted
   *  instructions or invalid irq vectors
   */
  if (_regs.IRequest != INT_NONE)
    return 0; // Irq already pending but not yet triggered!

  /*
   * if ((_regs.IFF & 1) == 0)
   *   return 0; // Interrupts are disabled!
   */
  
  // Ok, no pending irq
  return 1;
}

void
Z80::handleIrq(int vector)
{
  _regs.IRequest = vector;
}

void
Z80::printPC(void)
{
  cout << setw(4) << setfill('0') << hex << _regs.PC.W << "h: ";
}

void
Z80::start_floppy_cpu(void)
{
  fdc_z80->start();
  _enable_floppy_cpu = true;
}

void
Z80::halt_floppy_cpu(void)
{
  _enable_floppy_cpu = false;
}

static void
signalHandler(int sig) 
{
  cout << "\n *** signal caught (" << sig << ") ***\n\n";
  signal(sig, signalHandler);
  self->debug(true);
}
