/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2009 Torsten Paul
 *
 *  $Id$
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

#ifndef __kc_dma_h
#define __kc_dma_h

#include "kc/system.h"

#include "kc/ic.h"
#include "kc/ports.h"

class DMA : public InterfaceCircuit, public PortInterface
{
private:
  enum {
    DMA_REG_WR0              =  0,
    DMA_REG_START_ADDR_A_L   =  1,
    DMA_REG_START_ADDR_A_H   =  2,
    DMA_REG_BLOCK_SIZE_L     =  3,
    DMA_REG_BLOCK_SIZE_H     =  4,
    DMA_REG_WR1              =  5,
    DMA_REG_TIMING_A         =  6,
    DMA_REG_WR2              =  7,
    DMA_REG_TIMING_B         =  8,
    DMA_REG_WR3              =  9,
    DMA_REG_MASK_BYTE        = 10,
    DMA_REG_COMPARE_BYTE     = 11,
    DMA_REG_WR4              = 12,
    DMA_REG_START_ADDR_B_L   = 13,
    DMA_REG_START_ADDR_B_H   = 14,
    DMA_REG_IRQ_CONTROL      = 15,
    DMA_REG_PULSE_OFFSET     = 16,
    DMA_REG_IRQ_VECTOR       = 17,
    DMA_REG_WR5              = 18,
    DMA_REG_WR6              = 19,
    DMA_REG_READ_MASK        = 20,

    DMA_REG_RR0              = 0,
    DMA_REG_RR1              = 1,
    DMA_REG_RR2              = 2,
    DMA_REG_RR3              = 3,
    DMA_REG_RR4              = 4,
    DMA_REG_RR5              = 5,
    DMA_REG_RR6              = 6,
  };
protected:
  byte_t _rcnt;
  byte_t _ridx[6];
  byte_t _wcnt;
  byte_t _widx[6];
  byte_t _regs[21];
  
  /*
   *  interrupt mask for daisy chain handling
   */
  dword_t _z80_irq_mask;

protected:
  virtual const char * get_reg_name(int reg);
  virtual void out_base(word_t addr, byte_t val);
  virtual void write(int reg, byte_t val);
  virtual void queue(int base_reg, byte_t val, byte_t mask, byte_t result, int reg);
  virtual void exec0(void);
  virtual void exec1(void);
  virtual void exec2(void);
  virtual void exec3(void);
  virtual void exec4(void);
  virtual void exec5(void);
  virtual void exec6(void);
  virtual void exec6_log(int reg, const char *text);
  virtual const char * get_text1(byte_t val, const char *text0, const char *text1);
  virtual const char * get_text2(byte_t val, const char *text0, const char *text1, const char *text2, const char *text3);
  
public:
  DMA(void);
  virtual ~DMA(void);

  /*
   *  InterfaceCircuit
   */
  virtual void reti(void);
  virtual void irqreq(void);
  virtual word_t irqack(void);
  virtual void reset(bool power_on = false);

  /*
   *  PortInterface
   */
  virtual byte_t in(word_t addr);
  virtual void out(word_t addr, byte_t val);
};

#endif /* __kc_dma_h */
