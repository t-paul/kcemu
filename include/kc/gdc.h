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

#ifndef __kc_gdc_h
#define __kc_gdc_h

#include "kc/system.h"

#include "kc/ic.h"
#include "kc/ports.h"

class GDC : public PortInterface, public InterfaceCircuit
{
 private:
  int _idx;
  int _ptr;
  int _pptr;
  int _ridx;
  int _mask;
  int _mask_c;
  int _control;
  int _figs_dc;
  int _screen_on;
  int _nr_of_lines;
  int _cursor_top;
  int _cursor_bottom;
  byte_t _cmd;
  byte_t _arg[4096];
  byte_t _mem[65536];
  byte_t _col[65536];
  byte_t _pram[16];

 protected:
  void info(void);

 public:
  GDC(void);
  virtual ~GDC(void);

  virtual byte_t in(word_t addr);
  virtual void out(word_t addr, byte_t val);

  virtual long get_pram_SAD(int idx);
  virtual long get_pram_LEN(int idx);

  virtual byte_t get_mem(int addr);
  virtual byte_t get_col(int addr);
  virtual bool   get_cursor(int addr);
  virtual bool   get_cursor(int addr, int line);
  virtual int get_screen_on(void);
  virtual int get_nr_of_lines(void);

  virtual void v_retrace(bool value);
  
  /*
   *  InterfaceCircuit
   */
  virtual void reti(void);
  virtual void irqreq(void);
  virtual word_t irqack(void);
  virtual void reset(bool power_on = false);
};

#endif /* __kc_gdc_h */
