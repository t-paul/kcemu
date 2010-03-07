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

#include <stdio.h>

#include "kc/system.h"
#include "kc/prefs/prefs.h"

#include "kc/gide.h"

#include "sys/sysdep.h"

#include "libdbg/dbg.h"

using namespace std;

/*
 *  Information taken from IDE-TCJ.txt and GIDE.txt, written
 *  by Tilmann Reh.
 *
 *  (see http://www.gaby.de/gide/ for the orignal GIDE interface and
 *  http://www.iee.et.tu-dresden.de/~kc-club/gide/gide.html for the
 *  description of the KC Club version)
 *
 */

GIDE::GIDE(void)
{
  _cylinder = 0;
  _sector = 0;
  _head = 0;
  _drive = 0;
  _sector_count = 1;
  _rtc_register = 0;

  // the classical Seagate ST-157A
  _hard_disk_config[0].nr_of_cylinders = 560;
  _hard_disk_config[0].nr_of_heads = 6;
  _hard_disk_config[0].nr_of_sectors = 26;
  _hard_disk_config[0].name = "KCemu virtual hard disk (ST-157A)";
  _hard_disk_config[0].filename = Preferences::instance()->get_string_value("hard_disk_1", NULL);

  // MyZ80 hard disk format
  _hard_disk_config[1].nr_of_cylinders = 512;
  _hard_disk_config[1].nr_of_heads = 1;
  _hard_disk_config[1].nr_of_sectors = 32;
  _hard_disk_config[1].name = "KCemu virtual hard disk (MyZ80)";
  _hard_disk_config[1].filename = Preferences::instance()->get_string_value("hard_disk_2", NULL);

  DBG(2, form("KCemu/GIDE/image",
	      "GIDE::GIDE(): hard disk 1: (C/H/S) = (%d/%d/%d) -> '%s'\n",
          _hard_disk_config[0].nr_of_cylinders,
          _hard_disk_config[0].nr_of_heads,
          _hard_disk_config[0].nr_of_sectors,
          _hard_disk_config[0].filename));

  DBG(2, form("KCemu/GIDE/image",
	      "GIDE::GIDE(): hard disk 2: (C/H/S) = (%d/%d/%d) -> '%s'\n",
          _hard_disk_config[1].nr_of_cylinders,
          _hard_disk_config[1].nr_of_heads,
          _hard_disk_config[1].nr_of_sectors,
          _hard_disk_config[1].filename));

  _error = 0;
  set_status(GIDE_STATUS_ALL, GIDE_STATUS_READY
	     | GIDE_STATUS_SEEK_COMPLETE
	     | GIDE_STATUS_DATA_REQUEST);
}

GIDE::~GIDE(void)
{
}

void
GIDE::set_drive_id_word(byte_t *buf, int val)
{
  buf[0] = val & 0xff;
  buf[1] = (val >> 8) & 0xff;
}

void
GIDE::set_drive_id_string(byte_t *buf, int maxlen, const char *str)
{
  int idx = -1;
  do
    {
      idx++;

      if (((idx & 1) == 0) && (str[idx] == 0))
	break;

      buf[idx] = str[idx ^ 1];
    }
  while ((idx < maxlen) && (str[idx] != 0));
}

void
GIDE::set_drive_id(void)
{
  memset(_sector_buffer, 0, 512);

  set_drive_id_word(&_sector_buffer[0x00], 0x0101); // configuration word

  set_drive_id_word(&_sector_buffer[0x02], _hard_disk_config[_drive].nr_of_cylinders);
  set_drive_id_word(&_sector_buffer[0x06], _hard_disk_config[_drive].nr_of_heads);
  set_drive_id_word(&_sector_buffer[0x0c], _hard_disk_config[_drive].nr_of_sectors);

  set_drive_id_word(&_sector_buffer[0x09], 0x4000); // nr of unformatted bytes per physical track
  set_drive_id_word(&_sector_buffer[0x0a], 0x0200); // nr of unformatted bytes per sector

  set_drive_id_string(&_sector_buffer[0x14], 20, KCEMU_VERSION);
  set_drive_id_string(&_sector_buffer[0x2e],  8, "KCemu");
  set_drive_id_string(&_sector_buffer[0x36], 40, _hard_disk_config[_drive].name);
}

