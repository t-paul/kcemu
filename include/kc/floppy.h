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

#ifndef __kc_floppy_h
#define __kc_floppy_h

#include "kc/system.h"

#include "libdisk/libdisk.h"

class Floppy
{
 private:
  libdisk_prop_t *_disk_prop;

 public:
  Floppy(const char *cmdname);
  virtual ~Floppy(void);

  int get_head_count(void);
  int get_cylinder_count(void);
  int get_sector_size(void);
  int get_sectors_per_cylinder(void);
  
  bool attach(const char *filename);
  bool seek(int head, int cylinder, int sector);
  int read_sector(byte_t *buf, int len);
  int write_sector(byte_t *buf, int len);
};

#endif /* __kc_floppy_h */
