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
#include <string.h>
#include <unistd.h>

#include "kc/system.h"

#include "kc/z80.h" // DEBUG

#include "kc/fdc.h"
#include "kc/fdc_cmd.h"
#include "kc/floppy.h"

#include "libdbg/dbg.h"

static struct sector_def
{
  int MFM, N, size, SC, GPL1, GPL2;
} sector_def[] = {
  {  0,  0,  128, 0x1a, 0x07, 0x1b }, /* IBM Diskette 1 */
  {  0,  1,  256, 0x0f, 0x0e, 0x2a }, /* IBM Diskette 2 */
  {  0,  2,  512, 0x08, 0x1b, 0x3a },
  {  0,  3, 1024, 0x04, 0x00, 0x00 },
  {  0,  4, 2048, 0x02, 0x00, 0x00 },
  {  0,  5, 4096, 0x01, 0x00, 0x00 },
  {  1,  1,  256, 0x1a, 0x0e, 0x36 }, /* IBM Diskette 2D */
  {  1,  2,  512, 0x0f, 0x1b, 0x54 }, /* IBM Diskette 2D */
  {  1,  3, 1024, 0x08, 0x36, 0x74 },
  {  1,  4, 2048, 0x04, 0x00, 0x00 },
  {  1,  5, 4096, 0x02, 0x00, 0x00 },
  {  1,  6, 8192, 0x01, 0x00, 0x00 },
  { -1, -1,  -1,   -1,   -1,   -1 },
};

static int
N_to_sector_size(int MFM, int N)
{
  int a;

  a = 0;
  while (sector_def[a].N != -1)
    {
      if ((sector_def[a].MFM == MFM) && (sector_def[a].N == N))
        return sector_def[a].size;
      a++;
    }

  return -1;
}

FDC_CMD::FDC_CMD(FDC *fdc, int args, int results, const char *name)
{
  _fdc = fdc;
  _args = args;
  _results = results;
  _name = strdup(name);
  _sect = new SectorDesc(0, 0);
}

FDC_CMD::~FDC_CMD(void)
{
  free(_name);
  delete _sect;
}

const char * FDC_CMD::get_name(void)
{
  return _name;
}

FDC *
FDC_CMD::get_fdc(void)
{
  return _fdc;
}

void
FDC_CMD::execute_cmd(void)
{
  DBG(2, form("KCemu/FDC_CMD/command/execute",
              "FDC: --> execute '%s'\n",
              get_name()));
  
  _fdc->set_state(FDC::FDC_STATE_EXECUTE);
  _data_transfer = false;

  execute();
  if (_data_transfer)
    _fdc->set_state(FDC::FDC_STATE_DATA);
  else 
    finish_cmd();
}

void
FDC_CMD::finish_cmd(void)
{
  if (_results > 0)
    _fdc->set_state(FDC::FDC_STATE_RESULT);
  else
    _fdc->set_state(FDC::FDC_STATE_IDLE);
}

void
FDC_CMD::start(byte_t val)
{
  _w_idx = 0;
  _r_idx = 0;
  _arg[_w_idx++] = val;

  _fdc->set_state(FDC::FDC_STATE_COMMAND);

  DBG(2, form("KCemu/FDC_CMD/command/start",
              "FDC: --> start   '%s' [%02x] %d/%d\n",
              get_name(), val, _w_idx, _args));

  if (_args == 1)
    execute_cmd();
}

bool
FDC_CMD::write_arg(byte_t val)
{
  _arg[_w_idx++] = val;

  DBG(2, form("KCemu/FDC_CMD/command/arg",
              "FDC: --> arg     '%s' [%02x] %d/%d\n",
              get_name(), val, _w_idx, _args));

  if (_w_idx == _args)
    execute_cmd();

  return true;
}

byte_t
FDC_CMD::read_result(void)
{
  byte_t val = 0;

  val = _result[_r_idx++];
  DBG(2, form("KCemu/FDC_CMD/command/result",
              "FDC: <-- result  '%s' - [%02x] %d/%d\n",
              get_name(), val, _r_idx, _results));

  if (_r_idx >= _results)
    _fdc->set_state(FDC::FDC_STATE_IDLE);

  return val;
}

byte_t
FDC_CMD::read_byte(void)
{
  DBG(1, form("KCemu/warning",
              "FDC_CMD::read_byte() called! [current cmd is '%s']\n",
              get_name()));

  return 0xff;
}

void
FDC_CMD::write_byte(byte_t val)
{
  DBG(1, form("KCemu/warning",
              "FDC_CMD::write_byte() called! [%s] (value = 0x%02x)\n",
              get_name(), val));
}

int
FDC_CMD::get_read_idx(void)
{
  return _r_idx;
}

int
FDC_CMD::get_write_idx(void)
{
  return _w_idx;
}

/*
 *  INVALID
 */
FDC_CMD_INVALID::FDC_CMD_INVALID(FDC *fdc)
  : FDC_CMD(fdc, 1, 1, "INVALID")
{
}