bool
GIDE::seek(FILE *file, int cylinder, int head, int sector)
{
  int offset = 0x0100; // MyZ80 compatible offset

  int heads = _hard_disk_config[_drive].nr_of_heads;
  int sectors = _hard_disk_config[_drive].nr_of_sectors;

  offset += (_sector - 1) * 512;
  offset += _head * sectors * 512;
  offset += _cylinder * heads * sectors * 512;

  return fseek(file, offset, SEEK_SET) == 0;
}

void
GIDE::set_status(byte_t mask, byte_t val)
{
  byte_t status = (_status & ~mask) | val;

  DBG(2, form("KCemu/GIDE/status",
	      "GIDE::set_status(): mask = %02xh, val = %02xh [%02xh -> %02xh]\n",
	      mask, val, _status, status));

  _status = status;
}

/*
 *  Occupied I/O addresses:
 *
 *  x4	reserved for IDE expansion board
 *  x5	RTC access
 *  x6	IDE alternate status / digital output register
 *  x7	IDE drive address register
 *  x8	IDE data register
 *  x9	IDE error/feature register
 *  xA	IDE sector count register
 *  xB	IDE sector number register
 *  xC	IDE cylinder low register
 *  xD	IDE cylinder high register
 *  xE	IDE drive/head register
 *  xF	IDE command/status register
 *
 *  Wherein "x" represents the base address selected by J1. A plugged-on
 *  jumper sets the according address bit to zero, while removing a jumper
 *  sets it to a one. It should be noted that addresses x0 to x3 are not
 *  used at all - there will be no conflicts if any other I/O devices use
 *  these four addresses.
 *
 *
 *  The sixteen RTC registers are accessed by 16-bit I/O instructions,
 *  using the following addressing scheme:
 *
 *  y0x5	seconds, units
 *  y1x5	seconds, tens
 *  y2x5	minutes, units
 *  y3x5	minutes, tens
 *  y4x5	hours, units
 *  y5x5	hours, tens  & AM/PM flag
 *  y6x5	day, units
 *  y7x5	day, tens
 *  y8x5	month, units
 *  y9x5	month, tens
 *  yAx5	year, units
 *  yBx5	year, tens
 *  yCx5	day of week
 *  yDx5	control register D  (status/control)
 *  yEx5	control register E  (pulse output control)
 *  yFx5	control register F  (master control)
 *
 *  Wherein "x" represents the jumper selectable base address, and "y" is
 *  a "don't care". Each RTC register consists of 4 bits only, so the
 *  highermost four data bits are ignored during RTC writes and undefined
 *  during RTC reads.
 *
 *
 *  NOTE: on KC85/4 "x" must be 0
 *
 *
 *  IDE-Registers:
 *
 *  /CS0  /CS1  A2 A1 A0  Addr.   Read Function       Write Function
 *  ----------------------------------------------------------------------
 *    0     1    0  0  0   1F0    Data Register       Data Register
 *    0     1    0  0  1   1F1    Error Register      (Write Precomp Reg.)
 *    0     1    0  1  0   1F2    Sector Count        Sector Count
 *    0     1    0  1  1   1F3    Sector Number       Sector Number
 *    0     1    1  0  0   1F4    Cylinder Low        Cylinder Low
 *    0     1    1  0  1   1F5    Cylinder High       Cylinder High
 *    0     1    1  1  0   1F6    SDH Register        SDH Register
 *    0     1    1  1  1   1F7    Status Register     Command Register
 *    1     0    1  1  0   3F6    Alternate Status    Digital Output
 *    1     0    1  1  1   3F7    Drive Address       Not Used
 *
 */

byte_t
GIDE::in(word_t addr)
{
  byte_t val = 0xff;

  switch (addr & 0x0f)
    {
    case 0x05:
      val = in_rtc(addr);
      break;
    case 0x08:
      val = in_data();
      break;
    case 0x09:
      val = in_error();
      break;
    case 0x0a:
      val = in_sector_count();
      break;
    case 0x0b:
      val = in_sector_number();
      break;
    case 0x0c:
      val = in_cylinder_low();
      break;
    case 0x0d:
      val = in_cylinder_high();
      break;
    case 0x0e:
      val = in_drive_head();
      break;
    case 0x0f:
      val = in_status();
      break;
    default:
      break;
    }

  DBG(2, form("KCemu/GIDE/in",
	      "GIDE::in():  addr = %04x, val = %02x\n",
	      addr, val));

  return val;
}

