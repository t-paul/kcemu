/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-1998 Torsten Paul
 *
 *  $Id: z80.cc,v 1.24 2001/01/07 02:40:03 tp Exp $
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

#ifdef MSDOS
#include <allegro.h> /* FIXME: only for debugging! */
#endif /* MSDOS */

#include "kc/config.h"
#include "kc/system.h"

#include "kc/kc.h"
#include "kc/ctc.h"
#include "kc/pio.h"
#include "kc/z80.h"
#include "kc/tape.h"
#include "kc/ports.h"
#include "kc/timer.h"
#include "kc/memory.h"
#include "kc/keyboard.h"
#include "kc/sh_mem.h"
#include "kc/disk_io.h"

#include "z80core/z80.h"

#include "ui/ui.h"
#include "ui/status.h"

#include "libdbg/dbg.h"

extern "C" {
#include "z80core2/z80.h"
}

#define Z80_CB_DELAY_DEBUG
/* #define Z80_CB_LOG */
/* #define Z80_CB_ADD_LOG */
/* #define PARANOIA_CHECK */

#define TICKS_PER_SECOND (50)
#define Z80_PERIOD (1750000 / TICKS_PER_SECOND)

static Z80 *self;
static struct timeval __tv, __otv;

#define EXEC_TRACE_LEN (100)
word_t exec_trace[EXEC_TRACE_LEN];
int exec_trace_idx = 0;

byte_t fdc_mem[65536];
int Z80_IRQ;

static void signalHandler(int sig);

#ifdef PROFILE_WINDOW
static long __access_r[65536];
static long __access_w[65536];
#endif /* PROFILE_WINDOW */

static volatile int ___x = 0;

static long long ___t1;
static long long ___t2;
static long long ___diff = 0;
static long long ___diffm = 0;
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

class callback_list
{
private:
  static callback_list *_free_list;

public:  
  unsigned long long _counter;
  Callback *_cb;
  void *_data;
  long _id;
  callback_list *_next;

  callback_list(unsigned long long counter, Callback *cb, void *data,
                callback_list *next)
  {
    _cb      = cb;
    _data    = data;
    _next    = next;
    _counter = counter;
  }
  callback_list(unsigned long long counter, Callback *cb, void *data,
                callback_list *next, long id)
  {
    _cb      = cb;
    _data    = data;
    _next    = next;
    _counter = counter;
    _id = id;
  }
  static void init(void)
  {
    int a;

    _free_list = 0;
    for (a = 0;a < 1000;a++)
      _free_list = ::new callback_list(a, 0, 0, _free_list, a);
  }
  static void test(void)
  {
    callback_list *ptr;

    for (ptr = _free_list;ptr != 0;ptr = ptr->_next)
      cout.form("[%3d] ", ptr->_id);
    cout << endl;
  }
  void * operator new(size_t size)
  {
    callback_list *ptr = _free_list;

    _free_list = ptr->_next;
    //cout.form("op new: (id = %d) [%p]\n", ptr->_id, ptr);
    return ptr;
  }
  void operator delete(void *p)
  {
    callback_list *ptr = (callback_list *)p;
    ptr->_next = _free_list;
    _free_list = ptr;
    //cout.form("op del: (id = %d) [%p]\n", ptr->_id, p);
    //test();
  }
};
callback_list *callback_list::_free_list = 0;

#ifdef PARANOIA_CHECK
void _print_callback_list(const char *why, unsigned long long clock,
			  callback_list *list, const char *txt)
{
  int a;
  callback_list *ptr;

  a = 0;
  for (ptr = list;ptr != NULL;ptr = ptr->_next) a++;
  cerr.form("paranoia_check (%s):\n", why);
  cerr.form("\tfunction = %s\n", txt);
  cerr.form("\tpc       = %04x\n", z80->getPC());
  cerr.form("\tlength   = %10d\n", a);
  cerr.form("\tdiff     = %10Ld\n", list->_counter - clock);
  cerr.form("\tcounter  = %10Ld\n", clock);
  for (ptr = list;ptr != NULL;ptr = ptr->_next)
    {
      cerr.form("\tcounter  = %10Ld, cb = %8p [%s], data = %8p\n",
		ptr->_counter, ptr->_cb, ptr->_cb->get_name(), ptr->_data);
    }
}