FDC_CMD_INVALID::~FDC_CMD_INVALID(void)
{
}

void
FDC_CMD_INVALID::execute(void)
{
  DBG(2, form("KCemu/FDC_CMD/INVALID",
              "FDC: INVALID: ------------------------------------\n"
              "FDC: INVALID: code = %02x\n"
              "FDC: INVALID: ------------------------------------\n",
              _arg[0]));

  get_fdc()->set_ST0(FDC::ST_0_ALL_MASK, FDC::ST_0_IC_INVALID_COMMAND);

  _result[0] = get_fdc()->get_ST0();
}

/********************************************************************
 *  0x02 - FDC_CMD_READ_TRACK
 */
FDC_CMD_READ_TRACK::FDC_CMD_READ_TRACK(FDC *fdc)
  : FDC_CMD(fdc, 9, 7, "READ TRACK")
{
}

FDC_CMD_READ_TRACK::~FDC_CMD_READ_TRACK(void)
{
}

void
FDC_CMD_READ_TRACK::execute(void)
{
  DBG(2, form("KCemu/FDC_CMD/READ_TRACK",
              "FDC: READ TRACK: ---------------------------------\n"
              "FDC: READ TRACK: ---------------------------------\n"));
}

/********************************************************************
 *  0x03 - FDC_CMD_SPECIFY
 */
FDC_CMD_SPECIFY::FDC_CMD_SPECIFY(FDC *fdc)
  : FDC_CMD(fdc, 3, 0, "SPECIFY")
{
}

FDC_CMD_SPECIFY::~FDC_CMD_SPECIFY(void)
{
}

void
FDC_CMD_SPECIFY::execute(void)
{
  DBG(2, form("KCemu/FDC_CMD/SPECIFY",
              "FDC: SPECIFY: ------------------------------------\n"
              "FDC: SPECIFY: Step Rate Time   = %d ms\n"
              "FDC: SPECIFY: Head Unload Time = %d ms\n"
              "FDC: SPECIFY: Head Load Time   = %d ms\n"
              "FDC: SPECIFY: NON-DMA Mode     = %s\n"
              "FDC: SPECIFY: ------------------------------------\n",
              16 - ((_arg[1] >> 4) & 0x0f),
              (_arg[1] & 0x0f) * 16 + 16,
              (_arg[2] & 0xfe) + 2,
              (_arg[2] & 0x01) ? "yes" : "no (= DMA Mode)"));
}

/********************************************************************
 *  0x04 - FDC_CMD_SENSE_DRIVE_STATUS
 */
FDC_CMD_SENSE_DRIVE_STATUS::FDC_CMD_SENSE_DRIVE_STATUS(FDC *fdc)
  : FDC_CMD(fdc, 2, 1, "SENSE DRIVE STATUS")
{
}

FDC_CMD_SENSE_DRIVE_STATUS::~FDC_CMD_SENSE_DRIVE_STATUS(void)
{
}

void
FDC_CMD_SENSE_DRIVE_STATUS::execute(void)
{
  DBG(2, form("KCemu/FDC_CMD/SENSE_DRIVE_STATUS",
              "FDC: SENSE DRIVE STATUS: -------------------------\n"
              "FDC: SENSE DRIVE STATUS: Head Select        = %d\n"
              "FDC: SENSE DRIVE STATUS: Drive Select       = %d\n"
              "FDC: SENSE DRIVE STATUS: -------------------------\n",
              (_arg[1] >> 2) & 1,
              _arg[1] & 3));

  get_fdc()->select_floppy(_arg[1] & 3);
  _result[0] = get_fdc()->get_ST3();
}

/********************************************************************
 *  0x05 - FDC_CMD_WRITE_DATA
 */
FDC_CMD_WRITE_DATA::FDC_CMD_WRITE_DATA(FDC *fdc)
  : FDC_CMD(fdc, 9, 7, "WRITE DATA")
{
  _buf = 0;
}

FDC_CMD_WRITE_DATA::~FDC_CMD_WRITE_DATA(void)
{
  if (_buf != 0)
    delete _buf;
}

