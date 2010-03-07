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

#ifndef __kc_z80_fdc_h
#define __kc_z80_fdc_h

#include <z80ex/z80ex.h>

#include "kc/system.h"

#include "kc/ic.h"
#include "kc/cb.h"
#include "kc/cb_list.h"

class Z80_FDC
{
 private:
  typedef std::list<InterfaceCircuit *> ic_list_t;

  byte_t _next_irq;
  Z80EX_CONTEXT *_context;
  
  ic_list_t _ic_list;

 public:
  bool _debug;
  
  unsigned long long _counter;

  CallbackList _cb_list;

 private:
  void do_execute(void);

  /*callback that returns byte for a given adress*/
  static Z80EX_BYTE z80ex_dasm_readbyte_cb(Z80EX_WORD addr, void *user_data);
  
  /*read byte from memory <addr> -- called when RD & MREQ goes active.
  m1_state will be 1 if M1 signal is active*/
  static Z80EX_BYTE z80ex_mread_cb (Z80EX_CONTEXT *cpu, Z80EX_WORD addr, int m1_state, void *user_data);

  /*write <value> to memory <addr> -- called when WR & MREQ goes active*/
  static void z80ex_mwrite_cb (Z80EX_CONTEXT *cpu, Z80EX_WORD addr, Z80EX_BYTE value, void *user_data);

  /*read byte from <port> -- called when RD & IORQ goes active*/
  static Z80EX_BYTE z80ex_pread_cb (Z80EX_CONTEXT *cpu, Z80EX_WORD port, void *user_data);

  /*write <value> to <port> -- called when WR & IORQ goes active*/
  static void z80ex_pwrite_cb (Z80EX_CONTEXT *cpu, Z80EX_WORD port, Z80EX_BYTE value, void *user_data);

  /*read byte of interrupt vector -- called when M1 and IORQ goes active*/
  static Z80EX_BYTE z80ex_intread_cb (Z80EX_CONTEXT *cpu, void *user_data);

  public:
  Z80_FDC(void);
  virtual ~Z80_FDC(void);

  bool trace(void);
  void trace(bool value);

  void execute(void);
  long long get_counter();
  byte_t trigger_irq(byte_t irq_vector);
  void add_callback(unsigned long long offset, Callback *cb, void *data);
  void register_ic(InterfaceCircuit *h);
  void unregister_ic(InterfaceCircuit *h);

  void reset(bool power_on = false);
  void power_on();

  word_t getPC(void) { return z80ex_get_reg(_context, regPC); }
  byte_t getI(void)   { return z80ex_get_reg(_context, regI); }
};

#endif /* __kc_z80_fdc_h */
