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

#include <assert.h>
#include <iostream>
#include <iomanip>

#include "kc/system.h"

#include "kc/kc.h"
#include "ui/ui.h"
#include "kc/ctc.h"
#include "kc/tape.h"

#include "libdbg/dbg.h"

using namespace std;

CTC::CTC(const char *name) : InterfaceCircuit(name), Callback(name)
{
  _irq_valid[0] = 0;
  _irq_valid[1] = 1;
  _irq_valid[2] = 2;
  _irq_valid[3] = 3;
  _cb_list[0] = NULL;
  _cb_list[1] = NULL;
  _cb_list[2] = NULL;
  _cb_list[3] = NULL;

  reset(true);
}

CTC::~CTC(void)
{
}

void
CTC::reti(void)
{
  int a, b;

  b = 0;
  for (a = 0;a < 4;a++)
    {
      if (_irq_active[a])
	{
	  b++;
	  _irq_active[a] = 0;
	}
    }

  if (b > 1)
    DBG(2, form("KCemu/CTC/reti",
		"CTC::reti(): more than one irq active!!!"));

  if (b == 0)
    return;

  DBG(2, form("KCemu/CTC/reti",
	      "CTC::reti(): active: %d %d %d %d - pending: %d %d %d %d\n",
	      _irq_active[0],
	      _irq_active[1],
	      _irq_active[2],
	      _irq_active[3],
	      _irq_pending[0],
	      _irq_pending[1],
	      _irq_pending[2],
	      _irq_pending[3]));

  for (a = 0;a < 4;a++)
    {
      if (_irq_pending[a])
	{
	  DBG(2, form("KCemu/CTC/reti",
		      "CTC::reti(): trigger_irq(): channel = %d\n",
		      a));
	  try_trigger_irq(a);
	  break;
	}
    }
}

void
CTC::irqreq(void)
{
}

word_t
CTC::irqack(void)
{
  return IRQ_NOT_ACK;
}

void
CTC::reset(bool power_on)
{
  int a;
  
  _irq_vector = 0;
  for (a = 0;a < 4;a++)
    {
      _control[a]     = 0x23;
      _value[a]       = 0;
      _restart[a]     = 0;
      _timer_value[a] = 0;
      _irq_active[a]  = 0;
      _irq_pending[a] = 0;
      _irq_valid[a]   += 4;
    }
}

void
CTC::trigger(byte_t channel)
{
  byte_t c = channel & 3;

  if ((_control[c] & MODE) != MODE_COUNTER)
    return;

  _value[c]--;
  if (_value[c] > 0)
    return;

  _value[c] = _timer_value[c];
  try_trigger_irq(c);
}

void
CTC::handle_counter_mode(int channel)
{
  long cv;

  switch (channel)
    {
    case 0: cv = counter_value_0(); break;
    case 1: cv = counter_value_1(); break;
    case 2: cv = counter_value_2(); break;
    case 3: cv = counter_value_3(); break;
    default: assert(0);
    }
  
  if (cv == 0)
    return;
  
  cv *= _timer_value[channel]; // mapping from 0 to 256 is done in c_out()

  run_cb_tc(channel, _timer_value[channel]);

  _irq_valid[channel] += 4;
  add_callback(cv, this, (void *)((long)_irq_valid[channel]));
}

void
CTC::try_trigger_irq(int channel)
{
  if ((_control[channel] & IRQ) == IRQ_DISABLED)
    {
      _irq_pending[channel] = 0;
      return;
    }

  _irq_pending[channel] = 1;
  trigger_irq(channel);
}

