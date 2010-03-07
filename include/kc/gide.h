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

#ifndef __kc_gide_h
#define __kc_gide_h

#include "kc/system.h"

#include "kc/ports.h"

typedef struct hard_disk_config
{
  int nr_of_cylinders;
  int nr_of_heads;
  int nr_of_sectors;
  const char *name;
  const char *filename;
} hard_disk_config_t;

class GIDE : public PortInterface
{
 private:
  enum {
    GIDE_STATUS_ERROR          = (1 << 0),
    GIDE_STATUS_INDEX          = (1 << 1),
    GIDE_STATUS_CORRECTED_DATA = (1 << 2),
    GIDE_STATUS_DATA_REQUEST   = (1 << 3),
    GIDE_STATUS_SEEK_COMPLETE  = (1 << 4),
    GIDE_STATUS_WRITE_FAULT    = (1 << 5),
    GIDE_STATUS_READY          = (1 << 6),
    GIDE_STATUS_BUSY           = (1 << 7),
    GIDE_STATUS_ALL            = 0xff,
  };

  enum {
    GIDE_RTC_REG_F_TEST        = (1 << 3),
    GIDE_RTC_REG_F_12_24       = (1 << 2),
    GIDE_RTC_REG_F_STOP        = (1 << 1),
    GIDE_RTC_REG_F_RESET       = (1 << 0),
  };

 private:
  int _cylinder;
  int _sector;
  int _sector_count;
  int _head;
  int _drive;
  int _error;
  byte_t _status;
  word_t _rtc_register;
  byte_t _sector_buffer[512];
  hard_disk_config_t _hard_disk_config[2];

 private:
  void set_drive_id(void);
  void set_drive_id_word(byte_t *buf, int val);
  void set_drive_id_string(byte_t *buf, int maxlen, const char *str);
  bool seek(FILE *file, int cylinder, int head, int sector);

 protected:
  byte_t in_rtc(word_t addr);
  byte_t in_data(void);
  byte_t in_error(void);
  byte_t in_sector_count();
  byte_t in_sector_number();
  byte_t in_cylinder_low();
  byte_t in_cylinder_high();
  byte_t in_drive_head();
  byte_t in_status(void);

  void out_rtc(word_t addr, byte_t val);
  void out_data(byte_t val);
  void out_command(byte_t val);
  void out_digital_output(byte_t val);
  void out_sector_count(byte_t val);
  void out_sector_number(byte_t val);
  void out_cylinder_low(byte_t val);
  void out_cylinder_high(byte_t val);
  void out_drive_head(byte_t val);

  void set_status(byte_t mask, byte_t val);

 public:
  GIDE(void);
  virtual ~GIDE(void);
  
  /* PortInterface */
  byte_t in(word_t addr);
  void out(word_t addr, byte_t val);
};

#endif /* __kc_gide_h */
