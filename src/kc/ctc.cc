/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: ctc.cc,v 1.14 2001/05/06 21:21:33 tp Exp $
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

#include <iostream.h>
#include <iomanip.h>

#include "kc/config.h"
#include "kc/system.h"

#include "kc/kc.h"
#include "ui/ui.h"
#include "kc/ctc.h"
#include "kc/z80.h"
#include "kc/tape.h"

#include "libdbg/dbg.h"

#define PARANOIA_CHECK
// #define CTC_IRQ_DEBUG 0xff
// #define CTC_VALUE_DEBUG 0xff
// #define CTC_IN_DEBUG 3
// #define CTC_OUT_DEBUG 2
// #define CTC_CB_DEBUG 0xff

CTC::CTC(void) : Callback("CTC")
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
  z80->register_ic(this);
}

CTC::~CTC(void)
{
  z80->unregister_ic(this);
}

void
CTC::iei(byte_t val)
{
  ieo(val);
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
CTC::reti(void)
{
  int a;

  DBG(2, form("KCemu/CTC/reti",
	      "CTC::reti()\n"));
  
  for (a = 0;a < 4;a++)
    {
      if (_irq_active[a])
        {
          _irq_active[a] = 0;
        }

      if (_irq_pending[a])
	{
	  DBG(2, form("KCemu/CTC/reti",
		      "CTC::reti(): triggerIrq(): _irq_pending[a] = %d\n",
		      _irq_pending[a]));
	  if (z80->triggerIrq(_irq_vector | a << 1))
	    {
	      //cerr.form("CTC::reti() [%d] - irq ack\n", a);
	      _irq_active[a] = 1;
	      z80->handleIrq(_irq_vector | a << 1);
	    }
	  else
	    {
	      //cerr.form("CTC::reti() [%d] - irq nack\n", a);
	    }
	  _irq_pending[a] = 0;
	}
    }
}

void
CTC::trigger(byte_t channel)
{
  byte_t c = channel & 3;

  if ((_control[c] & MODE) != MODE_COUNTER) return;
  if ((_control[c] & IRQ) != IRQ_ENABLED) return;

  _value[c]--;
  if (_value[c] > 0) return;

  _value[c] = _timer_value[c];

  if (_irq_pending[c] == 1)
    return;

  DBG(2, form("KCemu/CTC/reti",
	      "CTC::trigger(%d): triggerIrq(): _irq_pending = %d\n",
	      channel, _irq_pending[c]));
  _irq_pending[c] = 1;
  if (z80->triggerIrq(_irq_vector | c << 1))
    {
      _irq_active[c] = 1;
      z80->handleIrq(_irq_vector | c << 1);
    }
}

void
CTC::callback(void *data)
{
  long tmp;
  bool cont;
  long val = (long)data;
  byte_t c = val & 3;

#ifdef PARANOIA_CHECK
  if (_timer_value[c] == 0)
    {
      cerr.form("CTC: [%d] - timer value is 0!\n", c);
      exit(1);
    }
#endif

  if (_irq_valid[c] != val)
    {
      // cerr.form("CTC: [%d] - invalid irq ignored\n", c);
      return;
    }

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
      if (c == 2)
        {
          /*
           *  CLK for channel 2 is 50 Hz
           */
          tmp = _timer_value[c] * CHANNEL_2_CLK;
          if (tmp == 0) tmp = 256 * CHANNEL_2_CLK;
          z80->addCallback(tmp, this, (void *)((long)_irq_valid[c]));
        }
      return;
    }

  /*
   *  TIMER mode (clock source is the internal clock prescaler)
   */
  switch (c)
    {
    case 1:
      if ((_control[c] & IRQ) != IRQ_ENABLED) return;
      break;
    case 2:
      if ((_control[c] & IRQ) != IRQ_ENABLED)
        {
          z80->addCallback(_timer_value[c], this, (void *)((long)val));
          return;
        }
      break;
    }
  
#ifdef CTC_IRQ_DEBUG
  if (c == CTC_IRQ_DEBUG || CTC_IRQ_DEBUG == 0xff)
    {
      /*
      cerr.form("CTC::callback(): [%d] %d [1fdh=%02x]\n", c, val,
                memory->memRead8(0x1fd));
      */
    }
#endif
  if ((_control[c] & IRQ) == IRQ_ENABLED)
    {
      if (_irq_pending[c] == 0)
	{
	  DBG(2, form("KCemu/CTC/reti",
		      "CTC::callback(): triggerIrq(): _irq_pending = %d\n",
		      _irq_pending[c]));
	  _irq_pending[c] = 1;
	  if (z80->triggerIrq(_irq_vector | c << 1))
	    {
	      _irq_active[c] = 1;
	      //cerr.form("CTC: [%d] - irq ack\n", c);
          z80->handleIrq(_irq_vector | c << 1);
	    }
	  else
	    {
	      //cerr.form("CTC: [%d] - irq nack\n", c);
	    }
	}
      z80->addCallback(_timer_value[c],
		       this, (void *)((long)val));
    }
}