void
FDC_CMD_WRITE_DATA::execute(void)
{
  DBG(2, form("KCemu/FDC_CMD/WRITE_DATA",
              "FDC: WRITE DATA: ----------------------------------\n"
              "FDC: WRITE DATA: Multi-Track          = %s\n"
              "FDC: WRITE DATA: FM or MFM Mode       = %s\n"
              "FDC: WRITE DATA: Skip                 = %s\n"
              "FDC: WRITE DATA: Head Select          = %d\n"
              "FDC: WRITE DATA: Drive Select         = %d\n"
              "FDC: WRITE DATA: Cylinder             = %d\n"
              "FDC: WRITE DATA: Head                 = %d\n"
              "FDC: WRITE DATA: Sector               = %d\n"
              "FDC: WRITE DATA: Number of Data Bytes = %d\n"
              "FDC: WRITE DATA: End of Track         = %d\n"
              "FDC: WRITE DATA: Gap Length           = %d\n"
              "FDC: WRITE DATA: Date Length          = %d\n"
              "FDC: WRITE DATA: ----------------------------------\n",
              ((_arg[0] >> 8) & 1) ? "yes" : "no",
              ((_arg[0] >> 7) & 1) ? "MFM Mode" : "FM Mode",
              ((_arg[0] >> 6) & 1) ? "yes" : "no",
              (_arg[1] >> 2) & 1,
              _arg[1] & 3,
              _arg[2],
              _arg[3],
              _arg[4],
              _arg[5],
              _arg[6],
              _arg[7],
              _arg[8]));
  get_fdc()->select_floppy(_arg[1] & 3);
  get_fdc()->set_input_gate(0x40, 0x40);
  _head = _arg[3];
  _cylinder = _arg[2];
  _sector = _arg[4];

  if (_buf != 0)
    delete _buf;
  _sector_size = N_to_sector_size((_arg[0] >> 7) & 1, _arg[5]);
  _buf = new byte_t[_sector_size];

  _idx = 0;

  _result[0] = get_fdc()->get_ST0();
  _result[1] = get_fdc()->get_ST1();
  _result[2] = get_fdc()->get_ST2();
  _result[3] = _arg[2];
  _result[4] = _arg[3];
  _result[5] = _arg[4];
  _result[6] = _arg[5];

  get_fdc()->set_msr(FDC::ST_MAIN_READ_WRITE | FDC::ST_MAIN_RQM | FDC::ST_MAIN_DIO,
		     FDC::ST_MAIN_READ_WRITE | FDC::ST_MAIN_RQM);

  _data_transfer = true;
}

void
FDC_CMD_WRITE_DATA::write_byte(byte_t val)
{
  DBG(2, form("KCemu/FDC_CMD/write_byte",
              "FDC_CMD_WRITE_DATA::write_byte(): c/h/s %d/%d/%d [%4d]: 0x%02x (%3d, '%c')\n",
	      get_fdc()->get_cylinder(),
	      get_fdc()->get_head(),
	      get_fdc()->get_sector(),
	      _idx,
              val, val, isprint(val) ? val : '.'));

  _buf[_idx++] = val;
  if (_idx == _sector_size)
    {
      Floppy *f = get_fdc()->get_floppy();
      if (f != 0)
        {
          get_fdc()->seek_internal(_head, _cylinder, _sector);
          f->write_sector(_buf, _sector_size);
        }

      if (_sector == _arg[6])
        {
          DBG(2, form("KCemu/FDC_CMD/WRITE_DATA",
                      "FDC: WRITE DATA: Writing sector %2d hit end of track (EOT = %d)\n",
                      _sector, _arg[6]));
          get_fdc()->set_ST0(FDC::ST_0_IC_MASK, FDC::ST_0_IC_ABNORMAL_TERMINATION);
          _data_transfer = false;
          finish_cmd();
        }
    }
}

/********************************************************************
 *  0x06 - READ DATA
 */
FDC_CMD_READ_DATA::FDC_CMD_READ_DATA(FDC *fdc)
  : FDC_CMD(fdc, 9, 7, "READ DATA")
{
  _buf = 0;
  _idx = 0;
  _size = 0;
}

FDC_CMD_READ_DATA::~FDC_CMD_READ_DATA(void)
{
  if (_buf != 0)
    delete _buf;
}

