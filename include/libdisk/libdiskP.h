/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: libdiskP.h,v 1.2 2002/10/31 00:51:48 torsten_paul Exp $
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

#ifndef __libdisk_libdiskp_h
#define __libdisk_libdiskp_h

#include "libdisk/libdisk.h"

typedef struct libdisk_loader
{
  const char * (*get_name)(void);
  int (*open)(libdisk_prop_t *prop, const char *path);
  void (*close)(libdisk_prop_t *prop);

  int (*read_sector)(libdisk_prop_t *prop, unsigned char *buf, int len);
  int (*write_sector)(libdisk_prop_t *prop, unsigned char *buf, int len);
} libdisk_loader_t;

#define MAX_DISK_LOADERS (10)

int  libdisk_register_loader(libdisk_loader_t *loader);

void loader_dir_init(void);
void loader_cdm_init(void);
void loader_22dsk_init(void);

#endif /* __libdisk_libdiskp_h */
