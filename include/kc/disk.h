/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: disk.h,v 1.4 2002/06/09 14:24:32 torsten_paul Exp $
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
