/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-1998 Torsten Paul
 *
 *  $Id: fdc.cc,v 1.2 2001/01/05 18:10:02 tp Exp $
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

#include "kc/config.h"
#include "kc/system.h"

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

SectorDesc::SectorDesc(long size, byte_t *buf)
{
  _buf = buf;
  _size = size;
}

FDC::FDC(void)
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
  _INPUT_GATE = 0x60;
  _fstate[0] = new FloppyState(0, 0, 1, new Floppy());
  _fstate[1] = new FloppyState(0, 0, 1, 0);
  _fstate[2] = new FloppyState(0, 0, 1, 0);
  _fstate[3] = new FloppyState(0, 0, 1, 0);

  set_state(FDC_STATE_COMMAND);
}

FDC::~FDC(void)
{
}

byte_t
FDC::in(word_t addr)
{
  byte_t val = 0;

  switch (addr & 0xff)
    {
    case 0xf0:
      return _MSR;
    case 0xf1:
      return in_F1(addr);
    case 0xf2:
      /* DAK-FDC (DMA-Acknowledge) */
      val = read_byte();
      break;
    case 0xf4:
      /* 
       *  Input-Gate:
       *
       *  Bit 7   DRQ (DMA-Request)
       *  Bit 6   INT (Interrupt)
       *  Bit 5   RDY (Drive-Ready)
       *  Bit 4   IDX (Index - Spuranfang)
       */
      val = _INPUT_GATE;
      break;
    case 0xf6:
      /* Select-Latch */
      val = 0x00;
      break;
    case 0xf8:
      /* TC-FDC (Terminalcount) - End of DMA Transfer */
      val = 0x00;
      break;
    case 0xfc:
    case 0xfd:
    case 0xfe:
    case 0xff:
      /* CTC */
      val = 0x00;
      break;
    default:
      break;
    }

  DBG(2, form("KCemu/FDC/in",
              "FDC::in():  addr = %04x, val = %02x\n",
              addr, val));

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
    case 0xf1:
      DBG(2, form("KCemu/FDC/out_F1",
                  "FDC::out(): addr = %04x, val = %02x [%c]\n",
                  addr, val, isprint(val) ? val : '.'));
      out_F1(addr, val);
//      if (val == 0x45)
//        Z80_Trace = 1;
      break;
    case 0xf2:
      DBG(2, form("KCemu/FDC/out_F2",
                  "FDC::out(): addr = %04x, val = %02x [%c]\n",
                  addr, val, isprint(val) ? val : '.'));
      write_byte(val);
      break;
    case 0xf8:
      DBG(2, form("KCemu/FDC/out_F8",
                  "FDC::out() - TC %02x\n",
                  val));
      set_input_gate(0x40, 0x00);
      break;
    default:
      DBG(2, form("KCemu/FDC/out_unhandled",
                  "FDC::out(): addr = %04x, val = %02x [%c]\n",
                  addr, val, isprint(val) ? val : '.'));
      break;
    }
}
 
byte_t
FDC::in_F1(word_t addr)
{
  byte_t val;
  
  val = 0xff;
  if (_state == FDC_STATE_RESULT)
    val = _cur_cmd->read_result();

  return val;
}

/*
 *  data input from disk
 */
byte_t
FDC::read_byte(void)
{
  return _cur_cmd->read_byte();
}

/*
 *  data output to disk ???
 */
void
FDC::write_byte(byte_t val)
{
  _cur_cmd->write_byte(val);
}

/*
 *  command output to floppy controller
 */
void
FDC::out_F1(word_t addr, byte_t val)
{
  if (_state == FDC_STATE_COMMAND)
    if (_cur_cmd == 0)
      {
        _cur_cmd = _cmds[val & 0x1f];
        _cur_cmd->start(val);
      }
    else
      _cur_cmd->write_arg(val);
}

void
FDC::select_floppy(int floppy_nr)
{
  DBG(2, form("KCemu/FDC/select_floppy",
              "FDC::select_floppy(): selecting floppy %d\n",
              floppy_nr));  
  _cur_floppy = _fstate[floppy_nr];
}

Floppy *
FDC::get_floppy(void)
{
  if (_cur_floppy == 0)
    return 0;
  return _cur_floppy->get_floppy();
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

  _MSR = (_MSR & ~(ST_MAIN_DIO | ST_MAIN_RQM));
  switch (_state)
    {
    case FDC_STATE_COMMAND:
      _cur_cmd = 0;
      _MSR |= ST_MAIN_RQM;
      DBG(2, form("KCemu/FDC/state",
                  "FDC::set_state(): FDC_STATE_COMMAND -> MSR: %02x\n",
                  _MSR));
      break;
    case FDC_STATE_EXECUTE:
      _MSR |= ST_MAIN_DIO;
      DBG(2, form("KCemu/FDC/state",
                  "FDC::set_state(): FDC_STATE_EXECUTE -> MSR: %02x\n",
                  _MSR));
      break;
    case FDC_STATE_RESULT:
      _MSR |= ST_MAIN_RQM;
      _MSR |= ST_MAIN_DIO;
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
    set_input_gate(0x40, 0x00);
  else
    set_input_gate(0x40, 0x40);

  return true;
}

void
FDC::set_input_gate(byte_t mask, byte_t val)
{
  _INPUT_GATE = ((_INPUT_GATE & ~mask) | (val & mask));
  DBG(2, form("KCemu/FDC/input-gate",
              "FDC::set_input_gate(): INPUT_GATE: %02x\n",
              _INPUT_GATE));
}

void
FDC::iei(byte_t val)
{
}

void
FDC::reset(bool power_on)
{
}

void
FDC::reti(void)
{
}
