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

byte_t
DMA::in(word_t addr)
{
  byte_t val = 0xff;

  DBG(2, form("KCemu/DMA/in",
              "DMA::in(): %04xh: %02x\n",
              z80->getPC(), val));

  return val;
}

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
      write(reg, val);

      if (reg == DMA_REG_IRQ_CONTROL)
        {
          queue(DMA_REG_IRQ_CONTROL, val, 0x10, 0x10, DMA_REG_IRQ_VECTOR);
          queue(DMA_REG_IRQ_CONTROL, val, 0x08, 0x08, DMA_REG_PULSE_OFFSET);
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

void
DMA::out_base(word_t addr, byte_t val)
{
  if ((val & 0x80) == 0)
    {
      if ((val & 0x03) != 0x00)
        {
          write(DMA_REG_WR0, val);
          queue(DMA_REG_WR0, val, 0x40, 0x40, DMA_REG_BLOCK_SIZE_H);
          queue(DMA_REG_WR0, val, 0x20, 0x20, DMA_REG_BLOCK_SIZE_L);
          queue(DMA_REG_WR0, val, 0x10, 0x10, DMA_REG_START_ADDR_A_H);
          queue(DMA_REG_WR0, val, 0x08, 0x08, DMA_REG_START_ADDR_A_L);
          exec0();
        }
      else if ((val & 0x07) == 0x04)
        {
          write(DMA_REG_WR1, val);
          queue(DMA_REG_WR1, val, 0x40, 0x40, DMA_REG_TIMING_A);
          exec1();
        }
      else if ((val & 0x07) == 0x00)
        {
          write(DMA_REG_WR2, val);
          queue(DMA_REG_WR2, val, 0x40, 0x40, DMA_REG_TIMING_B);
          exec2();
        }
    }
  else
    {
      if ((val & 0x03) == 0x00)
        {
          write(DMA_REG_WR3, val);
          queue(DMA_REG_WR3, val, 0x10, 0x10, DMA_REG_COMPARE_BYTE);
          queue(DMA_REG_WR3, val, 0x08, 0x08, DMA_REG_MASK_BYTE);
          exec3();
        }
      else if ((val & 0x03) == 0x01)
        {
          write(DMA_REG_WR4, val);
          queue(DMA_REG_WR4, val, 0x10, 0x10, DMA_REG_IRQ_CONTROL);
          queue(DMA_REG_WR4, val, 0x08, 0x08, DMA_REG_START_ADDR_B_H);
          queue(DMA_REG_WR4, val, 0x04, 0x04, DMA_REG_START_ADDR_B_L);
          exec4();
        }
      else if ((val & 0xc7) == 0x82)
        {
          write(DMA_REG_WR5, val);
          exec5();
        }
      else if ((val & 0x03) == 0x03)
        {
          write(DMA_REG_WR6, val);
          queue(DMA_REG_WR4, val, 0xff, 0xbb, DMA_REG_READ_MASK);
          exec6();
        }
    }
}

void
DMA::queue(int base_reg, byte_t val, byte_t mask, byte_t result, int reg)
{
  if ((val & mask) != result)
    return;

  DBG(2, form("KCemu/DMA/out_reg",
              "DMA::queue(): %04xh: %s -> queue write to register %s\n",
              z80->getPC(), get_reg_name(base_reg), get_reg_name(reg)));

  _widx[++_wcnt] = reg;
}

void
DMA::write(int reg, byte_t val)
{
  DBG(2, form("KCemu/DMA/out_reg",
              "DMA::write(): %04xh: %s -> %02x\n",
              z80->getPC(), get_reg_name(reg), val));
  _regs[reg] = val;
}

const char *
DMA::get_text1(byte_t val, const char *text0, const char *text1)
{
  return val ? text1 : text0;
}

const char *
DMA::get_text2(byte_t val, const char *text0, const char *text1, const char *text2, const char *text3)
{
  switch (val & 3)
    {
    case 0: return text0;
    case 1: return text1;
    case 2: return text2;
    default: return text3;
    }
}

void
DMA::exec0(void)
{
  const char *r = get_reg_name(DMA_REG_WR0);
  DBG(2, form("KCemu/DMA/out_WR0",
              "DMA::exec():  %s: ------------------------------------\n"
              "DMA::exec():  %s: Mode                    =  %s\n"
              "DMA::exec():  %s: Direction               =  %s\n"
              "DMA::exec():  %s: A: Start Address (lo) ? =  %s\n"
              "DMA::exec():  %s: A: Start Address (hi) ? =  %s\n"
              "DMA::exec():  %s: Block Length (lo) ?     =  %s\n"
              "DMA::exec():  %s: Block Length (hi) ?     =  %s\n"
              "DMA::exec():  %s: ------------------------------------\n",
              r,
              r, get_text2(_regs[DMA_REG_WR0] & 3, "invalid", "Copy", "Search", "Copy + Search"),
              r, get_text1(_regs[DMA_REG_WR0] & 4, "B -> A", "A -> B"),
              r, get_text1(_regs[DMA_REG_WR0] & 8, "unchanged", "follows"),
              r, get_text1(_regs[DMA_REG_WR0] & 16, "unchanged", "follows"),
              r, get_text1(_regs[DMA_REG_WR0] & 32, "unchanged", "follows"),
              r, get_text1(_regs[DMA_REG_WR0] & 64, "unchanged", "follows"),
              r));
}

void
DMA::exec1(void)
{
  const char *r = get_reg_name(DMA_REG_WR1);
  DBG(2, form("KCemu/DMA/out_WR1",
              "DMA::exec():  %s: ------------------------------------\n"
              "DMA::exec():  %s: A: Interface      =  %s\n"
              "DMA::exec():  %s: A: Address Change =  %s\n"
              "DMA::exec():  %s: A: Timing ?       =  %s\n"
              "DMA::exec():  %s: ------------------------------------\n",
              r,
              r, get_text1(_regs[DMA_REG_WR1] & 8, "Memory", "I/O"),
              r, get_text2(_regs[DMA_REG_WR1] >> 4, "Decrement", "Increment", "Fix", "Fix"),
              r, get_text1(_regs[DMA_REG_WR1] & 64, "unchanged", "follows"),
              r));
}

void
DMA::exec2(void)
{
  const char *r = get_reg_name(DMA_REG_WR2);
  DBG(2, form("KCemu/DMA/out_WR2",
              "DMA::exec():  %s: ------------------------------------\n"
              "DMA::exec():  %s: B: Interface      =  %s\n"
              "DMA::exec():  %s: B: Address Change =  %s\n"
              "DMA::exec():  %s: B: Timing ?       =  %s\n"
              "DMA::exec():  %s: ------------------------------------\n",
              r,
              r, get_text1(_regs[DMA_REG_WR2] & 8, "Memory", "I/O"),
              r, get_text2(_regs[DMA_REG_WR2] >> 4, "Decrement", "Increment", "Fix", "Fix"),
              r, get_text1(_regs[DMA_REG_WR2] & 64, "unchanged", "follows"),
              r));
}

void
DMA::exec3(void)
{
  const char *r = get_reg_name(DMA_REG_WR3);
  DBG(2, form("KCemu/DMA/out_WR3",
              "DMA::exec():  %s: ------------------------------------\n"
              "DMA::exec():  %s: End DMA on active search =  %s\n"
              "DMA::exec():  %s: Enable Interrupts        =  %s\n"
              "DMA::exec():  %s: Enable DMA               =  %s\n"
              "DMA::exec():  %s: Mask Byte ?              =  %s\n"
              "DMA::exec():  %s: Compare Byte ?           =  %s\n"
              "DMA::exec():  %s: ------------------------------------\n",
              r,
              r, get_text1(_regs[DMA_REG_WR3] & 4, "No", "Yes"),
              r, get_text1(_regs[DMA_REG_WR3] & 32, "No", "Yes"),
              r, get_text1(_regs[DMA_REG_WR3] & 64, "No", "Yes"),
              r, get_text1(_regs[DMA_REG_WR3] & 8, "unchanged", "follows"),
              r, get_text1(_regs[DMA_REG_WR3] & 16, "unchanged", "follows"),
              r));
}

void
DMA::exec4(void)
{
  const char *r = get_reg_name(DMA_REG_WR4);
  DBG(2, form("KCemu/DMA/out_WR4",
              "DMA::exec():  %s: ------------------------------------\n"
              "DMA::exec():  %s: DMA Mode                =  %s\n"
              "DMA::exec():  %s: B: Start Address (lo) ? =  %s\n"
              "DMA::exec():  %s: B: Start Address (hi) ? =  %s\n"
              "DMA::exec():  %s: Interrupt Control ?     =  %s\n"
              "DMA::exec():  %s: ------------------------------------\n",
              r,
              r, get_text2(_regs[DMA_REG_WR4] >> 5, "Byte", "Continuous", "Burst", "invalid"),
              r, get_text1(_regs[DMA_REG_WR4] & 4, "unchanged", "follows"),
              r, get_text1(_regs[DMA_REG_WR4] & 8, "unchanged", "follows"),
              r, get_text1(_regs[DMA_REG_WR4] & 16, "unchanged", "follows"),
              r));
}

void
DMA::exec5(void)
{
  const char *r = get_reg_name(DMA_REG_WR5);
  DBG(2, form("KCemu/DMA/out_WR5",
              "DMA::exec():  %s: ------------------------------------\n"
              "DMA::exec():  %s: Polarity RDY     =  %s\n"
              "DMA::exec():  %s: Wait Function    =  %s\n"
              "DMA::exec():  %s: DMA Auto-Restart =  %s\n"
              "DMA::exec():  %s: ------------------------------------\n",
              r,
              r, get_text1(_regs[DMA_REG_WR5] & 8, "L (= /RDY)", "H (= RDY)"),
              r, get_text1(_regs[DMA_REG_WR5] & 16, "No", "Yes"),
              r, get_text1(_regs[DMA_REG_WR5] & 32, "No", "Yes"),
              r));
}

void
DMA::exec6_log(int reg, const char *text)
{
  const char *r = get_reg_name(reg);
  DBG(2, form("KCemu/DMA/out_WR6",
              "DMA::exec():  %s: ------------------------------------\n"
              "DMA::exec():  %s: %s\n"
              "DMA::exec():  %s: ------------------------------------\n",
              r, r, text, r));
}

void
DMA::exec6(void)
{
  switch (_regs[DMA_REG_WR6])
    {
    case 0xc3:
      exec6_log(DMA_REG_WR6, "RESET");
      break;
    case 0xc7:
      exec6_log(DMA_REG_WR6, "RESET TIMING PORT A");
      break;
    case 0xcb:
      exec6_log(DMA_REG_WR6, "RESET TIMING PORT B");
      break;
    case 0xcf:
      exec6_log(DMA_REG_WR6, "LOAD");
      break;
    case 0xd3:
      exec6_log(DMA_REG_WR6, "CONTINUE");
      break;
    case 0xaf:
      exec6_log(DMA_REG_WR6, "DISABLE INTERRUPT");
      break;
    case 0xab:
      exec6_log(DMA_REG_WR6, "ENABLE INTERRUPT");
      break;
    case 0xa3:
      exec6_log(DMA_REG_WR6, "RESET INTERRUPT");
      break;
    case 0xb7:
      exec6_log(DMA_REG_WR6, "DMA AFTER RETI");
      break;
    case 0xbf:
      exec6_log(DMA_REG_WR6, "READ STATUS");
      break;
    case 0x8b:
      exec6_log(DMA_REG_WR6, "REFRESH STATUS");
      break;
    case 0xa7:
      exec6_log(DMA_REG_WR6, "BEGIN READ");
      break;
    case 0xb3:
      exec6_log(DMA_REG_WR6, "FORCE READY");
      break;
    case 0x87:
      exec6_log(DMA_REG_WR6, "ENABLE DMA");
      break;
    case 0x83:
      exec6_log(DMA_REG_WR6, "DISABLE DMA");
      break;
    case 0xbb:
      exec6_log(DMA_REG_WR6, "LOAD STATUS MASK");
      break;
    default:
      exec6_log(DMA_REG_WR6, "INVALID");
      break;
    }
}