void
FDC_CMD_READ_DATA::execute(void)
{
  Floppy *f;
  int size, len;

  DBG(2, form("KCemu/FDC_CMD/READ_DATA",
              "FDC: READ DATA: ----------------------------------\n"
              "FDC: READ DATA: Multi-Track          = %s\n"
              "FDC: READ DATA: FM or MFM Mode       = %s\n"
              "FDC: READ DATA: Skip                 = %s\n"
              "FDC: READ DATA: Head Select          = %d\n"
              "FDC: READ DATA: Drive Select         = %d\n"
              "FDC: READ DATA: Cylinder             = %d\n"
              "FDC: READ DATA: Head                 = %d\n"
              "FDC: READ DATA: Sector               = %d\n"
              "FDC: READ DATA: Number of Data Bytes = %d\n"
              "FDC: READ DATA: End of Track         = %d\n"
              "FDC: READ DATA: Gap Length           = %d\n"
              "FDC: READ DATA: Date Length          = %d\n"
              "FDC: READ DATA: ----------------------------------\n",
              ((_arg[0] >> 8) & 1) ? "yes" : "no",
              ((_arg[0] >> 7) & 1) ? "MFM Mode" : "FM Mode",
              ((_arg[0] >> 6) & 1) ? "yes" : "no",
              (_arg[1] >> 2) & 1,
              _arg[1] & 3,
              _arg[2],
              _arg[3],
              _arg[4],
              _arg[5],
              _arg[6],
              _arg[7],
              _arg[8]));
  get_fdc()->select_floppy(_arg[1] & 3);
  get_fdc()->set_input_gate(0x40, 0x40);
  get_fdc()->seek_internal(_arg[3], _arg[2], _arg[4]);
  f = get_fdc()->get_floppy();
  if (f == 0)
    return;

  _result[0] = get_fdc()->get_ST0();
  _result[1] = get_fdc()->get_ST1();
  _result[2] = get_fdc()->get_ST2();
  _result[3] = _arg[2];
  _result[4] = _arg[3];
  _result[5] = _arg[4];
  _result[6] = _arg[5];

  size = f->get_sector_size();
  if (size <= 0)
    {
      get_fdc()->set_ST0(FDC::ST_0_IC_MASK, FDC::ST_0_IC_ABNORMAL_TERMINATION);
      get_fdc()->set_ST1(FDC::ST_1_NO_DATE | FDC::ST_1_MISSING_ADDRESS_MARK,
			 FDC::ST_1_NO_DATE | FDC::ST_1_MISSING_ADDRESS_MARK);
      _result[0] = get_fdc()->get_ST0();
      _result[1] = get_fdc()->get_ST1();
      return;
    }

  DBG(2, form("KCemu/FDC_CMD/READ_DATA_FORMAT",
              "FDC: READ DATA: heads:        %d\n"
              "FDC: READ DATA: cylinders:    %d\n"
              "FDC: READ DATA: sector size:  %d\n"
              "FDC: READ DATA: sect per cyl: %d\n",
              f->get_head_count(),
              f->get_cylinder_count(),
              size,
              f->get_sectors_per_cylinder()));

  if (_buf != 0)
    delete[] _buf;
  _buf = new byte_t[size];

  len = f->read_sector(_buf, size);
  if (len != size)
    {
      get_fdc()->set_ST0(FDC::ST_0_IC_MASK, FDC::ST_0_IC_ABNORMAL_TERMINATION);
      get_fdc()->set_ST1(FDC::ST_1_NO_DATE | FDC::ST_1_MISSING_ADDRESS_MARK,
			 FDC::ST_1_NO_DATE | FDC::ST_1_MISSING_ADDRESS_MARK);
      _result[0] = get_fdc()->get_ST0();
      _result[1] = get_fdc()->get_ST1();
      return;
    }

  _idx = 0;
  _size = size;

  DBG(2, form("KCemu/FDC_CMD/READ_DATA_DUMP",
              "FDC: READ DATA: %02x %02x %02x %02x %02x %02x %02x %02x\n"
              "FDC: READ DATA: %02x %02x %02x %02x %02x %02x %02x %02x\n"
              "FDC: READ DATA: %02x %02x %02x %02x %02x %02x %02x %02x\n"
              "FDC: READ DATA: %02x %02x %02x %02x %02x %02x %02x %02x\n"
              "FDC: READ DATA: %02x %02x %02x %02x %02x %02x %02x %02x\n"
              "FDC: READ DATA: %02x %02x %02x %02x %02x %02x %02x %02x\n"
              "FDC: READ DATA: %02x %02x %02x %02x %02x %02x %02x %02x\n"
              "FDC: READ DATA: %02x %02x %02x %02x %02x %02x %02x %02x\n",
	      _buf[0x00], _buf[0x01], _buf[0x02], _buf[0x03], _buf[0x04], _buf[0x05], _buf[0x06], _buf[0x07],
	      _buf[0x08], _buf[0x09], _buf[0x0a], _buf[0x0b], _buf[0x0c], _buf[0x0d], _buf[0x0e], _buf[0x0f],
	      _buf[0x10], _buf[0x11], _buf[0x12], _buf[0x13], _buf[0x14], _buf[0x15], _buf[0x16], _buf[0x17],
	      _buf[0x18], _buf[0x19], _buf[0x1a], _buf[0x1b], _buf[0x1c], _buf[0x1d], _buf[0x1e], _buf[0x1f],
	      _buf[0x20], _buf[0x21], _buf[0x22], _buf[0x23], _buf[0x24], _buf[0x25], _buf[0x26], _buf[0x27],
	      _buf[0x28], _buf[0x29], _buf[0x2a], _buf[0x2b], _buf[0x2c], _buf[0x2d], _buf[0x2e], _buf[0x2f],
	      _buf[0x30], _buf[0x31], _buf[0x32], _buf[0x33], _buf[0x34], _buf[0x35], _buf[0x36], _buf[0x37],
	      _buf[0x38], _buf[0x39], _buf[0x3a], _buf[0x3b], _buf[0x3c], _buf[0x3d], _buf[0x3e], _buf[0x3f]));


  get_fdc()->set_msr(FDC::ST_MAIN_READ_WRITE | FDC::ST_MAIN_RQM | FDC::ST_MAIN_DIO,
		     FDC::ST_MAIN_READ_WRITE | FDC::ST_MAIN_RQM | FDC::ST_MAIN_DIO);
  get_fdc()->set_ST0(FDC::ST_0_IC_MASK | FDC::ST_0_SEEK_END,
		     FDC::ST_0_IC_NORMAL_TERMINATION);

  _data_transfer = true;
}