void
CTC::callback(void *data)
{
  bool cont;
  long val = (long)data;
  byte_t c = val & 3;

  if (_irq_valid[c] != val)
    return;

  DBG(2, form("KCemu/CTC/reti",
	      "CTC::callback(): active: %d %d %d %d - pending: %d %d %d %d\n",
	      _irq_active[0],
	      _irq_active[1],
	      _irq_active[2],
	      _irq_active[3],
	      _irq_pending[0],
	      _irq_pending[1],
	      _irq_pending[2],
	      _irq_pending[3]));

  switch (c)
    {
    case 0:
      DBG(2, form("KCemu/CTC/irq/0",
		  "CTC::callback() : irq channel 0\n"));
      cont = irq_0();
      break;
    case 1:
      DBG(2, form("KCemu/CTC/irq/1",
		  "CTC::callback() : irq channel 1\n"));
      cont = irq_1();
      break;
    case 2:
      DBG(2, form("KCemu/CTC/irq/2",
		  "CTC::callback() : irq channel 2\n"));
      cont = irq_2();
      break;
    case 3:
      DBG(2, form("KCemu/CTC/irq/3",
		  "CTC::callback() : irq channel 3\n"));
      cont = irq_3();
      break;
    }

  /*
   *  COUNTER mode (clock source comes from the CLK pin)
   */
  if ((_control[c] & MODE) == MODE_COUNTER)
    {
      handle_counter_mode(c);
      return;
    }

  if ((_control[c] & IRQ) == IRQ_ENABLED)
    {
      DBG(2, form("KCemu/CTC/callback",
		  "CTC::callback(): trigger_irq(): _irq_pending = %d\n",
		  _irq_pending[c]));
      try_trigger_irq(c);
    }

  add_callback(_timer_value[c], this, (void *)((long)val));
}

byte_t
CTC::c_in(byte_t c)
{
  word_t val;
  unsigned long long diff = 0;
  
  if (_timer_value[c] == 0)
    {
      return 0;
    }

  if ((_control[c] & MODE) == MODE_COUNTER)
    {
      diff = 0;
    }
  else
    {
      if ((_control[c] & RESET) == RESET_ACTIVE)
        {
          diff = 0;
        }
      else
        {
          diff = get_counter() - _counter[c];
        }
    }
  
  if ((_control[c] & PRESCALER) == PRESCALER_16)
    {
      diff /= 16;
      val = ((_value[c] / 16) - diff) & 0xff;
    }
  else
    {
      diff /= 256;
      val = ((_value[c] / 256) - diff) & 0xff;
    }

  _counter[c] = get_counter();
  
  return val;
}

void
CTC::c_out(byte_t channel, byte_t val)
{
  if ((_control[channel] & CONSTANT) == CONSTANT_LOAD)
    {
      _control[channel] &= ~(CONSTANT | RESET);
      run_cb_start(channel);

      if ((_control[channel] & MODE) == MODE_TIMER)
	{
	  if ((_control[channel] & PRESCALER) == PRESCALER_16)
	    {
	      _timer_value[channel] = val << 4; /* div 16 */
	      if (_timer_value[channel] == 0)
		_timer_value[channel] = 4096;
	    }
	  else
	    {
	      _timer_value[channel] = val << 8; /* div 256 */
	      /*
	       *  well, this should be 65536 but this would need a dword
	       *  for _timer_value
	       */
	      if (_timer_value[channel] == 0)
		_timer_value[channel] = 65535;
	    }
	}
      else
	{
	  /*
	   *  COUNTER MODE has no prescaler
	   */
	  _timer_value[channel] = val;
	}

      _counter[channel] = get_counter();
      _value[channel] = _timer_value[channel];
      run_cb_tc(channel, _timer_value[channel]);

      if ((_control[channel] & MODE) == MODE_COUNTER)
	{
	  handle_counter_mode(channel);
	  return;
        }
      
      _irq_valid[channel] += 4;
      /*
       *  Added a fixed offset for callback timing added for the poly880
       *  emulation. Without offset the CTC caused NMI is triggered
       *  one opcode too early.
       *
       *  FIXME: check timing
       */
      add_callback(_timer_value[channel] + 4, this, (void *)((long)_irq_valid[channel]));
      return;
    }
	
  if ((val & CONTROL) == CONTROL_VECTOR)
    {
      if (channel != 0) return;
      _irq_vector = val & ~ 7;
      return;
    }
	
  if ((val & RESET) == RESET_ACTIVE)
    {
      _value[channel] = _timer_value[channel];
      _irq_valid[channel] += 4;
      if ((_control[channel] & RESET) != RESET_ACTIVE)
	run_cb_stop(channel);
    }
	
  _control[channel] = val;
}

void
CTC::register_callback(int channel, CTCCallbackInterface *cbi)
{
  int c = channel & 3;

  if (_cb_list[c] == NULL)
    {
      _cb_list[c] = new cb_list_t();
    }

  _cb_list[c]->push_back(cbi);
}

