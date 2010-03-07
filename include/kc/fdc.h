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

#ifndef __kc_fdc_h
#define __kc_fdc_h

#include "kc/system.h"

#include "kc/ic.h"
#include "kc/cb.h"
#include "kc/ports.h"
#include "kc/fdc_cmd.h"
#include "kc/floppy.h"

class SectorDesc
{
 private:
  long _size;
  byte_t *_buf;

 public:
  SectorDesc(long size, byte_t *buf);
  long get_size(void) { return _size; }
  byte_t get_byte(int idx) { return _buf[idx]; }
};

class FloppyState
{
 private:
  byte_t _head;
  byte_t _cylinder;
  byte_t _sector;
  Floppy *_floppy;
  SectorDesc *_sect;
  
 public:
  FloppyState(byte_t head, byte_t cylinder, byte_t sector, Floppy *floppy);
  virtual ~FloppyState(void);

  byte_t get_head(void) { return _head; }
  byte_t get_cylinder(void) { return _cylinder; }
  byte_t get_sector(void) { return _sector; }
  virtual Floppy * get_floppy(void) { return _floppy; }
  void set_head(byte_t head) { _head = head; }
  void set_cylinder(byte_t cylinder) { _cylinder = cylinder; }
  void set_sector(byte_t sector) { _sector = sector; }
  bool seek(void) {
    if (_floppy == 0)
      return false;

    return _floppy->seek(_head, _cylinder, _sector);
  }
};

class FDC : public InterfaceCircuit, public PortInterface, public Callback
{
 public:
  typedef enum {
    FDC_STATE_IDLE,
    FDC_STATE_COMMAND,
    FDC_STATE_EXECUTE,
    FDC_STATE_DATA,
    FDC_STATE_RESULT
  } fdc_state_t;

  enum {
    NR_OF_FLOPPIES = 4,
  
    ST_MAIN_BUSY_MASK = 0x0f,
    ST_MAIN_FDD0_BUSY = 0x01, /* FDD number 0 is in the seek mode */
    ST_MAIN_FDD1_BUSY = 0x02, /* FDD number 1 is in the seek mode */
    ST_MAIN_FDD2_BUSY = 0x04, /* FDD number 2 is in the seek mode */
    ST_MAIN_FDD3_BUSY = 0x08, /* FDD number 3 is in the seek mode */
    ST_MAIN_READ_WRITE = 0x10, /* read or write command in progress */
    ST_MAIN_NON_DMA   = 0x20, /* FDC is in NON-DMA mode */
    ST_MAIN_DIO       = 0x40, /* direction of data transfer */
    ST_MAIN_RQM       = 0x80, /* Data Register ready */
  
    /*
     *  STATUS REGISTER 0
     */
    ST_0_ALL_MASK                = 0xff,
    ST_0_IC_MASK                 = 0xc0,   /* Interrupt Code */
    ST_0_IC_NORMAL_TERMINATION     = 0x00, /* Normal Termination of Command */
    ST_0_IC_ABNORMAL_TERMINATION   = 0x40, /* Abnormal Termination of Command */
    ST_0_IC_INVALID_COMMAND        = 0x80, /* Invalid Command issued */
    ST_0_IC_FDD_NOT_READY          = 0xc0, /* Abnormal Termination, not ready */
    ST_0_SEEK_END                  = 0x20, /* Seek End */
    ST_0_EC                        = 0x10, /* Equipment Check */
    ST_0_NR                        = 0x08, /* Not Ready */
    ST_0_HEAD_ADDRESS              = 0x04, /* Head Address */
    ST_0_UNIT_SELECT_MASK        = 0x03,
    ST_0_UNIT_SELECT_1             = 0x02,
    ST_0_UNIT_SELECT_0             = 0x01,

    /*
     *  STATUS REGISTER 1
     */
    ST_1_ALL_MASK                = 0xff,
    ST_1_END_OF_CYLINDER           = 0x80,
    ST_1_UNUSED1                   = 0x40,
    ST_1_DATA_ERROR                = 0x20,
    ST_1_OVER_RUN                  = 0x10,
    ST_1_UNUSED2                   = 0x08,
    ST_1_NO_DATE                   = 0x04,
    ST_1_NOT_WRITEABLE             = 0x02,
    ST_1_MISSING_ADDRESS_MARK      = 0x01,

