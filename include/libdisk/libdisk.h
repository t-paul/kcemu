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

#ifndef __libdisk_libdisk_h
#define __libdisk_libdisk_h

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
  LIBDISK_DISK_TYPE_UNKNOWN,
  LIBDISK_DISK_TYPE_22DSK,
  LIBDISK_DISK_TYPE_CDM,
  LIBDISK_DISK_TYPE_DIR,
} libdisk_type_t;

typedef struct libdisk_prop
{
  libdisk_type_t type;
  int read_only;
  int head_count;
  int cylinder_count;
  int sector_count;
  int sector_size;
  int sectors_per_cylinder;

  int head;
  int cylinder;
  int sector;

  struct libdisk_loader *loader;
  void *data;

  struct libdisk_prop *next;
} libdisk_prop_t;

/*
 *  initialize loader modules
 */
void            libdisk_init(void);

int             libdisk_open(libdisk_prop_t **prop, const char *path);
void            libdisk_close(libdisk_prop_t **prop);

int             libdisk_seek(libdisk_prop_t **prop, int head, int cylinder, int sector);
int             libdisk_read_sector(libdisk_prop_t **prop, unsigned char *buf, int len);
int             libdisk_write_sector(libdisk_prop_t **prop, unsigned char *buf, int len);
void            libdisk_show_config(void);

#ifdef __cplusplus
}
#endif

#endif /* __libdisk_libdisk_h */
