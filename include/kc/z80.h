/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: z80.h,v 1.20 2002/06/09 14:24:32 torsten_paul Exp $
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

#ifndef __kc_z80_h
#define __kc_z80_h

/*
 *  we include a patched version of the Z80-core include file
 *  due to name conflicts...
 *
 *  it uses a 'struct Z80' that clashes with the 'class Z80'
 *  we want define here.
 *
 *  'struct Z80' is changed to 'struct _Z80'
 *
 *  another problem is the union pair that is used in pair.h which
 *  is part of the STL.
 *
 *  'union pair' is changed to 'union reg_pair'
 */

#include <list>

#include "kc/system.h"

extern "C" {
#include "z80core/z80.h"
}

#include "kc/ic.h"
#include "kc/cb.h"
#include "kc/cb_list.h"

class Z80
{
  public:
	_Z80 _regs;

	bool _halt;
	bool _debug;
	bool _trace;
	bool _singlestep;
	bool _executestep;
        bool _enable_floppy_cpu;
	long _tracedelay;

	dword_t _irq_line;
	dword_t _daisy_chain_irq_mask;
	InterfaceCircuit *_daisy_chain_first;
	InterfaceCircuit *_daisy_chain_last;

	typedef std::list<InterfaceCircuit *> ic_list_t;

	static const int I_PERIOD;

	bool _do_quit;
	unsigned long long _counter;
	ic_list_t _ic_list;

        CallbackList _cb_list;

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

	void reset(word_t pc, bool power_on = false);
	void power_on(word_t pc);
	void jump(word_t pc);
	void reti(void);
	void register_ic(InterfaceCircuit *h);
	void unregister_ic(InterfaceCircuit *h);
	int triggerIrq(int vector);
	void handleIrq(int vector);
	bool irq_enabled(void);

	void addCallback(unsigned long long offset, Callback *cb, void *data);
	void remove_callback_listener(Callback *cb);
	inline unsigned long long getCounter(void) { return _counter; }

	void daisy_chain_set_first(InterfaceCircuit *ic);
	void daisy_chain_set_last(InterfaceCircuit *ic);
	dword_t daisy_chain_get_irqmask(void);
	void set_irq_line(dword_t mask);
	void reset_irq_line(dword_t mask);
	word_t irq_ack(void);

        void printPC(void);

	void quit(void);

        word_t getAF(void) { return _regs.AF.W; }
        word_t getBC(void) { return _regs.BC.W; }
        word_t getDE(void) { return _regs.DE.W; }
        word_t getHL(void) { return _regs.HL.W; }
        word_t getIX(void) { return _regs.IX.W; }
        word_t getIY(void) { return _regs.IY.W; }
	word_t getPC(void) { return _regs.PC.W; }
        word_t getSP(void) { return _regs.SP.W; }

        word_t getAFs(void) { return _regs.AF1.W; }
        word_t getBCs(void) { return _regs.BC1.W; }
        word_t getDEs(void) { return _regs.DE1.W; }
        word_t getHLs(void) { return _regs.HL1.W; }
        
        byte_t getIFF(void) { return _regs.IFF; }
        byte_t getI(void)   { return _regs.I; }

	bool get_halt(void) { return _halt; }

        void start_floppy_cpu(void);
        void halt_floppy_cpu(void);
};

#endif /* __kc_z80_h */
