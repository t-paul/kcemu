/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: fdc.cc,v 1.9 2002/06/09 14:24:33 torsten_paul Exp $
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

#include <ctype.h>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/fdc.h"
#include "kc/fdc_cmd.h"

#include "libdbg/dbg.h"

extern "C" {
#include "z80core2/z80.h"
}

FloppyState::FloppyState(byte_t head, byte_t cylinder, byte_t sector, Floppy *floppy)
{
  _head = head;
  _cylinder = cylinder;
  _sector = sector;
  _floppy = floppy;
}

FloppyState::~FloppyState(void)
{
}

SectorDesc::SectorDesc(long size, byte_t *buf)
{
  _buf = buf;
  _size = size;
}

FDC::FDC(void) : InterfaceCircuit("FDC")
{
  _cmds[0x00] = new FDC_CMD_INVALID(this);
  _cmds[0x01] = new FDC_CMD_INVALID(this);
  _cmds[0x02] = new FDC_CMD_READ_TRACK(this);
  _cmds[0x03] = new FDC_CMD_SPECIFY(this);
  _cmds[0x04] = new FDC_CMD_SENSE_DRIVE_STATUS(this);
  _cmds[0x05] = new FDC_CMD_WRITE_DATA(this);
  _cmds[0x06] = new FDC_CMD_READ_DATA(this);
  _cmds[0x07] = new FDC_CMD_RECALIBRATE(this);
  _cmds[0x08] = new FDC_CMD_SENSE_INTERRUPT_STATUS(this);
  _cmds[0x09] = new FDC_CMD_WRITE_DELETED_DATA(this);
  _cmds[0x0a] = new FDC_CMD_READ_ID(this);
  _cmds[0x0b] = new FDC_CMD_INVALID(this);
  _cmds[0x0c] = new FDC_CMD_READ_DELETED_DATA(this);
  _cmds[0x0d] = new FDC_CMD_FORMAT_A_TRACK(this);
  _cmds[0x0e] = new FDC_CMD_INVALID(this);
  _cmds[0x0f] = new FDC_CMD_SEEK(this);
  _cmds[0x10] = new FDC_CMD_INVALID(this);
  _cmds[0x11] = new FDC_CMD_SCAN_EQUAL(this);
  _cmds[0x12] = new FDC_CMD_INVALID(this);
  _cmds[0x13] = new FDC_CMD_INVALID(this);
  _cmds[0x14] = new FDC_CMD_INVALID(this);
  _cmds[0x15] = new FDC_CMD_INVALID(this);
  _cmds[0x16] = new FDC_CMD_INVALID(this);
  _cmds[0x17] = new FDC_CMD_INVALID(this);
  _cmds[0x18] = new FDC_CMD_INVALID(this);
  _cmds[0x19] = new FDC_CMD_SCAN_LOW_OR_EQUAL(this);
  _cmds[0x1a] = new FDC_CMD_INVALID(this);
  _cmds[0x1b] = new FDC_CMD_INVALID(this);
  _cmds[0x1c] = new FDC_CMD_INVALID(this);
  _cmds[0x1d] = new FDC_CMD_SCAN_HIGH_OR_EQUAL(this);
  _cmds[0x1e] = new FDC_CMD_INVALID(this);
  _cmds[0x1f] = new FDC_CMD_INVALID(this);

  _cur_cmd = 0;
  _cur_floppy = 0;
  _read_bytes = 0;
  _MSR = 0;
  _ST0 = 0;
  _ST1 = 0;
  _ST2 = 0;
  _ST3 = 0;
  _INPUT_GATE = 0x60;
  _selected_unit = 0;
  _terminal_count = false;

  _fstate[0] = new FloppyState(0, 0, 1, new Floppy("attach-1"));
  _fstate[1] = new FloppyState(0, 0, 1, new Floppy("attach-2"));
  _fstate[2] = new FloppyState(0, 0, 1, new Floppy("attach-3"));
  _fstate[3] = new FloppyState(0, 0, 1, new Floppy("attach-4"));

  set_state(FDC_STATE_IDLE);
}

FDC::~FDC(void)
{
}