void
GIDE::out(word_t addr, byte_t val)
{
  DBG(2, form("KCemu/GIDE/out",
	      "GIDE::out(): addr = %04x, val = %02x\n",
	      addr, val));

  switch (addr & 0x0f)
    {
    case 0x05:
      out_rtc(addr, val);
      break;
    case 0x06:
      out_digital_output(val);
      break;
    case 0x08:
      out_data(val);
      break;
    case 0x0a:
      out_sector_count(val);
      break;
    case 0x0b:
      out_sector_number(val);
      break;
    case 0x0c:
      out_cylinder_low(val);
      break;
    case 0x0d:
      out_cylinder_high(val);
      break;
    case 0x0e:
      out_drive_head(val);
      break;
    case 0x0f:
      out_command(val);
      break;
    default:
      break;
    }
}

byte_t
GIDE::in_data(void)
{
  static int x = 0;
  byte_t val = 0;

  val = _sector_buffer[x & 0x1ff];

  DBG(2, form("KCemu/GIDE/in",
	      "GIDE::in():  DATA READ %02xh [%c] (%3d)\n",
	      val,
	      isprint(val) ? val : '.',
	      x));

  x++;

  if (x == 512)
    {
      x = 0;
      set_status(GIDE_STATUS_DATA_REQUEST, 0);
    }

  return val;
}

byte_t
GIDE::in_error(void)
{
  return _error;
}

byte_t
GIDE::in_sector_count()
{
  DBG(2, form("KCemu/GIDE/in_sector_count",
	      "GIDE::in():  SECTOR COUNT %02xh (%d)\n",
	      _sector_count, _sector_count));

  return _sector_count;
}

byte_t
GIDE::in_sector_number()
{
  DBG(2, form("KCemu/GIDE/in_sector_number",
	      "GIDE::in():  SECTOR NUMBER %02xh (%d)\n",
	      _sector, _sector));

  return _sector;
}

byte_t
GIDE::in_cylinder_low()
{
  DBG(2, form("KCemu/GIDE/in_cylinder_low",
	      "GIDE::in():  CYLINDER LOW %02xh (%04xh / %d)\n",
	      _cylinder & 0xff, _cylinder, _cylinder));

  return _cylinder & 0xff;
}

byte_t
GIDE::in_cylinder_high()
{
  DBG(2, form("KCemu/GIDE/in_cylinder_high",
	      "GIDE::in():  CYLINDER LOW %02xh (%04xh / %d)\n",
	      (_cylinder >> 8) & 0xff, _cylinder, _cylinder));

  return (_cylinder >> 8) & 0xff;
}

byte_t
GIDE::in_drive_head()
{
  byte_t val = (_drive << 4) | _head;
  DBG(2, form("KCemu/GIDE/in_drive_head",
	      "GIDE::out(): DRIVE/HEAD %02xh\n"
	      "GIDE::out():   Extension Bit: %d\n"
	      "GIDE::out():   Sector Size:   %d\n"
	      "GIDE::out():   Drive:         %d\n"
	      "GIDE::out():   Head:          %d\n",
	      val,
	      (val >> 7) &  1, // fixed, do not append CRC
	      (val >> 6) &  3, // fixed, 512 bytes
	      (val >> 4) &  1,
	      (val     ) & 15));

  return val;
}

/*
 *  The read-only status register contains eight single-bit
 *  flags. It is updated at the completion of each command. If
 *  the busy bit is active, no other bits are valid. The index
 *  bit is valid independent of the applied command. The bit
 *  flags are:
 *
 *  Bit 7:    Busy flag. When this flag is set, the task file
 *            registers must not be accessed due to internal
 *            operations.
 *  Bit 6:    Drive ready. This bit is set when the drive is up
 *            to speed and ready to accept a command. When
 *            there is an error, this bit is not updated until
 *            the next read of the status register, so it can be
 *            used to determine the cause of the error.
 *  Bit 5:    Drive write fault. Similar to "drive ready", this
 *            bit is not updated after an error.
 *  Bit 4:    Drive seek complete. This bit is set when the
 *            actuator of the drive's head is on track. This
 *            bit also is updated similarly to "drive ready".
 *  Bit 3:    Data request. This bit indicates that the drive
 *            is ready for a data transfer.
 *  Bit 2:    Corrected data flag. Set when there was a
 *            correctable data error and the data has been
 *            corrected.
 *  Bit 1:    Index. This bit is active once per disk
 *            revolution. May be used to determine rotational
 *            speed.
 *  Bit 0:    Error flag. This bit is set whenever an error
 *            occurs. The other bits in the status register and
 *            the bits in the error register will then contain
 *            further information about the cause of the error.
 *
 */
