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

#include "kc/kc.h"
#include "kc/fdc.h"
#include "kc/fdc_cmd.h"

#include "libdbg/dbg.h"

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

  _floppy[0] = new Floppy("attach-1");
  _floppy[1] = new Floppy("attach-2");
  _floppy[2] = new Floppy("attach-3");
  _floppy[3] = new Floppy("attach-4");

  _fstate[0] = new FloppyState(0, 0, 1, _floppy[0]);
  _fstate[1] = new FloppyState(0, 0, 1, _floppy[1]);
  _fstate[2] = new FloppyState(0, 0, 1, _floppy[2]);
  _fstate[3] = new FloppyState(0, 0, 1, _floppy[3]);

  init();
}

FDC::~FDC(void)
{
  for (int a = 0;a <= 0x1f;a++)
    delete _cmds[a];

  for (int a = 0;a < 4;a++)
    {
      delete _fstate[a];
      delete _floppy[a];
    }
}

void
FDC::init(void)
{
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
  _selected_device = 0;
  _terminal_count = false;

  set_state(FDC_STATE_IDLE);
}

byte_t
FDC::in_data(word_t addr)
{
  byte_t val;

  val = 0xff;
  switch (_state)
    {
    case FDC_STATE_RESULT:
      val = _cur_cmd->read_result();
      DBG(2, form("KCemu/FDC/in_data",
		  "FDC::in(): addr = %04x, val = %02x [%c] FDC_STATE_RESULT\n",
		  addr, val, isprint(val) ? val : '.'));
      break;
    case FDC_STATE_DATA:
      val = read_byte();
      DBG(2, form("KCemu/FDC/in_data",
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
FDC::out_data(word_t addr, byte_t val)
{
  switch (_state)
    {
    case FDC_STATE_IDLE:
      DBG(2, form("KCemu/FDC/out_data",
		  "FDC::out(): addr = %04x, val = %02x [%c] FDC_STATE_IDLE\n",
		  addr, val, isprint(val) ? val : '.'));
      _cur_cmd = _cmds[val & 0x1f];
      _cur_cmd->start(val);
      break;
    case FDC_STATE_COMMAND:
      DBG(2, form("KCemu/FDC/out_data",
		  "FDC::out(): addr = %04x, val = %02x [%c] FDC_STATE_COMMAND\n",
		  addr, val, isprint(val) ? val : '.'));
      _cur_cmd->write_arg(val);
      break;
    case FDC_STATE_DATA:
      DBG(2, form("KCemu/FDC/out_data",
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

  byte_t msr = get_msr() & 0x0f;
  byte_t dio = get_msr() & ST_MAIN_DIO;
  switch (_state)
    {
    case FDC_STATE_IDLE:
      msr |= ST_MAIN_RQM;
      DBG(2, form("KCemu/FDC/state",
		  "FDC::set_state(): FDC_STATE_IDLE    -> MSR: %02x\n",
		  msr));
      break;
    case FDC_STATE_COMMAND:
      msr |= ST_MAIN_READ_WRITE;
      msr |= ST_MAIN_RQM;
      DBG(2, form("KCemu/FDC/state",
		  "FDC::set_state(): FDC_STATE_COMMAND -> MSR: %02x\n",
		  msr));
      break;
    case FDC_STATE_EXECUTE:
      msr |= ST_MAIN_NON_DMA;
      msr |= ST_MAIN_DIO;
      DBG(2, form("KCemu/FDC/state",
		  "FDC::set_state(): FDC_STATE_EXECUTE -> MSR: %02x\n",
		  msr));
      break;
    case FDC_STATE_DATA:
      msr |= ST_MAIN_READ_WRITE;
      msr |= ST_MAIN_NON_DMA;
      msr |= ST_MAIN_RQM;
      // don't change the direction register, this is set by the command
      // (1 while doing read, 0 while doing write command data transfer)
      msr |= dio;
      DBG(2, form("KCemu/FDC/state",
		  "FDC::set_state(): FDC_STATE_DATA    -> MSR: %02x\n",
		  msr));
      break;
    case FDC_STATE_RESULT:
      msr |= ST_MAIN_READ_WRITE;
      msr |= ST_MAIN_DIO;
      msr |= ST_MAIN_RQM;
      DBG(2, form("KCemu/FDC/state",
		  "FDC::set_state(): FDC_STATE_RESULT  -> MSR: %02x\n",
		  msr));
      break;
    }

  set_msr(0xf0, msr);
}

void
FDC::callback(void *data)
{
  long val = (long)data;
  switch (val & CB_MASK)
    {
    case CB_TYPE_SEEK:
      callback_seek(data);
      break;
    case CB_TYPE_INDEX:
      callback_index(data);
      break;
    default:
      DBG(0, form("KCemu/internal_error",
		  "FDC::callback(): unknown callback %08x\n",
		  val));
      break;
    }
}

/**
 *  Emulation of the index pulse generated by the floppy drive.
 *  We assume the drive rotates with 300rpm, means 5 rotations
 *  per second.
 */
void
FDC::callback_index(void *data)
{
  long index_value = ((long)data) & 1;

  byte_t input_gate = 0;
  if (_selected_device != 0)
    {
      int offset;
      if (index_value == 0)
        {
          offset = 1000;
          input_gate = 0x10;
        }
      else
        {
          offset = 350000;
        }

      add_callback(offset, this, (void *)(CB_TYPE_INDEX | (1 - index_value)));
    }

  set_input_gate(0x10, input_gate);
}

void
FDC::callback_seek(void *data)
{
  long unit = (long)data;
  byte_t unit_bit = 1 << (unit & 0xff);
  bool ok = (unit & 0x0100) == 0;

  DBG(2, form("KCemu/FDC/seek",
	      "FDC::seek(): [%8Ld] finished seek %s, floppy %d\n",
	      get_counter(),
	      ok ? "ok" : "with error",
	      unit & 0xff));

  set_msr(unit_bit, 0);

  if (ok)
    {
      // seek ok
      set_ST0(ST_0_IC_MASK | ST_0_SEEK_END | ST_0_EC,
	      ST_0_IC_NORMAL_TERMINATION | ST_0_SEEK_END);
      set_ST3(ST_3_READY | ST_3_TRACK_0, ST_3_READY | ST_3_TRACK_0);
    }
  else
    {
      // seek failed
      set_ST0(ST_0_IC_MASK | ST_0_SEEK_END | ST_0_EC,
	      ST_0_IC_ABNORMAL_TERMINATION | ST_0_SEEK_END | ST_0_EC);
      set_ST3(ST_3_READY | ST_3_TRACK_0, 0);
    }
  
  set_input_gate(0x40, 0x00);
}

bool
FDC::seek_internal(byte_t head, byte_t cylinder, byte_t sector)
{
  if (_cur_floppy == 0)
    return false;

  _cur_floppy->set_head(head);
  _cur_floppy->set_cylinder(cylinder);
  _cur_floppy->set_sector(sector);

  bool seek_ok = _cur_floppy->seek();

  return seek_ok;
}

bool
FDC::seek(byte_t head, byte_t cylinder, byte_t sector)
{
  if (_cur_floppy == 0)
    return false;

  int c1 = cylinder;
  int c2 = _cur_floppy->get_cylinder();
  int diff = abs(c1 - c2);

  bool seek_ok = seek_internal(head, cylinder, sector);
  set_ST0(ST_0_SEEK_END, 0);
  set_input_gate(0x40, 0x40);

  int offset = diff * 1000 + 500;

  if (seek_ok)
    {
      DBG(2, form("KCemu/FDC/seek",
		  "FDC::seek(): [%8Ld] starting seek to cylinder %d, diff = %d, floppy %d\n",
		  get_counter(),
		  cylinder,
		  diff,
		  _selected_unit));
      add_callback(offset, this, (void *)(CB_TYPE_SEEK | _selected_unit));
    }
  else
    {
      DBG(2, form("KCemu/FDC/seek",
		  "FDC::seek(): [%8Ld] seek failed to cylinder %d, diff = %d, floppy %d\n",
		  get_counter(),
		  cylinder,
		  diff,
		  _selected_unit));
      add_callback(offset, this, (void *)(CB_TYPE_SEEK | _selected_unit | 0x0100));
    }

  byte_t unit_bit = 1 << _selected_unit;
  set_msr(unit_bit, unit_bit);

  return seek_ok;
}

void
FDC::drive_select(byte_t val)
{
  DBG(2, form("KCemu/FDC/drive_select",
	      "FDC::drive_select(): output to drive select port, value = %02x\n",
	      val));

  val &= 0x0f;

  bool has_disc = false;
  for (int a = 0;a < 4;a++)
    {
      Floppy *floppy = get_floppy(a);
      if ((val & (1 << a)) && (floppy != NULL) && (floppy->get_sector_size() > 0))
        has_disc = true;
    }
  
  if (has_disc)
    {
      DBG(2, form("KCemu/FDC/drive_select",
                  "FDC::drive_select(): enable index-hole pulse generation%s\n",
                  _selected_device == 0 ? "" : " (already running)"));

      if (_selected_device == 0)
        add_callback(20000, this, (void *)(CB_TYPE_INDEX));

      _selected_device = val;
    }
  else
    {
      DBG(2, form("KCemu/FDC/drive_select",
                  "FDC::drive_select(): stop index-hole pulse generation\n"));

      _selected_device = 0;
    }

  set_input_gate(0x10, 0x00);
}

byte_t
FDC::get_input_gate(void)
{
  return _INPUT_GATE;
}

void
FDC::set_input_gate(byte_t mask, byte_t val)
{
  _INPUT_GATE = ((_INPUT_GATE & ~mask) | (val & mask));
  DBG(2, form("KCemu/FDC/input_gate",
	      "FDC::set_input_gate(): INPUT_GATE: %02x\n",
	      _INPUT_GATE));
}

byte_t
FDC::get_msr(void)
{
  return _MSR;
}

void
FDC::set_msr(byte_t mask, byte_t val)
{
  _MSR = ((_MSR & ~mask) | (val & mask));
  DBG(2, form("KCemu/FDC/MSR",
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
  set_ST3(ST_3_TRACK_0, get_cylinder() == 0 ? ST_3_TRACK_0 : 0);
  set_ST3(ST_3_HEAD_ADDRESS, get_head() == 1 ? ST_3_HEAD_ADDRESS : 0);
  set_ST3(ST_3_UNIT_SELECT_MASK, _selected_unit);

  return _ST3;
}

void
FDC::set_ST0(byte_t mask, byte_t val)
{
  _ST0 = (~mask & _ST0) | (mask & val);
  DBG(2, form("KCemu/FDC/ST0",
	      "FDC::set_ST0(): ST0: %02x\n",
	      _ST0));
}

void
FDC::set_ST1(byte_t mask, byte_t val)
{
  _ST1 = (~mask & _ST1) | (mask & val);
  DBG(2, form("KCemu/FDC/ST1",
	      "FDC::set_ST1(): ST1: %02x\n",
	      _ST1));
}

void
FDC::set_ST2(byte_t mask, byte_t val)
{
  _ST2 = (~mask & _ST2) | (mask & val);
  DBG(2, form("KCemu/FDC/ST2",
	      "FDC::set_ST2(): ST2: %02x\n",
	      _ST2));
}

void
FDC::set_ST3(byte_t mask, byte_t val)
{
  _ST3 = (~mask & _ST3) | (mask & val);
  DBG(2, form("KCemu/FDC/ST3",
	      "FDC::set_ST3(): ST3: %02x\n",
	      _ST3));
}

void
FDC::reset(bool power_on)
{
  init();
}

void
FDC::reti(void)
{
}