bool
FDC_CMD_READ_DATA::fetch_next_sector(void)
{
  Floppy *f;
  int sector, size, cnt, len;

  f = get_fdc()->get_floppy();
  if (f == 0)
    return false;

  size = f->get_sector_size();
  if (size == 0)
    return false;
  
  cnt = f->get_sectors_per_cylinder();
  sector = get_fdc()->get_sector();
  if (sector < 0)
    return false;

  if (sector >= cnt)
    return false;

  DBG(2, form("KCemu/FDC_CMD/READ_DATA",
              "FDC: READ DATA: Reading sector %2d\n",
              sector));

  get_fdc()->seek_internal(get_fdc()->get_head(),
			   get_fdc()->get_cylinder(),
			   sector + 1);
  len = f->read_sector(_buf, size);
  if (len != size)
    return false;

  _idx = 0;
  _size = size;

  _result[3] = get_fdc()->get_head();
  _result[4] = get_fdc()->get_cylinder();
  _result[5] = sector + 1;

  return true;
}
  
byte_t
FDC_CMD_READ_DATA::read_byte(void)
{
  byte_t b = 0xff;

  if (_idx == _size)
    {
      if (!fetch_next_sector())
	{
	  _data_transfer = false;
	  finish_cmd();
	}
    }

  if (_idx < _size)
    b = _buf[_idx++];  

  DBG(2, form("KCemu/FDC_CMD/read_byte",
              "FDC_CMD_READ_DATA::read_byte():   c/h/s %d/%d/%d [%4d]: 0x%02x (%3d, '%c')\n",
	      get_fdc()->get_cylinder(),
	      get_fdc()->get_head(),
	      get_fdc()->get_sector(),
	      _idx - 1,
              b, b, isprint(b) ? b : '.'));
  
  int sector = get_fdc()->get_sector();
  if ((_idx == _size) && (sector == _arg[6]))
    {
      DBG(2, form("KCemu/FDC_CMD/READ_DATA",
                  "FDC: READ DATA: Reading sector %2d hit end of track (EOT = %d)\n",
                  sector, _arg[6]));
      get_fdc()->set_ST0(FDC::ST_0_IC_MASK, FDC::ST_0_IC_ABNORMAL_TERMINATION);
      _data_transfer = false;
      finish_cmd();
    }

  return b;
}

/********************************************************************
 *  0x07 - FDC_CMD_RECALIBRATE
 */
FDC_CMD_RECALIBRATE::FDC_CMD_RECALIBRATE(FDC *fdc)
  : FDC_CMD(fdc, 2, 0, "RECALIBRATE")
{
}

FDC_CMD_RECALIBRATE::~FDC_CMD_RECALIBRATE(void)
{
}

void
FDC_CMD_RECALIBRATE::execute(void)
{
  DBG(2, form("KCemu/FDC_CMD/RECALIBRATE",
              "FDC: RECALIBRATE: --------------------------------\n"
              "FDC: RECALIBRATE: Drive Select = %d\n"
              "FDC: RECALIBRATE: --------------------------------\n",
              _arg[1] & 3));

  get_fdc()->select_floppy(_arg[1] & 3);

  // Head retracted to Track 0, always sets SEEK END, if not track 0
  // signal is received from the floppy the EC bit is set
  get_fdc()->seek(get_fdc()->get_head(), 0, get_fdc()->get_sector());
}

/********************************************************************
 *  0x08 - FDC_CMD_SENSE_INTERRUPT_STATUS
 */
FDC_CMD_SENSE_INTERRUPT_STATUS::FDC_CMD_SENSE_INTERRUPT_STATUS(FDC *fdc)
  : FDC_CMD(fdc, 1, 2, "SENSE INTERRUPT STATUS")
{
}

FDC_CMD_SENSE_INTERRUPT_STATUS::~FDC_CMD_SENSE_INTERRUPT_STATUS(void)
{
}

void
FDC_CMD_SENSE_INTERRUPT_STATUS::execute(void)
{
  DBG(2, form("KCemu/FDC_CMD/SENSE_INTERRUPT_STATUS",
              "FDC: SENSE INTERRUPT STATUS: --------------------------------\n"
              "FDC: SENSE INTERRUPT STATUS: --------------------------------\n"
              ));

  _result[0] = get_fdc()->get_ST0();
  _result[1] = get_fdc()->get_cylinder(); // PCN (current cylinder)
}

/********************************************************************
 *  0x09 - FDC_CMD_WRITE_DELETED_DATA
 */
FDC_CMD_WRITE_DELETED_DATA::FDC_CMD_WRITE_DELETED_DATA(FDC *fdc)
  : FDC_CMD(fdc, 9, 7, "WRITE DELETED DATA")
{
}

FDC_CMD_WRITE_DELETED_DATA::~FDC_CMD_WRITE_DELETED_DATA(void)
{
}

void
FDC_CMD_WRITE_DELETED_DATA::execute(void)
{
  DBG(2, form("KCemu/FDC_CMD/WRITE_DELETED_DATA",
              "FDC: WRITE DELETED DATA: -------------------------\n"
              "FDC: WRITE DELETED DATA: -------------------------\n"));
}

