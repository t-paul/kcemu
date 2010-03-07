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

#ifndef __libdisk_teledisk_h
#define __libdisk_teledisk_h

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct teledisk_prop
{
  FILE *f;
  char id[3];
  int volume;
  int signature;
  int version_minor;
  int version_major;
  int heads;
  int cylinders;
  int sectors;
  int sector_size;
  int sectors_total;
  int dos_mode;
  char *comment;
  char *comment_date;
  char *filename;
  char *density;
  char *drive_type;
  unsigned char buf[2048];

  struct {
    int h, c, s, l;
    long o;
  } offset[8192];
} teledisk_prop_t;

teledisk_prop_t * teledisk_open(const char *filename);
int               teledisk_read_sector(teledisk_prop_t *prop, int c, int h, int s);
void              teledisk_close(teledisk_prop_t *prop);
  
#ifdef __cplusplus
}
#endif

#endif /* __libdisk_teledisk_h */