    /*
     *  STATUS REGISTER 2
     */
    ST_2_UNUSED                    = 0x80,
    ST_2_CONTROL_MARK              = 0x40,
    ST_2_DATA_ERROR_IN_DATA        = 0x20,
    ST_2_WRONG_CYLINDER            = 0x10,
    ST_2_SCAN_MASK               = 0x0c,
    ST_2_SCAN_EQUAL_HIT            = 0x08,
    ST_2_SCAN_NOT_SATISFIED        = 0x04,
    ST_2_BAD_CYLINDER              = 0x02,
    ST_2_MISSING_ADDRESS_MARK      = 0x01,

    /*
     *  STATUS REGISTER 3
     */
    ST_3_ALL_MASK                = 0xff,
    ST_3_FAULT                     = 0x80,
    ST_3_WRITE_PROTECTED           = 0x40,
    ST_3_READY                     = 0x20,
    ST_3_TRACK_0                   = 0x10,
    ST_3_TWO_SIDE                  = 0x08,
    ST_3_HEAD_ADDRESS              = 0x04, /* side select signal */
    ST_3_UNIT_SELECT_MASK        = 0x03,
    ST_3_UNIT_SELECT_1             = 0x02,
    ST_3_UNIT_SELECT_0             = 0x01,

    /*
     *  Callback selector
     */
    CB_MASK                      = 0xff0000,
    CB_TYPE_SEEK                   = 0x010000,
    CB_TYPE_INDEX                  = 0x020000,
  };

 private:
  fdc_state_t _state;
  FDC_CMD *_cmds[32];
  FDC_CMD *_cur_cmd;
  Floppy *_floppy[NR_OF_FLOPPIES];
  FloppyState *_fstate[NR_OF_FLOPPIES];
  FloppyState *_cur_floppy;
  int _selected_unit;
  int _selected_device; // select line of the floppy drive, not in the FDC
  
  byte_t _MSR; /* Main Status Register */
  byte_t _INPUT_GATE;
  byte_t _ST0;
  byte_t _ST1;
  byte_t _ST2;
  byte_t _ST3;
  byte_t _head;
  byte_t _cylinder;
  byte_t _sector;

  int _read_bytes;
  bool _terminal_count;

  void init(void);
  
 public:
  FDC(void);
  virtual ~FDC(void);

  void callback(void *data);
  void callback_seek(void *data);
  void callback_index(void *data);

  void drive_select(byte_t val);

  virtual byte_t in_data(word_t addr);
  virtual void out_data(word_t addr, byte_t val);

  virtual byte_t read_byte(void);
  virtual void write_byte(byte_t val);

  Floppy * get_floppy(void);
  Floppy * get_floppy(int idx);
  int get_head(void);
  int get_cylinder(void);
  int get_sector(void);
  void select_floppy(int floppy_nr);
  void set_state(fdc_state_t state);
  bool seek(byte_t head, byte_t cylinder, byte_t sector);
  bool seek_internal(byte_t head, byte_t cylinder, byte_t sector);
  byte_t get_input_gate(void);
  void set_input_gate(byte_t mask, byte_t val);
  byte_t get_msr(void);
  void set_msr(byte_t mask, byte_t val);
  void set_terminal_count(bool val);

  byte_t get_ST0(void);
  byte_t get_ST1(void);
  byte_t get_ST2(void);
  byte_t get_ST3(void);

  void set_ST0(byte_t mask, byte_t val);
  void set_ST1(byte_t mask, byte_t val);
  void set_ST2(byte_t mask, byte_t val);
  void set_ST3(byte_t mask, byte_t val);

  virtual long long get_counter() = 0;
  virtual void add_callback(unsigned long long offset, Callback *cb, void *data) = 0;

  /*
   *  PortInterface
   */
  virtual byte_t in(word_t addr) = 0;
  virtual void out(word_t addr, byte_t val) = 0;

  /*
   *  InterfaceCircuit
   */
  virtual void reti(void);
  virtual void irqreq(void) {}
  virtual word_t irqack() { return IRQ_NOT_ACK; }
  virtual void reset(bool power_on = false);
};

#endif /* __kc_fdc_h */