/********************************************************************
 *  0x0a - FDC_CMD_READ_ID
 */
FDC_CMD_READ_ID::FDC_CMD_READ_ID(FDC *fdc)
  : FDC_CMD(fdc, 2, 7, "READ ID")
{
}

FDC_CMD_READ_ID::~FDC_CMD_READ_ID(void)
{
}

void
FDC_CMD_READ_ID::execute(void)
{
  DBG(2, form("KCemu/FDC_CMD/READ_ID",
              "FDC: READ ID: ------------------------------------\n"
	      "FDC: READ ID: FM or MFM Mode         = %s\n"
              "FDC: READ ID: Head Select            = %d\n"
              "FDC: READ ID: Drive Select           = %d\n"
              "FDC: READ ID: ------------------------------------\n",
              ((_arg[0] >> 7) & 1) ? "MFM Mode" : "FM Mode",
              (_arg[1] >> 2) & 1,
              _arg[1] & 3));

  get_fdc()->select_floppy(_arg[1] & 3);
  get_fdc()->set_input_gate(0x40, 0x40);

  _result[0] = get_fdc()->get_ST0();
  _result[1] = get_fdc()->get_ST1();
  _result[2] = get_fdc()->get_ST2();
  _result[3] = get_fdc()->get_cylinder();
  _result[4] = get_fdc()->get_head();
  _result[5] = get_fdc()->get_sector();
  _result[6] = 0x03; /* FIXME: N */
}

/********************************************************************
 *  0x0c - FDC_CMD_READ_DELETED_DATA
 */
FDC_CMD_READ_DELETED_DATA::FDC_CMD_READ_DELETED_DATA(FDC *fdc)
  : FDC_CMD(fdc, 9, 7, "READ DELETED DATA")
{
}

FDC_CMD_READ_DELETED_DATA::~FDC_CMD_READ_DELETED_DATA(void)
{
}

void
FDC_CMD_READ_DELETED_DATA::execute(void)
{
  DBG(2, form("KCemu/FDC_CMD/READ_DELETED_DATA",
              "FDC: READ DELETED DATA: --------------------------\n"
              "FDC: READ DELETED DATA: --------------------------\n"));
}

/********************************************************************
 *  0x0d - FDC_CMD_FORMAT_A_TRACK
 */
FDC_CMD_FORMAT_A_TRACK::FDC_CMD_FORMAT_A_TRACK(FDC *fdc)
  : FDC_CMD(fdc, 6, 7, "FORMAT A TRACK")
{
  _buf = NULL;
}

FDC_CMD_FORMAT_A_TRACK::~FDC_CMD_FORMAT_A_TRACK(void)
{
  if (_buf != NULL)
    delete _buf;
}

void
FDC_CMD_FORMAT_A_TRACK::execute(void)
{
  DBG(2, form("KCemu/FDC_CMD/FORMAT_A_TRACK",
              "FDC: FORMAT A TRACK: --------------------------------\n"
	      "FDC: FORMAT A TRACK: FM or MFM Mode  = %s\n"
              "FDC: FORMAT A TRACK: Head Select     = %d\n"
              "FDC: FORMAT A TRACK: Drive Select    = %d\n"
              "FDC: FORMAT A TRACK: Bytes/Sector    = %d\n"
              "FDC: FORMAT A TRACK: Sector/Cylinder = %d\n"
              "FDC: FORMAT A TRACK: Gap 3 Length    = %d\n"
              "FDC: FORMAT A TRACK: Filler byte     = %02xh (%d)\n"
              "FDC: FORMAT A TRACK: --------------------------------\n",
              ((_arg[0] >> 7) & 1) ? "MFM Mode" : "FM Mode",
              (_arg[1] >> 2) & 1,
              _arg[1] & 3,
	      _arg[2],
	      _arg[3],
	      _arg[4],
	      _arg[5], _arg[5]));
  get_fdc()->select_floppy(_arg[1] & 3);
  get_fdc()->set_input_gate(0x40, 0x40);

  _ridx = 0;
  _widx = 0;
  _cur_sector = 1;
  _formatted_sectors = 0;
  _sectors_per_track = _arg[3];
  if (_buf != NULL)
    delete _buf;
  _sector_size = N_to_sector_size((_arg[0] >> 7) & 1, _arg[2]);
  _buf = new byte_t[_sector_size];
  memset(_buf, _arg[5], _sector_size);

  _result[0] = get_fdc()->get_ST0();
  _result[1] = get_fdc()->get_ST1();
  _result[2] = get_fdc()->get_ST2();
  _result[3] = get_fdc()->get_cylinder();
  _result[4] = get_fdc()->get_head();
  _result[5] = get_fdc()->get_sector();
  _result[6] = _arg[2]; // N

  _data_transfer = true;
}

