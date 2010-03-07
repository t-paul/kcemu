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

#ifndef __kc_z80_h
#define __kc_z80_h

#include <list>

#include <z80ex/z80ex.h>

#include "kc/system.h"

#include "kc/ic.h"
#include "kc/cb.h"
#include "kc/cb_list.h"

class Z80
{
  public:
        Z80EX_CONTEXT *_context;

	bool _debug;
	bool _trace;
	bool _singlestep;
	bool _executestep;
        bool _enable_floppy_cpu;
	long _tracedelay;

        byte_t  _next_irq;
	dword_t _irq_line;
	dword_t _irq_mask;

	typedef std::list<InterfaceCircuit *> ic_list_t;

	static const int I_PERIOD;

	bool _do_quit;
	unsigned long long _counter;
	ic_list_t _ic_list;

        CallbackList _cb_list;

  private:
        void reset(word_t pc, bool power_on);
        
        /*callback that returns byte for a given adress*/
        static Z80EX_BYTE z80ex_dasm_readbyte_cb(Z80EX_WORD addr, void *user_data);

        /*read byte from memory <addr> -- called when RD & MREQ goes active.
        m1_state will be 1 if M1 signal is active*/
        static Z80EX_BYTE z80ex_mread_cb(Z80EX_CONTEXT *cpu, Z80EX_WORD addr, int m1_state, void *user_data);

        /*write <value> to memory <addr> -- called when WR & MREQ goes active*/
        static void z80ex_mwrite_cb(Z80EX_CONTEXT *cpu, Z80EX_WORD addr, Z80EX_BYTE value, void *user_data);

        /*read byte from <port> -- called when RD & IORQ goes active*/
        static Z80EX_BYTE z80ex_pread_cb(Z80EX_CONTEXT *cpu, Z80EX_WORD port, void *user_data);

        /*write <value> to <port> -- called when WR & IORQ goes active*/
        static void z80ex_pwrite_cb(Z80EX_CONTEXT *cpu, Z80EX_WORD port, Z80EX_BYTE value, void *user_data);

        /*read byte of interrupt vector -- called when M1 and IORQ goes active*/
        static Z80EX_BYTE z80ex_intread_cb(Z80EX_CONTEXT *cpu, void *user_data);

        /*called when the reti instruction is executed*/
        static void z80ex_reti_cb(Z80EX_CONTEXT *cpu, void *user_data);

  public:
	Z80(void);
	~Z80(void) {}
	bool run(void);

	void executestep(void);
	bool debug(void);
	void debug(bool value);
	bool trace(void);
	void trace(bool value);
	void tracedelay(long delay);
	bool singlestep();
	void singlestep(bool value);

	void nmi(void);
	void reset(void);
	void power_on(void);
	void jump(word_t pc);
	void reti(void);
	void register_ic(InterfaceCircuit *h);
	void unregister_ic(InterfaceCircuit *h);

	void addCallback(unsigned long long offset, Callback *cb, void *data);
	void remove_callback_listener(Callback *cb);
	inline unsigned long long getCounter(void) { return _counter; }

	dword_t get_irq_mask(void);
	void set_irq_line(dword_t mask);
	void reset_irq_line(dword_t mask);

        void printPC(void);

	void quit(void);

        word_t getAF(void) { return z80ex_get_reg(_context, regAF); }
        word_t getBC(void) { return z80ex_get_reg(_context, regBC); }
        word_t getDE(void) { return z80ex_get_reg(_context, regDE); }
        word_t getHL(void) { return z80ex_get_reg(_context, regHL); }
        word_t getIX(void) { return z80ex_get_reg(_context, regIX); }
        word_t getIY(void) { return z80ex_get_reg(_context, regIY); }
	word_t getPC(void) { return z80ex_get_reg(_context, regPC); }
        word_t getSP(void) { return z80ex_get_reg(_context, regSP); }

        word_t getAFs(void) { return z80ex_get_reg(_context, regAF_); }
        word_t getBCs(void) { return z80ex_get_reg(_context, regBC_); }
        word_t getDEs(void) { return z80ex_get_reg(_context, regDE_); }
        word_t getHLs(void) { return z80ex_get_reg(_context, regHL_); }
        
        byte_t getIFF(void) { return z80ex_get_reg(_context, regIFF1); }
        byte_t getI(void)   { return z80ex_get_reg(_context, regI); }

	bool get_halt(void) { return z80ex_doing_halt(_context); }

        void start_floppy_cpu(void);
        void halt_floppy_cpu(bool power_on);
};

#endif /* __kc_z80_h */