byte_t
GIDE::in_status(void)
{
  return _status;
}

void
GIDE::out_data(byte_t val)
{
  static int x = 0;
  FILE *file;

  _sector_buffer[x & 0x1ff] = val;

  DBG(2, form("KCemu/GIDE/out_data",
	      "GIDE::out():  DATA WRITE %02xh [%c] (%3d)\n",
	      val,
	      isprint(val) ? val : '.',
	      x));

  x++;

  if (x == 512)
    {
      x = 0;
      set_status(GIDE_STATUS_DATA_REQUEST, 0);

      file = fopen(_hard_disk_config[_drive].filename, "rb+");
      if (file == NULL)
	file = fopen(_hard_disk_config[_drive].filename, "wb+");
      if (file != NULL)
	{
	  if (seek(file, _cylinder, _head, _sector))
	    fwrite(_sector_buffer, 1, 512, file);
	  fclose(file);
	}
    }
}

void
GIDE::out_digital_output(byte_t val)
{
  DBG(2, form("KCemu/GIDE/out_digital_output",
	      "GIDE::out(): DIGITAL OUTPUT %02xh: reset: %d, interrupt enable: %d\n",
	      val,
	      (val > 2) & 1,
	      (val > 1) & 1));
}

void
GIDE::out_command(byte_t val)
{
  bool ok;
  FILE *file;

  switch (val)
    {
    case 0x20:
      DBG(2, form("KCemu/GIDE/out_command",
		  "GIDE::out(): %02xh: READ SECTORS WITH RETRY\n", val));
      set_status(GIDE_STATUS_DATA_REQUEST, GIDE_STATUS_DATA_REQUEST);

      ok = false;
      memset(_sector_buffer, 0, 512);
      file = fopen(_hard_disk_config[_drive].filename, "rb");
      if (file != NULL)
	{
	  if (seek(file, _cylinder, _head, _sector))
	    if (fread(_sector_buffer, 1, 512, file) == 512)
	      ok = true;
	  fclose(file);
	}

      if (!ok)
	memset(_sector_buffer, 0xe5, 512);

      break;
    case 0x30:
      DBG(2, form("KCemu/GIDE/out_command",
		  "GIDE::out(): %02xh: WRITE SECTORS\n", val));
      set_status(GIDE_STATUS_DATA_REQUEST, GIDE_STATUS_DATA_REQUEST);
      break;
    case 0x90:
      DBG(2, form("KCemu/GIDE/out_command",
		  "GIDE::out(): %02xh: EXECUTE DIAGNOSTICS\n", val));

      if (_hard_disk_config[1].filename == NULL)
	_error = 0x81; // no error for first hd, no slave
      else
	_error = 0x01; // no error, slave present

      break;
    case 0x91:
      DBG(2, form("KCemu/GIDE/out_command",
		  "GIDE::out(): %02xh: SET DRIVE PARAMETERS\n", val));
      break;
    case 0xe1:
      DBG(2, form("KCemu/GIDE/out_command",
		  "GIDE::out(): %02xh: SET IDLE MODE\n", val));
      break;
    case 0xec:
      DBG(2, form("KCemu/GIDE/out_command",
		  "GIDE::out(): %02xh: IDENTIFY DRIVE\n", val));
      _error = 0x00;
      set_status(GIDE_STATUS_DATA_REQUEST, GIDE_STATUS_DATA_REQUEST);

      set_drive_id();

      break;
    }
}

void
GIDE::out_sector_count(byte_t val)
{
  _sector_count = val;

  DBG(2, form("KCemu/GIDE/out_sector_count",
	      "GIDE::out(): SECTOR COUNT %02xh (%d)\n",
	      val, val));
}