void _callback_paranoia_check(unsigned long long clock, callback_list *list,
			      char *txt)
{
  unsigned long long last = 0;
  callback_list *tmp;
  callback_list *ptr = list;

  while (ptr != NULL)
    {
      if (ptr->_counter < clock)
	{
	  _print_callback_list("ptr->counter < clock", clock, list, txt);
	  return;
	}
      if (ptr->_counter < last)
        {
          _print_callback_list("ptr->counter < last", clock, list, txt);
	  return;
	}
      
      tmp = list;
      while (tmp != ptr)
        {
          if (tmp->_counter > ptr->_counter)
            {
              _print_callback_list("tmp->counter > ptr->counter",
				   clock, list, txt);
              return;
            }
          tmp = tmp->_next;
        }
      
      last = ptr->_counter;
      ptr = ptr->_next;
    }
}
#endif

byte_t
RdZ80(word_t Addr)
{
#ifdef PROFILE_WINDOW
  __access_r[Addr]++;
  ui->profile_mem_access(Addr, PF_MEM_READ);
#endif
  
  byte_t Value = memory->memRead8(Addr);

#if 0
  if ((Addr >= 0x0407) && (Addr <= 0x0407))
    {
      z80->printPC();
      cerr.form("memRead: %04x - %02x\n", Addr, Value);
    }
#endif
  
  return Value;
}

int
__W(word_t Addr, word_t loc)
{
  if ((Addr == loc) || (Addr == loc + 1))
    {
      cerr.form("memWrite: word at %04xh - %04xh\n",
                loc,
                memory->memRead8(loc) + (memory->memRead8(loc + 1) << 8));
      return 1;
    }
  return 0;
}

void
__WrZ80(word_t Addr, byte_t Value)
{
  if (Addr >= 0x01fd && Addr <= 0x01fd)
    {
      z80->printPC();
      cerr.form("memWrite: %04x - %02x -> %02x\n",
                Addr,
                memory->memRead8(Addr),
                Value);
    }
}

void
WrZ80(word_t Addr, byte_t Value)
{
#ifdef PROFILE_WINDOW
  __access_w[Addr]++;
  ui->profile_mem_access(Addr, PF_MEM_WRITE);
#endif

#if 0
  __WrZ80(Addr, Value);
#endif

  memory->memWrite8(Addr, Value);
  ui->memWrite(Addr, Value);
}

unsigned int
Z80_RDMEM(dword A)
{
  return fdc_mem[(A & 0xffff)];
}

void
Z80_WRMEM(dword A, byte V)
{
  fdc_mem[(A & 0xffff)] = V;
}

void
OutZ80(word_t Port, byte_t Value)
{
  DBG(3, form("KCemu/Z80/OutZ80",
              "OutZ80(): %04x -> %02x\n",
              Port, Value));
  ports->out(Port, Value);
}

