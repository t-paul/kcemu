/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: fdc_cmd.h,v 1.4 2002/01/02 00:35:05 torsten_paul Exp $
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

#ifndef __kc_fdc_cmd_h
#define __kc_fdc_cmd_h

#include "kc/config.h"
#include "kc/system.h"

class FDC;
class SectorDesc;

class FDC_CMD
{
 private:
  enum {
    MAX_ARGS = 10,
    MAX_RESULTS = 10,
  };
  
  FDC *_fdc;
  int _args;
  int _results;
  int _r_idx;
  int _w_idx;
  char *_name;
  SectorDesc *_sect;
  
 protected:
  byte_t _arg[MAX_ARGS];
  byte_t _result[MAX_RESULTS];
  
  virtual void execute(void) = 0;
  virtual void execute_cmd(void);

 public:
  FDC_CMD(FDC *fdc, int args, int results, const char *name);
  virtual ~FDC_CMD(void);

  virtual const char * get_name(void);
  virtual FDC * get_fdc(void);

  virtual void start(byte_t val);
  virtual bool write_arg(byte_t val);
  virtual byte_t read_result(void);

  virtual byte_t read_byte(void);
  virtual void write_byte(byte_t val);
};

class FDC_CMD_INVALID : public FDC_CMD
{
 protected:
  virtual void execute(void);
  
 public:
  FDC_CMD_INVALID(FDC *fdc);
  virtual ~FDC_CMD_INVALID(void);
};

/*
 *  0x02
 */
class FDC_CMD_READ_TRACK : public FDC_CMD
{
 protected:
  virtual void execute(void);
  
 public:
  FDC_CMD_READ_TRACK(FDC *fdc);
  virtual ~FDC_CMD_READ_TRACK(void);
};

/*
 *  0x03
 */
class FDC_CMD_SPECIFY : public FDC_CMD
{
 protected:
  virtual void execute(void);
  
 public:
  FDC_CMD_SPECIFY(FDC *fdc);
  virtual ~FDC_CMD_SPECIFY(void);
};

/*
 *  0x04
 */
class FDC_CMD_SENSE_DRIVE_STATUS : public FDC_CMD
{
 protected:
  virtual void execute(void);
  
 public:
  FDC_CMD_SENSE_DRIVE_STATUS(FDC *fdc);
  virtual ~FDC_CMD_SENSE_DRIVE_STATUS(void);
};

/*
 *  0x05
 */
class FDC_CMD_WRITE_DATA : public FDC_CMD
{
 private:
  int _idx;
  int _sector_size;
  int _head, _cylinder, _sector;
  byte_t *_buf;
  
 protected:
  virtual void execute(void);
  
 public:
  FDC_CMD_WRITE_DATA(FDC *fdc);
  virtual ~FDC_CMD_WRITE_DATA(void);

  virtual void write_byte(byte_t val);
};

/*
 *  0x06
 */
class FDC_CMD_READ_DATA : public FDC_CMD
{
 private:
  int _idx;
  int _size;
  byte_t *_buf;
  
 protected:
  virtual void execute(void);
  
 public:
  FDC_CMD_READ_DATA(FDC *fdc);
  virtual ~FDC_CMD_READ_DATA(void);

  virtual bool fetch_next_sector(void);
  virtual byte_t read_byte(void);
};

/*
 *  0x07
 */
class FDC_CMD_RECALIBRATE : public FDC_CMD
{
 protected:
  virtual void execute(void);
  
 public:
  FDC_CMD_RECALIBRATE(FDC *fdc);
  virtual ~FDC_CMD_RECALIBRATE(void);
};

/*
 *  0x08
 */
class FDC_CMD_SENSE_INTERRUPT_STATUS : public FDC_CMD
{
 protected:
  virtual void execute(void);
  
 public:
  FDC_CMD_SENSE_INTERRUPT_STATUS(FDC *fdc);
  virtual ~FDC_CMD_SENSE_INTERRUPT_STATUS(void);
};

/*
 *  0x09
 */
class FDC_CMD_WRITE_DELETED_DATA : public FDC_CMD
{
 protected:
  virtual void execute(void);
  
 public:
  FDC_CMD_WRITE_DELETED_DATA(FDC *fdc);
  virtual ~FDC_CMD_WRITE_DELETED_DATA(void);
};

/*
 *  0x0a
 */
class FDC_CMD_READ_ID : public FDC_CMD
{
 protected:
  virtual void execute(void);
  
 public:
  FDC_CMD_READ_ID(FDC *fdc);
  virtual ~FDC_CMD_READ_ID(void);
};

/*
 *  0x0c
 */
class FDC_CMD_READ_DELETED_DATA : public FDC_CMD
{
 protected:
  virtual void execute(void);
  
 public:
  FDC_CMD_READ_DELETED_DATA(FDC *fdc);
  virtual ~FDC_CMD_READ_DELETED_DATA(void);
};

/*
 *  0x0d
 */
class FDC_CMD_FORMAT_A_TRACK : public FDC_CMD
{
 private:
  int _idx;
  int _head;
  int _cylinder;
  int _sector;
  int _sector_size;
  int _bytes_per_sector;
  int _sectors_per_track;
  int _cur_sector;
  byte_t *_buf;
  
 protected:
  virtual void execute(void);
  
 public:
  FDC_CMD_FORMAT_A_TRACK(FDC *fdc);
  virtual ~FDC_CMD_FORMAT_A_TRACK(void);

  virtual void write_byte(byte_t val);
  virtual void format(void);
};

/*
 *  0x0f
 */
class FDC_CMD_SEEK : public FDC_CMD
{
 protected:
  virtual void execute(void);
  
 public:
  FDC_CMD_SEEK(FDC *fdc);
  virtual ~FDC_CMD_SEEK(void);
};

/*
 *  0x11
 */
class FDC_CMD_SCAN_EQUAL : public FDC_CMD
{
 protected:
  virtual void execute(void);
  
 public:
  FDC_CMD_SCAN_EQUAL(FDC *fdc);
  virtual ~FDC_CMD_SCAN_EQUAL(void);
};

/*
 *  0x19
 */
class FDC_CMD_SCAN_LOW_OR_EQUAL : public FDC_CMD
{
 protected:
  virtual void execute(void);
  
 public:
  FDC_CMD_SCAN_LOW_OR_EQUAL(FDC *fdc);
  virtual ~FDC_CMD_SCAN_LOW_OR_EQUAL(void);
};

/*
 *  0x1d
 */
class FDC_CMD_SCAN_HIGH_OR_EQUAL : public FDC_CMD
{
 protected:
  virtual void execute(void);
  
 public:
  FDC_CMD_SCAN_HIGH_OR_EQUAL(FDC *fdc);
  virtual ~FDC_CMD_SCAN_HIGH_OR_EQUAL(void);
};

#endif /* __kc_fdc_h */
