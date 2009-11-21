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

#include <iostream>
#include <iomanip>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/dma.h"
#include "kc/z80.h"
#include "kc/tape.h"
#include "kc/memory.h"

#include "libdbg/dbg.h"

using namespace std;

DMA::DMA(void) : InterfaceCircuit("DMA")
{
  reset(true);
  z80->register_ic(this);
  _z80_irq_mask = z80->get_irq_mask();
}

DMA::~DMA(void)
{
  z80->unregister_ic(this);
}

void
DMA::reset(bool power_on)
{
  _rcnt = 0;
  _wcnt = 0;
}

void
DMA::irqreq(void)
{
  DBG(2, form("KCemu/DMA/reti",
              "DMA::irqreq(): %04xh\n",
              z80->getPC()));
  z80->set_irq_line(_z80_irq_mask);
}

word_t
DMA::irqack(void)
{
  return IRQ_NOT_ACK;
}

void
DMA::reti(void) { }

byte_t
DMA::in(word_t addr) { }

void
DMA::out(word_t addr, byte_t val)
{
  DBG(2, form("KCemu/DMA/out",
              "DMA::out(): %04xh: %02x\n",
              z80->getPC(), val));

  if (_wcnt == 0)
    {
      out_base(addr, val);
    }
  else
    {
      int reg = _widx[_wcnt--];
      _regs[reg] = val;

      if (reg == DMA_REG_IRQ_CONTROL)
        {
          if (val & 0x10)
            _widx[++_wcnt] = DMA_REG_IRQ_VECTOR;
          if (val & 0x08)
            _widx[++_wcnt] = DMA_REG_PULSE_OFFSET;
        }
      else if (reg == DMA_REG_READ_MASK)
        {
          if (val & 0x40)
            _ridx[++_rcnt] = DMA_REG_RR6;
          if (val & 0x20)
            _ridx[++_rcnt] = DMA_REG_RR5;
          if (val & 0x10)
            _ridx[++_rcnt] = DMA_REG_RR4;
          if (val & 0x08)
            _ridx[++_rcnt] = DMA_REG_RR3;
          if (val & 0x04)
            _ridx[++_rcnt] = DMA_REG_RR2;
          if (val & 0x02)
            _ridx[++_rcnt] = DMA_REG_RR1;
          if (val & 0x01)
            _ridx[++_rcnt] = DMA_REG_RR0;
        }
    }
}

const char *
DMA::get_reg_name(int reg)
{
  switch (reg)
    {
    case DMA_REG_WR0: return "WR0";
    case DMA_REG_WR1: return "WR1";
    case DMA_REG_WR2: return "WR2";
    case DMA_REG_WR3: return "WR3";
    case DMA_REG_WR4: return "WR4";
    case DMA_REG_WR5: return "WR5";
    case DMA_REG_WR6: return "WR6";

    case DMA_REG_START_ADDR_A_L: return "START ADDR A (low)";
    case DMA_REG_START_ADDR_A_H: return "START ADDR A (high)";
    case DMA_REG_BLOCK_SIZE_L:   return "BLOCK SIZE (low)";
    case DMA_REG_BLOCK_SIZE_H:   return "BLOCK SIZE (high)";
    case DMA_REG_TIMING_A:       return "TIMING A";
    case DMA_REG_TIMING_B:       return "TIMING B";
    case DMA_REG_MASK_BYTE:      return "MASK BYTE";
    case DMA_REG_COMPARE_BYTE:   return "COMPARE BYTE";
    case DMA_REG_START_ADDR_B_L: return "START ADDR B (low)";
    case DMA_REG_START_ADDR_B_H: return "START ADDR B (high)";
    case DMA_REG_IRQ_CONTROL:    return "IRQ CONTROL";
    case DMA_REG_PULSE_OFFSET:   return "PULSE OFFSET";
    case DMA_REG_IRQ_VECTOR:     return "IRQ VECTOR";
    case DMA_REG_READ_MASK:      return "READ MASK";
    }

  return "???";
}

