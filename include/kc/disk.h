/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-1998 Torsten Paul
 *
 *  $Id: disk.h,v 1.1 2000/07/08 17:50:08 tp Exp $
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

#include "kc/config.h"
#include "kc/system.h"

typedef enum
{
  DISK_OK,
  DISK_NOENT,
  DISK_ERROR,
} disk_error_t;

class Disk
{
 public:
  Disk(void);
  virtual ~Disk(void);

  virtual disk_error_t attach(int disk_no,
			      const char *filename,
			      bool create = false);
  virtual disk_error_t detach(int disk_no);
};

#endif /* __kc_disk_h */