void
FDC_CMD_FORMAT_A_TRACK::format(void)
{
  Floppy *f;

  DBG(2, form("KCemu/FDC_CMD/format",
              "FDC: FORMAT A TRACK: -sector data--------------------\n"
              "FDC: FORMAT A TRACK: Sector %d of %d\n"
              "FDC: FORMAT A TRACK: Head                 = %d\n"
              "FDC: FORMAT A TRACK: Cylinder             = %d\n"
              "FDC: FORMAT A TRACK: Sector               = %d\n"
              "FDC: FORMAT A TRACK: Number of Data Bytes = %d\n"
              "FDC: FORMAT A TRACK: -sector data--------------------\n",
              _cur_sector, _sectors_per_track,
              _head, _cylinder, _sector, _bytes_per_sector));

  _ridx = 0;
  _widx = 0;

  f = get_fdc()->get_floppy();
  if (f != 0)
    {
      get_fdc()->seek(_head, _cylinder, _sector);
      f->write_sector(_buf, _sector_size);
    }

  if (_cur_sector == _sectors_per_track)
    get_fdc()->set_input_gate(0x40, 0x00);

  _cur_sector++;
  _formatted_sectors++;
}

byte_t
FDC_CMD_FORMAT_A_TRACK::read_byte(void)
{
  switch (_ridx)
    {
    case 0:
      _ridx++;
      return _cylinder;
    case 1:
      _ridx++;
      return _head;
    case 2:
      _ridx++;
      return _sector + 1;
    case 3:
      _ridx = 0;
      return _bytes_per_sector;
    }

  return 0;
}

void
FDC_CMD_FORMAT_A_TRACK::write_byte(byte_t val)
{
  DBG(2, form("KCemu/FDC_CMD/write_byte",
              "FDC_CMD_FORMAT_A_TRACK::write_byte(): 0x%02x (%3d, '%c')\n",
              val, val, isprint(val) ? val : '.'));

  switch (_widx)
    {
    case 0:
      _cylinder = val;
      _widx++;
      break;
    case 1:
      _head = val;
      _widx++;
      break;
    case 2:
      _sector = val;
      _widx++;
      break;
    case 3:
      _bytes_per_sector = val;
      _widx = 0;
      format();

      if (_formatted_sectors == _sectors_per_track)
	{
	  _data_transfer = false;
	  finish_cmd();
	}

      break;
    }
}

/********************************************************************
 *  0x0f - FDC_CMD_SEEK
 */
FDC_CMD_SEEK::FDC_CMD_SEEK(FDC *fdc)
  : FDC_CMD(fdc, 3, 0, "SEEK")
{
}

FDC_CMD_SEEK::~FDC_CMD_SEEK(void)
{
}

void
FDC_CMD_SEEK::execute(void)
{
  DBG(2, form("KCemu/FDC_CMD/SEEK",
              "FDC: SEEK: ---------------------------------------\n"
              "FDC: SEEK: Head Select         = %d\n"
              "FDC: SEEK: Drive Select        = %d\n"
              "FDC: SEEK: New Cylinder Number = %d\n"
              "FDC: SEEK: ---------------------------------------\n",
              (_arg[1] >> 2) & 1,
              _arg[1] & 3,
              _arg[2]));

  get_fdc()->select_floppy(_arg[1] & 3);
  get_fdc()->seek((_arg[1] >> 2) & 1, _arg[2], 1);
}

/********************************************************************
 *  0x11 - FDC_CMD_SCAN_EQUAL
 */
FDC_CMD_SCAN_EQUAL::FDC_CMD_SCAN_EQUAL(FDC *fdc)
  : FDC_CMD(fdc, 9, 7, "SCAN EQUAL")
{
  _buf = 0;
}

FDC_CMD_SCAN_EQUAL::~FDC_CMD_SCAN_EQUAL(void)
{
  if (_buf != 0)
    delete _buf;
}