byte_t
FDC::in(word_t addr)
{
  Z80_Regs r;
  byte_t val = 0;

  Z80_GetRegs(&r);

  switch (addr & 0xff)
    {
    case 0x40: // BIC
    case 0x98: // CPM-Z9 module (Status Register)
    case 0x10: // CPM-Z9 module (Status Register) ???
    case 0xf0: // D004 (KC85/4)
      /* CS-FDC (Chipselect) */
      val = _MSR;
      DBG(2, form("KCemu/FDC/in_F0",
                  "FDC::in(): %04xh addr = %04x, val = %02x [%c]\n",
                  r.PC.D, addr, val, isprint(val) ? val : '.'));
      break;
    case 0x41: // BIC
    case 0x99: // CPM-Z9 module (Data Register)
    case 0x11: // CPM-Z9 module (Data Register) ??
    case 0xf1: // D004 (KC85/4)
      val = in_F1(addr);
      DBG(2, form("KCemu/FDC/in_F1",
                  "FDC::in(): %04xh addr = %04x, val = %02x [%c]\n",
                  r.PC.D, addr, val, isprint(val) ? val : '.'));
      break;
    case 0x42: // BIC
    case 0xf2: // D004 (KC85/4)
      /* DAK-FDC (DMA-Acknowledge) */
      val = read_byte();
      DBG(2, form("KCemu/FDC/in_F2",
                  "FDC::in(): %04xh addr = %04x, val = %02x [%c]\n",
                  r.PC.D, addr, val, isprint(val) ? val : '.'));
      break;
    case 0x44: // BIC
    case 0xf4: // D004 (KC85/4)
      /* 
       *  Input-Gate:
       *
       *  Bit 7   DRQ (DMA-Request)
       *  Bit 6   INT (Interrupt)
       *  Bit 5   RDY (Drive-Ready)
       *  Bit 4   IDX (Index - Spuranfang)
       */
      val = _INPUT_GATE;
      DBG(2, form("KCemu/FDC/in_F4",
                  "FDC::in(): %04xh addr = %04x, val = %02x [%c]\n",
                  r.PC.D, addr, val, isprint(val) ? val : '.'));
      break;
    case 0x46: // BIC
    case 0xf6: // D004 (KC85/4)
      /* Select-Latch */
      val = 0x00;
      DBG(2, form("KCemu/FDC/in_F6",
                  "FDC::in(): %04xh addr = %04x, val = %02x [%c]\n",
                  r.PC.D, addr, val, isprint(val) ? val : '.'));
      break;
    case 0x48: // BIC
    case 0xf8: // D004 (KC85/4)
      /* TC-FDC (Terminalcount) - End of DMA Transfer */
      val = 0x00;
      DBG(2, form("KCemu/FDC/in_F8",
                  "FDC::in(): %04xh addr = %04x, val = %02x [%c]\n",
                  r.PC.D, addr, val, isprint(val) ? val : '.'));
      break;
    case 0xa0: // CPM-Z9 module
      break;
    }

  DBG(2, form("KCemu/FDC/in",
              "FDC::in(): %04xh  addr = %04x, val = %02x\n",
              r.PC.D, addr, val));

  return val;
}

void
FDC::out(word_t addr, byte_t val)
{
  Z80_Regs r;

  Z80_GetRegs(&r);
  DBG(2, form("KCemu/FDC/out",
              "FDC::out(): %04xh (I=%02x): addr = %04x, val = %02x [%c]\n",
              r.PC.D, r.I, addr, val, isprint(val) ? val : '.'));

  switch (addr & 0xff)
    {
    case 0x41: // BIC
    case 0x99: // CPM-Z9 module (Status Register)
    case 0xf1: // D004 (KC85/4)
      DBG(2, form("KCemu/FDC/out_F1",
                  "FDC::out(): %04xh addr = %04x, val = %02x [%c]\n",
                  r.PC.D, addr, val, isprint(val) ? val : '.'));
      out_F1(addr, val);
//      if (val == 0x45)
//        Z80_Trace = 1;
      break;
    case 0x42: // BIC
    case 0x98: // CPM-Z9 module (Data Register)
    case 0xf2: // D004 (KC85/4)
      DBG(2, form("KCemu/FDC/out_F2",
                  "FDC::out(): %04xh addr = %04x, val = %02x [%c]\n",
                  r.PC.D, addr, val, isprint(val) ? val : '.'));
      write_byte(val);
      break;
    case 0x46: // BIC
    case 0xf6: // D004 (KC85/4)
      DBG(2, form("KCemu/FDC/out_F6",
                  "FDC::out(): %04xh addr = %04x, val = %02x [%c]\n",
                  r.PC.D, addr, val, isprint(val) ? val : '.'));
      break;
    case 0x48: // BIC
    case 0xf8: // D004 (KC85/4)
      DBG(2, form("KCemu/FDC/out_F8",
                  "FDC::out(): %04xh TC %02x\n",
                  r.PC.D, val));
      if (val == 0x00)
	set_state(FDC_STATE_IDLE);
      if (val == 0x11)
	set_state(FDC_STATE_RESULT);
      if (val == 0xc0) // kc85/4
	set_state(FDC_STATE_RESULT);
      set_input_gate(0x40, 0x00);
      break;
    case 0xa0: // CPM-Z9 module
      set_terminal_count((val & 0x10) == 0x10);

      if ((val & 0x20) == 0x20)
	; // RESET

      break;
    default:
      DBG(2, form("KCemu/FDC/out_unhandled",
                  "FDC::out(): %04xh addr = %04x, val = %02x [%c]\n",
                  r.PC.D, addr, val, isprint(val) ? val : '.'));
      break;
    }
}

