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

#ifndef __kc_pio_h
#define __kc_pio_h

#include "kc/system.h"

#include "kc/ic.h"
#include "kc/ports.h"

class PIOCallbackInterface
{
 public:
  PIOCallbackInterface(void) {}
  virtual ~PIOCallbackInterface(void) {}

  virtual int callback_A_in(void) = 0;
  virtual int callback_B_in(void) = 0;
  virtual void callback_A_out(byte_t val) = 0;
  virtual void callback_B_out(byte_t val) = 0;
};

class PIO : public InterfaceCircuit, public PortInterface
{
  protected:
  enum {
    A = 0,
    B = 1,
  };

  enum {
    MODE_OUTPUT        = 0,
    MODE_INPUT         = 1,
    MODE_BIDIRECTIONAL = 2,
    MODE_CONTROL       = 3,
  };

  /*
   *  track first out instruction to allow complete
   *  initialisation
   */
  bool _first_out[2];

    /*
     *  external value
     */
    byte_t _ext[2];

    /*
     *  value of the logical function that triggers the irq in bit-mode
     *
     */
    byte_t _ext_fn[2];
    
    /*
     *  pio mode: 0 - byte output
     *            1 - byte input
     *            2 - byte input/output
     *            3 - bit  input/output
     */
    byte_t _mode[2];
    byte_t _bit_mode[2];
    bool   _bit_mode_follows[2];

    /*
     *  interrupt vector:
     */
    byte_t _irq_vector[2];

    /*
     *  interrupt control: 0 - irq disabled
     *                     1 - irq enabled
     */
    byte_t _irq[2];

    /*
     *  interrupt logic:
     */
    byte_t _irq_mask[2];
    byte_t _irq_h_l[2];
    byte_t _irq_and_or[2];
    bool   _irq_mask_follows[2];

    /*
     *  interrupt enable input/ output
     */
    byte_t _irq_enable[2];
    byte_t _irq_active[2];

    byte_t _strobe[2];
    byte_t _ready[2];

    byte_t _value[2];

    /*
     *  interrupt mask for daisy chain handling
     */
    dword_t _z80_irq_mask;

    /*
     *  callbacks
     */
    PIOCallbackInterface *_cb_a_in;
    PIOCallbackInterface *_cb_a_out;
    PIOCallbackInterface *_cb_b_in;
    PIOCallbackInterface *_cb_b_out;

    void out_CTRL(int port, byte_t val);
    void set_EXT(int port, byte_t mask, byte_t val);
    void trigger_irq(int port);
    
    virtual void change_A(byte_t changed, byte_t val) = 0;
    virtual void change_B(byte_t changed, byte_t val) = 0;

  public:
    PIO(void);
    virtual ~PIO(void);

    virtual void info(void);
    virtual byte_t in_A_DATA(void);
    virtual byte_t in_B_DATA(void);
    virtual byte_t in_A_CTRL(void);
    virtual byte_t in_B_CTRL(void);
    virtual void out_A_DATA(byte_t val);
    virtual void out_B_DATA(byte_t val);
    virtual void out_A_CTRL(byte_t val);
    virtual void out_B_CTRL(byte_t val);
    virtual void set_A_EXT(byte_t mask, byte_t val);
    virtual void set_B_EXT(byte_t mask, byte_t val);
    virtual void strobe_A(void);
    virtual void strobe_B(void);
    virtual int  ready_A(void);
    virtual int  ready_B(void);

    virtual byte_t getModeA(void) { return _mode[A]; }
    virtual byte_t getModeB(void) { return _mode[B]; }
    virtual byte_t getIRQA(void) { return _irq[A]; }
    virtual byte_t getIRQB(void) { return _irq[B]; }
    virtual byte_t getIRQVectorA(void) { return _irq_vector[A]; }
    virtual byte_t getIRQVectorB(void) { return _irq_vector[B]; }

    /*
     *  InterfaceCircuit
     */
    virtual void reti(void);
    virtual void irqreq(void);
    virtual word_t irqack(void);
    virtual void reset(bool power_on = false);

    /*
     *  callback registering functions
     */
    virtual void register_callback_A_in(PIOCallbackInterface *cbi);
    virtual void register_callback_A_out(PIOCallbackInterface *cbi);
    virtual void register_callback_B_in(PIOCallbackInterface *cbi);
    virtual void register_callback_B_out(PIOCallbackInterface *cbi);
};

#endif /* __kc_pio_h */