void CTC::run_cb_start(int channel)
{
  if (_cb_list[channel] == NULL)
    return;

  for (iterator it = _cb_list[channel]->begin();it != _cb_list[channel]->end();it++)
    (*it)->ctc_callback_start(channel);
}

void CTC::run_cb_stop(int channel)
{
  if (_cb_list[channel] == NULL)
    return;

  for (iterator it = _cb_list[channel]->begin();it != _cb_list[channel]->end();it++)
    (*it)->ctc_callback_stop(channel);
}

void CTC::run_cb_tc(int channel, long tc)
{
  if (_cb_list[channel] == NULL)
    return;

  for (iterator it = _cb_list[channel]->begin();it != _cb_list[channel]->end();it++)
    (*it)->ctc_callback_TC(channel, tc);
}

void
CTC::info(void)
{
    cout << "  CTC:" << endl;
    cout << "  ----                   channel 0     channel 1     "
	 << "channel 2     channel 3" << endl;
    cout << "\tirq vector:      "
	 << hex << setfill('0') << setw(2) << (int)(_irq_vector)
	 << "h           "
	 << hex << setfill('0') << setw(2) << (int)(_irq_vector | 0x02)
	 << "h           "
	 << hex << setfill('0') << setw(2) << (int)(_irq_vector | 0x04)
	 << "h           "
	 << hex << setfill('0') << setw(2) << (int)(_irq_vector | 0x06)
	 << "h" << endl;
    cout << "\tirq enabled:     "
	 << ((_control[0] & IRQ)?"yes":"no ")
	 << "           "
	 << ((_control[1] & IRQ)?"yes":"no ")
	 << "           "
	 << ((_control[2] & IRQ)?"yes":"no ")
	 << "           "
	 << ((_control[3] & IRQ)?"yes":"no ") << endl;
    cout << "\tcounter mode:    "
	 << ((_control[0] & MODE)?"counter":"timer  ")
	 << "       "
	 << ((_control[1] & MODE)?"counter":"timer  ")
	 << "       "
	 << ((_control[2] & MODE)?"counter":"timer  ")
	 << "       "
	 << ((_control[3] & MODE)?"counter":"timer") << endl;
    cout << "\tclock divider:   "
	 << ((_control[0] & PRESCALER)?"256":"16 ")
	 << "           "
	 << ((_control[1] & PRESCALER)?"256":"16 ")
	 << "           "
	 << ((_control[2] & PRESCALER)?"256":"16 ")
	 << "           "
	 << ((_control[3] & PRESCALER)?"256":"16 ") << endl;
    cout << "\twait for tv:     "
	 << ((_control[0] & CONSTANT)?"yes":"no ")
	 << "           "
	 << ((_control[1] & CONSTANT)?"yes":"no ")
	 << "           "
	 << ((_control[2] & CONSTANT)?"yes":"no ")
	 << "           "
	 << ((_control[3] & CONSTANT)?"yes":"no ") << endl;
    cout << "\treset:           "
	 << ((_control[0] & RESET)?"yes":"no ")
	 << "           "
	 << ((_control[1] & RESET)?"yes":"no ")
	 << "           "
	 << ((_control[2] & RESET)?"yes":"no ")
	 << "           "
	 << ((_control[3] & RESET)?"yes":"no ") << endl;
    cout << "\ttimer value:     "
	 << hex << setfill('0') << setw(2) << (int)_timer_value[0]
	 << "h           "
	 << hex << setfill('0') << setw(2) << (int)_timer_value[1]
	 << "h           "
	 << hex << setfill('0') << setw(2) << (int)_timer_value[2]
	 << "h           "
	 << hex << setfill('0') << setw(2) << (int)_timer_value[3]
	 << "h" << endl;
    cout << "\tvalue:           "
	 << hex << setfill('0') << setw(2) << (int)_value[0]
	 << "h           "
	 << hex << setfill('0') << setw(2) << (int)_value[1]
	 << "h           "
	 << hex << setfill('0') << setw(2) << (int)_value[2]
	 << "h           "
	 << hex << setfill('0') << setw(2) << (int)_value[3]
	 << "h" << endl << endl;
}