byte_t
FDC::in_F1(word_t addr)
{
  byte_t val;
  
  val = 0xff;
  switch (_state)
    {
    case FDC_STATE_RESULT:
      val = _cur_cmd->read_result();
      DBG(2, form("KCemu/FDC/in_F1",
                 "FDC::in(): addr = %04x, val = %02x [%c] FDC_STATE_RESULT\n",
                 addr, val, isprint(val) ? val : '.'));
      break;
    case FDC_STATE_DATA:
      val = read_byte();
      DBG(2, form("KCemu/FDC/in_F1",
                 "FDC::in(): addr = %04x, val = %02x [%c] FDC_STATE_DATA\n",
                 addr, val, isprint(val) ? val : '.'));
      break;
    default:
      break;
    }

  return val;
}

/*
 *  data input from disk
 */
byte_t
FDC::read_byte(void)
{
  if (_cur_cmd)
    return _cur_cmd->read_byte();

  return 0xff;
}

/*
 *  data output to disk ???
 */
void
FDC::write_byte(byte_t val)
{
  if (_cur_cmd)
    _cur_cmd->write_byte(val);
}

/*
 *  command output to floppy controller
 */
void
FDC::out_F1(word_t addr, byte_t val)
{
  switch (_state)
    {
    case FDC_STATE_IDLE:
      DBG(2, form("KCemu/FDC/out_F1",
		  "FDC::out(): addr = %04x, val = %02x [%c] FDC_STATE_IDLE\n",
		  addr, val, isprint(val) ? val : '.'));
      _cur_cmd = _cmds[val & 0x1f];
      _cur_cmd->start(val);
      break;
    case FDC_STATE_COMMAND:
      DBG(2, form("KCemu/FDC/out_F1",
		  "FDC::out(): addr = %04x, val = %02x [%c] FDC_STATE_COMMAND\n",
		  addr, val, isprint(val) ? val : '.'));
      _cur_cmd->write_arg(val);
      break;
    case FDC_STATE_DATA:
      DBG(2, form("KCemu/FDC/out_F1",
		  "FDC::out(): addr = %04x, val = %02x [%c] FDC_STATE_DATA\n",
		  addr, val, isprint(val) ? val : '.'));
      write_byte(val);
      break;
    default:
      break;
    }
}

void
FDC::select_floppy(int floppy_nr)
{
  DBG(2, form("KCemu/FDC/select_floppy",
              "FDC::select_floppy(): selecting floppy %d\n",
              floppy_nr));
  _selected_unit = floppy_nr;
  _cur_floppy = _fstate[floppy_nr];
}

Floppy *
FDC::get_floppy(void)
{
  if (_cur_floppy == 0)
    return 0;

  return _cur_floppy->get_floppy();
}

Floppy *
FDC::get_floppy(int idx)
{
  if (idx < 0)
    return NULL;
  if (idx > 3)
    return NULL;

  return _fstate[idx]->get_floppy();
}

int
FDC::get_head(void)
{
  if (_cur_floppy == 0)
    return -1;
  return _cur_floppy->get_head();
}

int
FDC::get_cylinder(void)
{
  if (_cur_floppy == 0)
    return -1;
  return _cur_floppy->get_cylinder();
}

int
FDC::get_sector(void)
{
  if (_cur_floppy == 0)
    return -1;
  return _cur_floppy->get_sector();
}