byte_t
CTC::c_in(byte_t c)
{
  word_t val;
  unsigned long long diff = 0;
  
  if (_timer_value[c] == 0)
    {
      //Z80::printPC();
      //cerr.form("CTC: [%d] timer value is 0\n", c);
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
          diff = z80->getCounter() - _counter[c];
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

  /*
    cerr.form("%02x ", val);
    if (val < 0x10) {
    cerr.form("<#%02x#> ", memory->memRead8(0x1fd));
    }
    */
#ifdef CTC_IN_DEBUG
  if ((c == CTC_IN_DEBUG) || (CTC_IN_DEBUG == 0xff))
    {
      cerr.form("CTC: %04x in [%d] - cur = %Ld, old = %Ld, diff = %8Ld, tv = %04x - 0x%02x\n",
		z80->getPC(),
		c,
		z80->getCounter(),
		_counter[c],
		z80->getCounter() - _counter[c],
		_timer_value[c],
		val);
    }
#endif
  _counter[c] = z80->getCounter();
  
  return val;
}

void
CTC::c_out(byte_t channel, byte_t val)
{
  long tmp;

  if ((_control[channel] & CONSTANT) == CONSTANT_LOAD)
    {
      _control[channel] &= ~(CONSTANT | RESET);
      run_cb_start(channel);
      
      if ((_control[channel] & MODE) == MODE_COUNTER)
        {
#ifdef CTC_OUT_DEBUG
          if ((channel == CTC_OUT_DEBUG) || (CTC_OUT_DEBUG == 0xff))
            {
              z80->printPC();
              cerr << "CTC: new counter value for channel "
                   << (int)channel << ": 0x"
                   << hex << (int)val << " -> "
                   << (int)_timer_value[channel] << "\n";
            }
#endif
          _timer_value[channel] = val;
          if (_timer_value[channel] == 0) _timer_value[channel] = 256;
          _value[channel] = _timer_value[channel];
	  run_cb_tc(channel, _timer_value[channel]);
          if (channel == 2)
            {
              /*
               *  CLK for channel 2 is 50 Hz
               */
              tmp = _timer_value[channel] * CHANNEL_2_CLK;
              if (tmp == 0) tmp = 256 * CHANNEL_2_CLK;
              _irq_valid[channel] += 4;
              z80->addCallback(tmp, this, (void *)((long)_irq_valid[channel]));
            }
          return;
        }
      
      if ((_control[channel] & PRESCALER) == PRESCALER_16)
        {
          _timer_value[channel] = val << 4; /* div 16 */
          if (_timer_value[channel] == 0) _timer_value[channel] = 4096;
        }
      else
        {
          _timer_value[channel] = val << 8; /* div 256 */
          /*
           *  well, this should be 65536 but this would need a dword
           *  for _timer_value
           */
          if (_timer_value[channel] == 0) _timer_value[channel] = 65535;
        }
      _counter[channel] = z80->getCounter();
      _value[channel] = _timer_value[channel];
      run_cb_tc(channel, _timer_value[channel]);
#ifdef CTC_OUT_DEBUG
      if ((channel == CTC_OUT_DEBUG) || (CTC_OUT_DEBUG == 0xff))
        {
          z80->printPC();
          cerr << "CTC: new timer value for channel " << (int)channel << ": 0x"
               << hex << (int)val << " -> "
               << (int)_timer_value[channel] << "\n";
        }
#endif
      if ((_control[channel] & IRQ) == IRQ_ENABLED)
        {
#ifdef CTC_CB_DEBUG
          if ((channel == CTC_CB_DEBUG) || (CTC_CB_DEBUG == 0xff))
            {
              cerr.form("CTC: addCallback [%d] - %d\n",
                        channel,
                        _timer_value[channel]);
            }
#endif
          _irq_valid[channel] += 4;
          z80->addCallback(_timer_value[channel], this,
                           (void *)((long)_irq_valid[channel]));
        }
      else
        {
          /*
           *  special handling for tape io (channel 1)
           */
          if (channel == 1)
            {
              _irq_valid[channel] += 4;
              z80->addCallback(_timer_value[channel], this,
                               (void *)((long)_irq_valid[channel]));
            }
          /*
           *  special handling for blink (channel 2)
           */
          if (channel == 2)
            {
              _irq_valid[channel] += 4;
              z80->addCallback(_timer_value[channel], this,
                               (void *)((long)_irq_valid[channel]));
            }
        }

      return;
    }
	
  if ((val & CONTROL) == CONTROL_VECTOR)
    {
      if (channel != 0) return;
#ifdef CTC_OUT_DEBUG
      z80->printPC();
      cerr << "CTC: new irq vector: 0x" << hex << (int)val << "\n";
#endif
      _irq_vector = val & ~ 7;
      return;
    }
	
#ifdef CTC_OUT_DEBUG
  if ((channel == CTC_OUT_DEBUG) || (CTC_OUT_DEBUG == 0xff))
    {
      z80->printPC();
      cerr << "CTC: new control byte for channel " << (int)channel << ": 0x"
           << hex << (int)val << "\n";
      if ((val & MODE) == MODE_TIMER)
        {
          z80->printPC();
          cerr.form("CTC: [%d] mode = TIMER\n", channel);
        }
      else
        {
          z80->printPC();
          cerr.form("CTC: [%d] mode = COUNTER\n", channel);
        }
    }
#endif

  if ((val & RESET) == RESET_ACTIVE)
    {
      _value[channel] = _timer_value[channel];
      _irq_valid[channel] += 4;
      if ((_control[channel] & RESET) != RESET_ACTIVE)
	run_cb_stop(channel);
#ifdef CTC_OUT_DEBUG
      if ((channel == CTC_OUT_DEBUG) || (CTC_OUT_DEBUG == 0xff)) {
        z80->printPC();
        cerr << "CTC: reset channel " << (int)channel << endl;
      }
#endif
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