void
Z80_Out(byte Port, byte Value)
{
  DBG(3, form("KCemu/Z80FDC/OutZ80",
              "OutZ80(): %04x: %04x -> %02x\n",
              Z80_GetPC(), Port, Value));
  fdc_ports->out(Port, Value);
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

/*
 *  not used
 */
void
PatchZ80(_Z80 * /* R */)
{
}

void
Z80_Patch(Z80_Regs * /* Regs */)
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

int
Z80_Interrupt(void)
{
  return Z80_IGNORE_INT;
}

void
RetiZ80(void)
{
  z80->reti();
}

void
Z80_Reti(void)
{
  printf("+++ Z80Core2: Z80_Reti\n");
}

void
Z80_Retn(void)
{
  printf("+++ Z80Core2: Z80_Retn\n");
}

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

#ifdef MSDOS
void
dos_timer(void)
{
  ___x++;
}
END_OF_FUNCTION(dos_timer);
#endif /* MSDOS */

const int Z80::I_PERIOD = 256;

Z80::Z80(void)
{
  self = this;

  callback_list::init();

#ifdef PROFILE_WINDOW
  for (int a = 0;a < 65536;a++)
    {
      __access_r[a] = 0;
      __access_w[a] = 0;
    }
#endif /* PROFILE_WINDOW */

  _enable_floppy_cpu = false;
	
  _regs.IPeriod = I_PERIOD;
  _regs.TrapBadOps = 1;
  _regs.Trace = 0;
  _regs.Trap = 0xffff;
  ResetZ80(&_regs);
  _regs.PC.W = 0xf000;
  
  _counter = 0;
  _callback_list = 0;

  _do_quit = false;
}

static void
print_regs(_Z80 *r)
{
  int c1, c2;

  z80->printPC();
  cout.form("a=%02x, bc=%04x, de=%04x, hl=%04x ",
	    r->AF.B.l,
	    r->BC.W,
	    r->DE.W,
	    r->HL.W);
  c1 = RdZ80(r->DE.W) & 0xff;
  c2 = RdZ80(r->HL.W) & 0xff;
  cout.form("(de)=%02x '%c' (hl)=%02x '%c'\n",
	    c1, isprint(c1) ? c1 : '.',
	    c2, isprint(c2) ? c2 : '.');
}

bool
Z80::run(void)
{
  int a;
  int period;
  long tdiff;
  long tdiff_old;
  long tick;  
  long tick_old;
  callback_list *ptr;

#ifdef MSDOS  
  install_timer();
  LOCK_VARIABLE(___x);
  LOCK_FUNCTION(dos_timer);
  install_int(dos_timer, 20);
#endif /* MSDOS */

  signal(SIGINT, signalHandler);
  gettimeofday(&__otv, 0);

  Z80_IPeriod = 0;
  /* Z80_IPeriod = 1000; */
  Z80_IRQ = Z80_IGNORE_INT;
  // atexit(dump_core);

  _regs.PC.W = 0xf000;

  timer->start();

  a = 0;
  period = 4 * Z80_PERIOD;
  tdiff_old = 0;
  tick_old = get_time();
  while (!_do_quit)
    {
      //if (_regs.PC.W == 0xbf70)
      //_regs.Trace = 2;
      
      //if (_regs.PC.W == 0xe348)
      //cout.form("e348: a=%02xh\n", _regs.AF.B.l);
      
      exec_trace[exec_trace_idx] = _regs.PC.W;
      exec_trace_idx = (exec_trace_idx + 1) % EXEC_TRACE_LEN;

      if (_regs.Trace)
	if (!DebugZ80(&_regs))
	  break;

      _regs.ICount = 0;
      ExecZ80(&_regs);

      if (_enable_floppy_cpu)
        {
	  Z80_Regs r;
          int _trap_addr = 0xffff;
	  Z80_GetRegs(&r); if (r.PC.D == _trap_addr) Z80_Trace = 1;
          Z80_Execute();
	  Z80_GetRegs(&r); if (r.PC.D == _trap_addr) Z80_Trace = 1;
          Z80_Execute();
	  Z80_GetRegs(&r); if (r.PC.D == _trap_addr) Z80_Trace = 1;
          Z80_Execute();
        }

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

#ifdef PROFILE_WINDOW
      ui->profile_mem_access(_regs.PC.W, PF_MEM_EXEC);
#endif

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
      period += _regs.ICount;

      while (_callback_list && _callback_list->_counter <= _counter)
        {
#ifdef Z80_CB_DELAY_DEBUG
          if ((_counter - _callback_list->_counter) > 30)
            {
              cerr.form("Z80::Callback(): delayed callback: %Ld\n",
                        _counter - _callback_list->_counter);
            }
#endif
          ptr = _callback_list;
#ifdef Z80_CB_LOG
	  if (strcmp("Keyboard", ptr->_cb->get_name()) == 0)
	    {
	      cerr.form("Z80::Callback(): [%s] cur = %Ld, wanted = %Ld, diff = %3Ld, data = %p\n",
			ptr->_cb->get_name(),
			_counter,
			_callback_list->_counter,
			_callback_list->_counter - _counter,
			ptr->_data);
	    }
#endif
          _callback_list = ptr->_next;
          ptr->_cb->callback(ptr->_data);
          delete ptr;
        }
#ifdef PARANOIA_CHECK
      _callback_paranoia_check(_counter, _callback_list, "Z80::run()");
#endif

      if (period < 0)
        {
          //___t1 = rdtsc();
          //ui->processEvents();
          //___t2 = rdtsc();
#ifdef LINUX

          tick = get_time();
          /*
          cout.form("counter = %Ld, tick = %ld, diff = %ld\n",
                    _counter, tick, tick - tick_old);
                    */
          tick_old += 20;
	  //while (tick < tick_old)
	  //tick = get_time();

          period += Z80_PERIOD;
#endif /* LINUX */

#ifdef MSDOS
          period += 4 * Z80_PERIOD;
          {
            char buf[100];
            while (___x < 4);
            sprintf(buf, ">%ld< ", ___x);
            textout(screen, font, buf, 0, 16, 15);
            if ((___t2 - ___t1) > ___diff) ___diff = ___t2 - ___t1;
            if (___diffm < ___diff)
              ___diffm = ___diff;
            sprintf(buf, "%Ld    ", ___diffm);
            textout(screen, font, buf, 0, SCREEN_H - 8, 15);
            ___x = 0;
            ___diff = 0;
          }
#endif /* MSDOS */
        }
    }

#ifdef MSDOS
  remove_int(dos_timer);
  remove_timer();
#endif /* MSDOS */

  return false;
}

void
Z80::quit(void)
{
	_do_quit = true;

        /*
        ofstream os;
        
        os.open("access.z80");

        cerr.form("Z80: writing access log...\n");
        if (!os) {
                cerr << "can't write 'access.z80'\n";
        } else {
                for (int a = 0;a < 65536;a++) {
                        os.form("%04xh: %10ld %10ld\n",
                                a, __access_r[a], __access_w[a]);
                }
                os.close();
        }
        */
}

void
Z80::addCallback(unsigned long long offset, Callback *cb, void *data)
{
  callback_list *ptr, *next;
  unsigned long long c = _counter + offset;

#ifdef Z80_CB_ADD_LOG
  cerr.form("Z80::addCallback(): [%s] _counter = %Ld, offset = %Ld,"
	    " data = %ld\n",
            cb->get_name(), _counter, offset, data);
#endif

  if (_callback_list == 0)
    {
      /*
       *  easy, empty list
       */
      _callback_list = new callback_list(c, cb, data, 0);
    }
  else
    {
      ptr = _callback_list;
      if (_callback_list->_counter > c)
        {
          /*
           *  add front
           */
          _callback_list = new callback_list(c, cb, data, ptr);
        }
      else
        {
          /*
           *  insert sorted
           */
          while (1)
            {
              if (ptr->_next && ptr->_next->_counter > c)
                {
                  next = ptr->_next;
                  ptr->_next = new callback_list(c, cb, data, next);
                  break;
                }
              else
                {
              if (ptr->_next == 0)
                {
                  ptr->_next = new callback_list(c, cb, data, 0);
                  break;
                }
                }
              ptr = ptr->_next;
            }
        }
    }
}

int
Z80::trace(void)
{
  return _regs.Trace;
}

void
Z80::trace(int level)
{
  if (level < 0) level = 0;
  _regs.Trace = level;
}

void
Z80::reset(word_t pc, bool power_on)
{
  callback_list *ptr;
  ic_list_t::iterator it;

  while (_callback_list)
    {
      ptr = _callback_list;
      _callback_list = ptr->_next;
      delete ptr;
    }
  ResetZ80(&_regs);
  _regs.PC.W = pc;
  for (it = _ic_list.begin();it != _ic_list.end();it++)
    (*it)->reset(power_on);

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
  cout.form("%04xh: ", _regs.PC.W);
}

void
Z80::start_floppy_cpu(void)
{
  Z80_Regs r;

  memset(fdc_mem, 0, 0xfc00);
  Z80_Reset();
  Z80_GetRegs(&r);
  r.PC.D = 0xfc00;
  Z80_SetRegs(&r);
  Z80_Trace = 0;
  if (DBG_check("KCemu/Z80core2/trace"))
    Z80_Trace = 1;
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
  self->trace(2);
}