void
FDC::set_state(fdc_state_t state)
{
  _state = state;

  _MSR = 0;
  switch (_state)
    {
    case FDC_STATE_IDLE:
      _MSR |= ST_MAIN_RQM;
      DBG(2, form("KCemu/FDC/state",
                  "FDC::set_state(): FDC_STATE_IDLE    -> MSR: %02x\n",
                  _MSR));
      break;
    case FDC_STATE_COMMAND:
      _MSR |= ST_MAIN_READ_WRITE;
      _MSR |= ST_MAIN_RQM;
      DBG(2, form("KCemu/FDC/state",
                  "FDC::set_state(): FDC_STATE_COMMAND -> MSR: %02x\n",
                  _MSR));
      break;
    case FDC_STATE_EXECUTE:
      _MSR |= ST_MAIN_NON_DMA;
      _MSR |= ST_MAIN_DIO;
      DBG(2, form("KCemu/FDC/state",
                  "FDC::set_state(): FDC_STATE_EXECUTE -> MSR: %02x\n",
                  _MSR));
      break;
    case FDC_STATE_DATA:
      _MSR |= ST_MAIN_READ_WRITE;
      _MSR |= ST_MAIN_NON_DMA;
      _MSR |= ST_MAIN_DIO;
      _MSR |= ST_MAIN_RQM;
      DBG(2, form("KCemu/FDC/state",
                  "FDC::set_state(): FDC_STATE_DATA    -> MSR: %02x\n",
                  _MSR));
      break;
    case FDC_STATE_RESULT:
      _MSR |= ST_MAIN_READ_WRITE;
      _MSR |= ST_MAIN_DIO;
      _MSR |= ST_MAIN_RQM;
      DBG(2, form("KCemu/FDC/state",
                  "FDC::set_state(): FDC_STATE_RESULT  -> MSR: %02x\n",
                  _MSR));
      break;
    }
}

bool
FDC::seek(byte_t head, byte_t cylinder, byte_t sector)
{
  if (_cur_floppy == 0)
    return false;

  _cur_floppy->set_head(head);
  _cur_floppy->set_cylinder(cylinder);
  _cur_floppy->set_sector(sector);

  if (_cur_floppy->seek())
    set_input_gate(0x40, 0x40);
  else
    set_input_gate(0x40, 0x00);

  return true;
}

void
FDC::set_input_gate(byte_t mask, byte_t val)
{
  _INPUT_GATE = ((_INPUT_GATE & ~mask) | (val & mask));
  DBG(2, form("KCemu/FDC/input_gate",
              "FDC::set_input_gate(): INPUT_GATE: %02x\n",
              _INPUT_GATE));
}

void
FDC::set_msr(byte_t mask, byte_t val)
{
  _MSR = ((_MSR & ~mask) | (val & mask));
  DBG(2, form("KCemu/FDC/msr",
              "FDC::set_msr(): MSR: %02x\n",
              _MSR));
}

/*
 *  handle the terminal count signal (TC)
 *
 *  when receiving high on this pin the floppy controller
 *  aborts the running command (mostly read/write) and
 *  goes into result phase
 */
void
FDC::set_terminal_count(bool val)
{
  if (_terminal_count == val)
    return;

  _terminal_count = val;
  if (!_terminal_count)
    return;

  if (_cur_cmd)
    _cur_cmd->finish_cmd();
  else
    set_state(FDC_STATE_IDLE);
}

byte_t
FDC::get_ST0(void)
{
  set_ST0(ST_0_HEAD_ADDRESS, get_head() == 1 ? ST_0_HEAD_ADDRESS : 0);
  set_ST0(ST_0_UNIT_SELECT_MASK, _selected_unit);
  
  return _ST0;
} 

byte_t
FDC::get_ST1(void)
{
  return _ST1;
}

byte_t
FDC::get_ST2(void)
{
  return _ST2;
}

byte_t
FDC::get_ST3(void)
{
  set_ST3(ST_3_READY, ST_3_READY);
  set_ST3(ST_3_TWO_SIDE, ST_3_TWO_SIDE);
  set_ST3(ST_3_WRITE_PROTECTED, 0);
  if (get_kc_type() == KC_TYPE_A5105)
    set_ST3(ST_3_WRITE_PROTECTED, ST_3_WRITE_PROTECTED);
  set_ST3(ST_3_TRACK_0, get_cylinder() == 0 ? ST_3_TRACK_0 : 0);
  set_ST3(ST_3_HEAD_ADDRESS, get_head() == 1 ? ST_3_HEAD_ADDRESS : 0);
  set_ST3(ST_3_UNIT_SELECT_MASK, _selected_unit);

  return _ST3;
}

void
FDC::set_ST0(byte_t mask, byte_t val)
{
  _ST0 = (~mask & _ST0) | (mask & val);
}

void
FDC::set_ST1(byte_t mask, byte_t val)
{
  _ST1 = (~mask & _ST1) | (mask & val);
}

void
FDC::set_ST2(byte_t mask, byte_t val)
{
  _ST2 = (~mask & _ST2) | (mask & val);
}

void
FDC::set_ST3(byte_t mask, byte_t val)
{
  _ST3 = (~mask & _ST3) | (mask & val);
}

void
FDC::reset(bool power_on)
{
}

void
FDC::reti(void)
{
}
