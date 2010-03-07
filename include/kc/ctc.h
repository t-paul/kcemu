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

#ifndef __kc_ctc_h
#define __kc_ctc_h

#include <list>

#include "kc/system.h"

#include "kc/ic.h"
#include "kc/cb.h"
#include "kc/ports.h"

class CTCCallbackInterface
{
 public:
  CTCCallbackInterface(void) {}
  virtual ~CTCCallbackInterface(void) {}

  virtual void ctc_callback_ZC(int channel) = 0;
  virtual void ctc_callback_TC(int channel, long tc) = 0;
  virtual void ctc_callback_start(int channel) = 0;
  virtual void ctc_callback_stop(int channel) = 0;
};

class CTC : public InterfaceCircuit, public PortInterface, public Callback
{
 protected:
  typedef std::list<CTCCallbackInterface *>cb_list_t;
  typedef cb_list_t::iterator iterator;

  enum {
    IRQ            = 0x80,
    IRQ_DISABLED   = 0x00,
    IRQ_ENABLED    = 0x80,
                 
    MODE           = 0x40,
    MODE_TIMER     = 0x00,
    MODE_COUNTER   = 0x40,
                 
    PRESCALER      = 0x20,
    PRESCALER_16   = 0x00,
    PRESCALER_256  = 0x20,
                 
    EDGE           = 0x10,
    EDGE_FALLING   = 0x00,
    EDGE_RISING    = 0x10,
                 
    TRIGGER        = 0x08,
    TRIGGER_AUTO   = 0x00,
    TRIGGER_CLOCK  = 0x08,
                 
    CONSTANT       = 0x04,
    CONSTANT_NONE  = 0x00,
    CONSTANT_LOAD  = 0x04,
                 
    RESET          = 0x02,
    RESET_NONE     = 0x00,
    RESET_ACTIVE   = 0x02,
                 
    CONTROL        = 0x01,
    CONTROL_VECTOR = 0x00,
    CONTROL_WORD   = 0x01,
  };
        
  byte_t  _irq_vector;
  byte_t  _control[4];
  word_t  _value[4];
  word_t _timer_value[4];
  byte_t  _restart[4];
  long long _counter[4];

  byte_t _irq_pending[4];
  byte_t _irq_active[4];
  long   _irq_valid[4];

  cb_list_t *_cb_list[4];

 private:
  void handle_counter_mode(int channel);

 protected:
  virtual long long get_counter() = 0;
  virtual void trigger_irq(int channel) = 0;
  virtual void add_callback(unsigned long long offset, Callback *cb, void *data) = 0;

  virtual void try_trigger_irq(int channel);

 public:
  CTC(const char *name);
  virtual ~CTC(void);

  virtual bool irq_0(void) = 0;
  virtual bool irq_1(void) = 0;
  virtual bool irq_2(void) = 0;
  virtual bool irq_3(void) = 0;

  virtual long counter_value_0(void) = 0;
  virtual long counter_value_1(void) = 0;
  virtual long counter_value_2(void) = 0;
  virtual long counter_value_3(void) = 0;

  void info(void);
  
  byte_t c_in(byte_t channel);
  void c_out(byte_t channel, byte_t val);
  
  void trigger(byte_t channel);
  void callback(void *data);

  byte_t getIRQVector(byte_t channel)
    {
      return _irq_vector + 2 * channel;
    }
  byte_t getIRQ(byte_t channel)
    {
      return (_control[channel] & IRQ) == IRQ_ENABLED;
    }
  word_t getTimerValue(byte_t channel)
    {
      return _timer_value[channel];
    }
  bool isCounter(byte_t channel)
    {
      return ((_control[channel] & MODE) == MODE_COUNTER);
    }
  bool isReset(byte_t channel)
    {
      return ((_control[channel] & RESET) == RESET_ACTIVE);
    }

  /*
   *  InterfaceCircuit
   */
  virtual void reti(void);
  virtual void irqreq(void);
  virtual word_t irqack(void);
  virtual void reset(bool power_on = false);

  virtual void register_callback(int channel, CTCCallbackInterface *cbi);
  virtual void run_cb_start(int channel);
  virtual void run_cb_stop(int channel);
  virtual void run_cb_tc(int channel, long tc);
};

#endif /* __kc_ctc_h */
