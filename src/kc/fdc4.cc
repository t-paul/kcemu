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

#include <ctype.h>

#include "kc/system.h"

#include "kc/fdc4.h"
#include "kc/z80_fdc.h"

#include "libdbg/dbg.h"

FDC4::FDC4(void)
{
  fdc_z80->register_ic(this);
}

FDC4::~FDC4(void)
{
  fdc_z80->unregister_ic(this);
}

long long
FDC4::get_counter()
{
  return fdc_z80->get_counter();
}

void
FDC4::add_callback(unsigned long long offset, Callback *cb, void *data)
{
  fdc_z80->add_callback(offset, cb, data);
}

byte_t
FDC4::in(word_t addr)
{
  byte_t val = 0;

  word_t pc = fdc_z80->getPC();

  switch (addr & 0xff)
    {
    case 0xf0: // D004 (KC85/4)
      /* CS-FDC (Chipselect) */
      val = get_msr();
      DBG(2, form("KCemu/FDC/in_F0",
                  "FDC::in(): %04xh addr = %04x, val = %02x [%c]\n",
                  pc, addr, val, isprint(val) ? val : '.'));
      break;
    case 0xf1: // D004 (KC85/4)
      val = in_data(addr);
      DBG(2, form("KCemu/FDC/in_F1",
                  "FDC::in(): %04xh addr = %04x, val = %02x [%c]\n",
                  pc, addr, val, isprint(val) ? val : '.'));
      break;
    case 0xf2: // D004 (KC85/4)
      /* DAK-FDC (DMA-Acknowledge) */
      val = read_byte();
      DBG(2, form("KCemu/FDC/in_F2",
                  "FDC::in(): %04xh addr = %04x, val = %02x [%c]\n",
                  pc, addr, val, isprint(val) ? val : '.'));
      break;
    case 0xf4: // D004 (KC85/4)
      /* 
       *  Input-Gate:
       *
       *  Bit 7   DRQ (DMA-Request)
       *  Bit 6   INT (Interrupt)
       *  Bit 5   RDY (Drive-Ready)
       *  Bit 4   IDX (Index - Spuranfang)
       */
      val = get_input_gate();
      DBG(2, form("KCemu/FDC/in_F4",
                  "FDC::in(): %04xh addr = %04x, val = %02x [%c]\n",
                  pc, addr, val, isprint(val) ? val : '.'));
      break;
    case 0xf6: // D004 (KC85/4)
      /* Select-Latch */
      val = 0x00;
      DBG(2, form("KCemu/FDC/in_F6",
                  "FDC::in(): %04xh addr = %04x, val = %02x [%c]\n",
                  pc, addr, val, isprint(val) ? val : '.'));
      break;
    case 0xf8: // D004 (KC85/4)
      /* TC-FDC (Terminalcount) - End of DMA Transfer */
      val = 0x00;
      DBG(2, form("KCemu/FDC/in_F8",
                  "FDC::in(): %04xh addr = %04x, val = %02x [%c]\n",
                  pc, addr, val, isprint(val) ? val : '.'));
      break;
    }

  DBG(2, form("KCemu/FDC/in",
              "FDC::in(): %04xh  addr = %04x, val = %02x\n",
              pc, addr, val));

  return val;
}

void
FDC4::out(word_t addr, byte_t val)
{
  word_t pc = fdc_z80->getPC();

  DBG(2, form("KCemu/FDC/out",
              "FDC::out(): %04xh (I=%02x): addr = %04x, val = %02x [%c]\n",
              pc, fdc_z80->getI(), addr, val, isprint(val) ? val : '.'));

  switch (addr & 0xff)
    {
    case 0xf1: // D004 (KC85/4)
      DBG(2, form("KCemu/FDC/out_F1",
                  "FDC::out(): %04xh addr = %04x, val = %02x [%c]\n",
                  pc, addr, val, isprint(val) ? val : '.'));
      out_data(addr, val);
      break;
    case 0xf2: // D004 (KC85/4)
      DBG(2, form("KCemu/FDC/out_F2",
                  "FDC::out(): %04xh addr = %04x, val = %02x [%c]\n",
                  pc, addr, val, isprint(val) ? val : '.'));
      write_byte(val);
      break;
    case 0xf6: // D004 (KC85/4)
      DBG(2, form("KCemu/FDC/out_F6",
                  "FDC::out(): %04xh addr = %04x, val = %02x [%c]\n",
                  pc, addr, val, isprint(val) ? val : '.'));
      drive_select(val & 0x0f);
      break;
    case 0xf8: // D004 (KC85/4)
      DBG(2, form("KCemu/FDC/out_F8",
                  "FDC::out(): %04xh Terminal Count %02x\n",
                  pc, val));

      set_state(FDC_STATE_RESULT);
      set_input_gate(0x40, 0x00);
      break;
    default:
      DBG(2, form("KCemu/FDC/out_unhandled",
                  "FDC::out(): %04xh addr = %04x, val = %02x [%c]\n",
                  pc, addr, val, isprint(val) ? val : '.'));
      break;
    }
}
