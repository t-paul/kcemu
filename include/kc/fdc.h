/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-1998 Torsten Paul
 *
 *  $Id: fdc.h,v 1.2 2000/06/24 03:39:19 tp Exp $
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

#ifndef __kc_fdc_h
#define __kc_fdc_h

#include "kc/config.h"
#include "kc/system.h"

#include "kc/ic.h"
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

class FDC : public InterfaceCircuit, public PortInterface
{
 public:
  typedef enum {
    FDC_STATE_COMMAND,
    FDC_STATE_EXECUTE,
    FDC_STATE_RESULT
  } fdc_state_t;

  enum {
    NR_OF_FLOPPIES = 4,
  
    ST_MAIN_FDD0_BUSY = 0x01, /* FDD number 0 is in the seek mode */
    ST_MAIN_FDD1_BUSY = 0x02, /* FDD number 1 is in the seek mode */
    ST_MAIN_FDD2_BUSY = 0x04, /* FDD number 2 is in the seek mode */
    ST_MAIN_FDD3_BUSY = 0x08, /* FDD number 3 is in the seek mode */
    ST_MAIN_FDD4_BUSY = 0x10, /* read or write command in progress */
    ST_MAIN_NON_DMA   = 0x20, /* FDC is in NON-DMA mode */
    ST_MAIN_DIO       = 0x40, /* direction of data transfer */
    ST_MAIN_RQM       = 0x80, /* Data Register ready */
  
    ST_0_IC_MASK = 0xc0, /* Interrupt Code */
    ST_0_IC_NT     = 0x00, /* Normal Termination of Command */
    ST_0_IC_AT     = 0x40, /* Abnormal Tremination of Command */
    ST_0_IC_IC     = 0x80, /* Invalid Command issued */
    ST_0_IC_FDD    = 0xc0, /* Abnormal Termination, not ready */
    ST_0_SE        = 0x20, /* Seek End */
    ST_0_EC        = 0x10, /* Equipment Check */
    ST_0_NR        = 0x08, /* Not Ready */
    ST_0_HD        = 0x04, /* Head Address */
    ST_0_US_MASK = 0x03, /* Unit Select */
    ST_0_US0       = 0x00, /* Unit Select 0 */
    ST_0_US1       = 0x01, /* Unit Select 1 */
    ST_0_US2       = 0x02, /* Unit Select 2 */
    ST_0_US3       = 0x03, /* Unit Select 3 */
  };

 private:
  fdc_state_t _state;
  FDC_CMD *_cmds[32];
  FDC_CMD *_cur_cmd;
  FloppyState *_fstate[NR_OF_FLOPPIES];
  FloppyState *_cur_floppy;
  
  byte_t _MSR; /* Main Status Register */
  byte_t _INPUT_GATE;
  byte_t _head;
  byte_t _cylinder;
  byte_t _sector;

  int _read_bytes;
  
 public:
  FDC(void);
  virtual ~FDC(void);

  virtual void iei(byte_t val);
  virtual void reset(bool power_on = false);
  virtual void reti(void);

  virtual byte_t in(word_t addr);
  virtual void out(word_t addr, byte_t val);

  virtual byte_t in_F1(word_t addr);
  virtual void out_F1(word_t addr, byte_t val);

  virtual byte_t read_byte(void);
  virtual void write_byte(byte_t val);

  Floppy * get_floppy(void);
  int get_head(void);
  int get_cylinder(void);
  int get_sector(void);
  void select_floppy(int floppy_nr);
  void set_state(fdc_state_t state);
  bool seek(byte_t head, byte_t cylinder, byte_t sector);
  void set_input_gate(byte_t mask, byte_t val);
};

#endif /* __kc_fdc_h */
