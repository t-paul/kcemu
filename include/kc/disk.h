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

#ifndef __kc_disk_h
#define __kc_disk_h

#include "kc/system.h"

typedef enum
{
  DISK_OK,
  DISK_NOENT,
  DISK_ERROR,
} disk_error_t;

typedef enum {
  DISK_TYPE_UNKNOWN,
  DISK_TYPE_22DSK,
  DISK_TYPE_CDM,
  DISK_TYPE_KCD,
} disk_type_t;

class Disk
{
 private:
  CMD *_cmd;

 protected:
  bool create_disk_file(FILE *f);
  bool write_sector(FILE *f, int c, int h, int s);

 public:
  Disk(void);
  virtual ~Disk(void);

  virtual disk_error_t attach(int disk_no,
			      const char *filename,
			      bool create = false);
  virtual disk_error_t detach(int disk_no);

  bool seek(int head, int cylinder, int sector);
  int read_sector(byte_t *buf, int len);
  int write_sector(byte_t *buf, int len);
};

#endif /* __kc_disk_h */