void
FDC_CMD_SCAN_EQUAL::execute(void)
{
  DBG(2, form("KCemu/FDC_CMD/SCAN_EQUAL",
              "FDC: SCAN EQUAL: ----------------------------------\n"
              "FDC: SCAN EQUAL: Head Select          = %d\n"
              "FDC: SCAN EQUAL: Drive Select         = %d\n"
              "FDC: SCAN EQUAL: Cylinder             = %d\n"
              "FDC: SCAN EQUAL: Head                 = %d\n"
              "FDC: SCAN EQUAL: Sector               = %d\n"
              "FDC: SCAN EQUAL: Number of Data Bytes = %d\n"
              "FDC: SCAN EQUAL: End of Track         = %d\n"
              "FDC: SCAN EQUAL: Gap Length           = %d\n"
              "FDC: SCAN EQUAL: Date Length          = %d\n"
              "FDC: SCAN EQUAL: ----------------------------------\n",
              (_arg[1] >> 2) & 1,
              _arg[1] & 3,
              _arg[2],
              _arg[3],
              _arg[4],
              _arg[5],
              _arg[6],
              _arg[7],
              _arg[8]));

  get_fdc()->select_floppy(_arg[1] & 3);
  get_fdc()->set_input_gate(0x40, 0x40);

  _head = _arg[3];
  _cylinder = _arg[2];
  _sector = _arg[4];
  get_fdc()->seek_internal(_head, _cylinder, _sector);

  Floppy *f = get_fdc()->get_floppy();
  if (f == NULL)
    return;

  _result[0] = get_fdc()->get_ST0();
  _result[1] = get_fdc()->get_ST1();
  _result[2] = get_fdc()->get_ST2();
  _result[3] = _arg[2];
  _result[4] = _arg[3];
  _result[5] = _arg[4];
  _result[6] = _arg[5];

  get_fdc()->set_ST2(FDC::ST_2_SCAN_MASK, 0);

  int size = f->get_sector_size();
  if (size <= 0)
    {
      get_fdc()->set_ST0(FDC::ST_0_IC_MASK, FDC::ST_0_IC_ABNORMAL_TERMINATION);
      _result[0] = get_fdc()->get_ST0();
      _result[1] = get_fdc()->get_ST1();
      return;
    }

  if (_buf != 0)
    delete _buf;
  _buf = new byte_t[size];

  int len = f->read_sector(_buf, size);
  if (len != size)
    {
      get_fdc()->set_ST0(FDC::ST_0_IC_MASK, FDC::ST_0_IC_ABNORMAL_TERMINATION);
      _result[0] = get_fdc()->get_ST0();
      _result[1] = get_fdc()->get_ST1();
      return;
    }

  _idx = 0;
  _sector_size = size;

  get_fdc()->set_msr(FDC::ST_MAIN_READ_WRITE | FDC::ST_MAIN_RQM | FDC::ST_MAIN_DIO,
		     FDC::ST_MAIN_READ_WRITE | FDC::ST_MAIN_RQM);

  _data_transfer = true;
}

void
FDC_CMD_SCAN_EQUAL::write_byte(byte_t val)
{
  byte_t b = _buf[_idx];
  DBG(2, form("KCemu/FDC_CMD/write_byte",
              "FDC_CMD_SCAN_EQUAL::write_byte(): c/h/s %d/%d/%d [%4d]: "
	      "0x%02x (%3d, '%c') == 0x%02x (%3d, '%c')\n",
	      get_fdc()->get_cylinder(),
	      get_fdc()->get_head(),
	      get_fdc()->get_sector(),
	      _idx,
              val, val, isprint(val) ? val : '.',
	      b, b, isprint(b) ? b : '.'));

  if (b != val)
    {
      get_fdc()->set_ST2(FDC::ST_2_SCAN_MASK, FDC::ST_2_SCAN_NOT_SATISFIED);
      _result[0] = get_fdc()->get_ST0();
      _result[1] = get_fdc()->get_ST1();
      _result[2] = get_fdc()->get_ST2();
      _data_transfer = false;
      finish_cmd();
    }

  _idx++;

  if (_idx == _sector_size)
    {
      get_fdc()->set_ST2(FDC::ST_2_SCAN_MASK, FDC::ST_2_SCAN_EQUAL_HIT);
      _result[0] = get_fdc()->get_ST0();
      _result[1] = get_fdc()->get_ST1();
      _result[2] = get_fdc()->get_ST2();
      _data_transfer = false;
      finish_cmd();
    }
}

/********************************************************************
 *  0x19 - FDC_CMD_SCAN_LOW_OR_EQUAL
 */
FDC_CMD_SCAN_LOW_OR_EQUAL::FDC_CMD_SCAN_LOW_OR_EQUAL(FDC *fdc)
  : FDC_CMD(fdc, 9, 7, "SCAN LOW OR EQUAL")
{
}

FDC_CMD_SCAN_LOW_OR_EQUAL::~FDC_CMD_SCAN_LOW_OR_EQUAL(void)
{
}

void
FDC_CMD_SCAN_LOW_OR_EQUAL::execute(void)
{
  DBG(2, form("KCemu/FDC_CMD/SCAN_LOW_OR_EQUAL",
              "FDC: SCAN LOW OR EQUAL: --------------------------\n"
              "FDC: SCAN LOW OR EQUAL: --------------------------\n"));
}

/********************************************************************
 *  0x1d - FDC_CMD_SCAN_HIGH_OR_EQUAL
 */
FDC_CMD_SCAN_HIGH_OR_EQUAL::FDC_CMD_SCAN_HIGH_OR_EQUAL(FDC *fdc)
  : FDC_CMD(fdc, 9, 7, "SCAN HIGH OR EQUAL")
{
}

FDC_CMD_SCAN_HIGH_OR_EQUAL::~FDC_CMD_SCAN_HIGH_OR_EQUAL(void)
{
}

void
FDC_CMD_SCAN_HIGH_OR_EQUAL::execute(void)
{
  DBG(2, form("KCemu/FDC_CMD/SCAN_HIGH_OR_EQUAL",
              "FDC: SCAN HIGH OR EQUAL: -------------------------\n"
              "FDC: SCAN HIGH OR EQUAL: --------------------------\n"));
}