void
GIDE::out_sector_number(byte_t val)
{
  _sector = val;

  DBG(2, form("KCemu/GIDE/out_sector_number",
	      "GIDE::out(): SECTOR NUMBER %02xh (%d)\n",
	      val, val));
}

void
GIDE::out_cylinder_low(byte_t val)
{
  _cylinder = (_cylinder & 0xff00) | val;

  DBG(2, form("KCemu/GIDE/out_cylinder_low",
	      "GIDE::out(): CYLINDER LOW %02xh (%04xh / %d)\n",
	      val, _cylinder, _cylinder));
}

void
GIDE::out_cylinder_high(byte_t val)
{
  _cylinder = (_cylinder & 0x00ff) | (val << 8);

  DBG(2, form("KCemu/GIDE/out_cylinder_high",
	      "GIDE::out(): CYLINDER HIGH %02xh (%04xh / %d)\n",
	      val, _cylinder, _cylinder));
}
/*
 *  The SDH register is a special register serving several
 *  functions. SDH is an abbreviation for "Sector size, Drive
 *  and Head". The bits of this register are arranged as
 *  follows:
 *
 *  Bit 7:    Historical: Extension Bit. When zero, CRC data is
 *            appended to the sector's data fields. When set to
 *            one, no CRC data is appended. Since today's
 *            drives always use ECC error correction, this bit
 *            must always be set (no CRC).
 *  Bit 6-5:  Sector Size. Since today's drives always have 512-
 *            byte sectors (unchangeable by the user) because
 *            PCs are not able to support other sizes, these
 *            bits must always be 0-1.
 *  Bit 4:    Drive. This bit distinguishes between the two
 *            connected drives when using the master-slave
 *            chain. Single drives are always accessed with the
 *            drive bit set to zero.
 *  Bit 3-0:  Head number. These four bits contain the head
 *            number (that is, the disk surface number) for all
 *            following accesses. Similar to the cylinder and
 *            sector number, these bits are updated by the
 *            drive. The head number field is also used for
 *            drive initialization to specify the number of
 *            heads.
 */
void
GIDE::out_drive_head(byte_t val)
{
  DBG(2, form("KCemu/GIDE/out_drive_head",
	      "GIDE::out(): DRIVE/HEAD %02xh\n"
	      "GIDE::out():   Extension Bit: %d\n"
	      "GIDE::out():   Sector Size:   %d\n"
	      "GIDE::out():   Drive:         %d\n"
	      "GIDE::out():   Head:          %d\n",
	      val,
	      (val >> 7) &  1,
	      (val >> 6) &  3,
	      (val >> 4) &  1,
	      (val     ) & 15));

  _head = val & 15;
  _drive = (val >> 4) & 1;
}

byte_t
GIDE::in_rtc(word_t addr)
{
  byte_t val = 0xff;
  int year, month, day, hour, minute, second;

  sys_localtime(&year, &month, &day, &hour, &minute, &second);

  switch ((addr >> 8) & 0x0f)
    {
    case 0x00:
      val = second % 10;
      break;
    case 0x01:
      val = second / 10;
      break;
    case 0x02:
      val = minute % 10;
      break;
    case 0x03:
      val = minute / 10;
      break;
    case 0x04:
      val = hour % 10;
      break;
    case 0x05:
      val = hour / 10;
      break;
    case 0x06:
      val = day % 10;
      break;
    case 0x07:
      val = day / 10;
      break;
    case 0x08:
      val = month % 10;
      break;
    case 0x09:
      val = month / 10;
      break;
    case 0x0a:
      val = year % 10;
      break;
    case 0x0b:
      val = (year / 10) % 10;
      break;
    case 0x0c: // day of the week
      val = 0x00;
      break;
    case 0x0d: // control D
      val = 0x00;
      break;
    case 0x0e: // control E
      val = 0x00;
      break;
    case 0x0f: // control F
      val = GIDE_RTC_REG_F_12_24;
      break;
    }

  DBG(2, form("KCemu/GIDE/RTC/in",
	      "GIDE::in_rtc():  addr = %04x, val = %02x\n",
	      addr, val));

  return val;
}

void
GIDE::out_rtc(word_t addr, byte_t val)
{
  DBG(2, form("KCemu/GIDE/RTC/out",
	      "GIDE::out_rtc(): addr = %04x, val = %02x\n",
	      addr, val));

  _rtc_register = val;
}