void
DMA::queue_write(int base_reg, byte_t val, byte_t mask, byte_t result, int reg)
{
  if ((val & mask) != result)
    return;

  DBG(2, form("KCemu/DMA/out",
              "DMA::queue_write(): %04xh: %s - queue write to register %s\n",
              z80->getPC(), get_reg_name(base_reg), get_reg_name(reg)));

  _widx[++_wcnt] = reg;
}

void
DMA::out_base(word_t addr, byte_t val)
{
  if ((val & 0x80) == 0)
    {
      if ((val & 0x03) != 0x00)
        {
          DBG(2, form("KCemu/DMA/out",
                      "DMA::out_base():    %04xh: %s -> %02x\n",
                      z80->getPC(), get_reg_name(DMA_REG_WR0), val));
          _regs[DMA_REG_WR0] = val;
          queue_write(DMA_REG_WR0, val, 0x40, 0x40, DMA_REG_BLOCK_SIZE_H);
          queue_write(DMA_REG_WR0, val, 0x20, 0x20, DMA_REG_BLOCK_SIZE_L);
          queue_write(DMA_REG_WR0, val, 0x10, 0x10, DMA_REG_START_ADDR_A_H);
          queue_write(DMA_REG_WR0, val, 0x08, 0x08, DMA_REG_START_ADDR_A_L);
        }
      else if ((val & 0x07) == 0x04)
        {
          DBG(2, form("KCemu/DMA/out",
                      "DMA::out_base():    %04xh: %s -> %02x\n",
                      z80->getPC(), get_reg_name(DMA_REG_WR1), val));
          _regs[DMA_REG_WR1] = val;
          queue_write(DMA_REG_WR1, val, 0x40, 0x40, DMA_REG_TIMING_A);
        }
      else if ((val & 0x07) == 0x00)
        {
          DBG(2, form("KCemu/DMA/out",
                      "DMA::out_base():    %04xh: %s -> %02x\n",
                      z80->getPC(), get_reg_name(DMA_REG_WR2), val));
          _regs[DMA_REG_WR2] = val;
          queue_write(DMA_REG_WR2, val, 0x40, 0x40, DMA_REG_TIMING_B);
        }
    }
  else
    {
      if ((val & 0x03) == 0x00)
        {
          DBG(2, form("KCemu/DMA/out",
                      "DMA::out_base():    %04xh: %s -> %02x\n",
                      z80->getPC(), get_reg_name(DMA_REG_WR3), val));
          _regs[DMA_REG_WR3] = val;
          queue_write(DMA_REG_WR3, val, 0x10, 0x10, DMA_REG_COMPARE_BYTE);
          queue_write(DMA_REG_WR3, val, 0x08, 0x08, DMA_REG_MASK_BYTE);
        }
      else if ((val & 0x03) == 0x01)
        {
          DBG(2, form("KCemu/DMA/out",
                      "DMA::out_base():    %04xh: %s -> %02x\n",
                      z80->getPC(), get_reg_name(DMA_REG_WR4), val));
          _regs[DMA_REG_WR4] = val;
          queue_write(DMA_REG_WR4, val, 0x10, 0x10, DMA_REG_IRQ_CONTROL);
          queue_write(DMA_REG_WR4, val, 0x08, 0x08, DMA_REG_START_ADDR_B_H);
          queue_write(DMA_REG_WR4, val, 0x04, 0x04, DMA_REG_START_ADDR_B_L);
        }
      else if ((val & 0xc7) == 0x82)
        {
          DBG(2, form("KCemu/DMA/out",
                      "DMA::out_base():    %04xh: %s -> %02x\n",
                      z80->getPC(), get_reg_name(DMA_REG_WR5), val));
          _regs[DMA_REG_WR5] = val;
        }
      else if ((val & 0x03) == 0x03)
        {
          DBG(2, form("KCemu/DMA/out",
                      "DMA::out_base():    %04xh: %s -> %02x\n",
                      z80->getPC(), get_reg_name(DMA_REG_WR6), val));
          _regs[DMA_REG_WR6] = val;
          queue_write(DMA_REG_WR4, val, 0xff, 0xbb, DMA_REG_READ_MASK);
        }
    }
}