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

/*
 *  The following CP/M Disk Parameter Block description is from the
 *  22DISK Tool by Sydex:
 *
 *  BSH refers to the block shift or the number of left shifts needed
 *       to translate an allocation block number to a relative sector
 *       address.
 *  
 *  BLM refers to the block mask or a bit mask of ones  corresponding
 *       to the number of bits specified by BSH.  For example, if BSH
 *       is 3, BLM is 7 (3 bits).
 *  
 *  EXM refers  to  the  extent  mask  or a bit mask of ones used in
 *       delimiting extents, or groups of 128 128-byte records.
 *  
 *  DSM refers to the total number of sectors present on a  diskette.
 *       This  quantity,  like all of the CP/M system variables,  as-
 *       sumes a 128-byte sector.
 *  
 *  DRM refers to the total number of directory entries available  on
 *       a diskette.  DRM is also related to the AL0 and AL1 fields.
 *  
 *  AL0 and AL1 form a bit mask,  beginning with the most significant
 *       bit in byte AL0 and ending with the least significant bit in
 *       AL1.   These two bytes map the first 16 allocation blocks of
 *       the  disk.   A 1 bit in a position indicates that an alloca-
 *       tion block is reserved,  usually for the directory (see DRM,
 *       above).
 *  
 *  OFS indicates the number of tracks that are used for storing the
 *       CP/M system and BIOS at the beginning of a diskette.
 *  
 *  SOFS indicates the number of sectors that are  used  for  storing
 *       the  CP/M  system  and  BIOS at the beginning of a diskette.
 *       SOFS is used when the number of sectors used by the CP/M  is
 *       not  an  integral  number of tracks,  such as for the Coleco
 *       Adam.
 *
 *
 *  A working parameter block to read standard KC-CAOS disks looks
 *  like this:
 *
 *    BEGIN KC KC
 *    DENSITY MFM,LOW
 *    CYLINDERS 80
 *    SIDES 2
 *    SECTORS 5,1024
 *    SIDE1 0 1,2,3,4,5
 *    SIDE2 1 1,2,3,4,5
 *    ORDER SIDES
 *    BSH 4 BLM 15 EXM 0 DSM 390 DRM 127 AL0 11000000B AL1 0 OFS 4
 *    END
 *
 */

#include <zlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libdisk/libdiskP.h"

typedef struct {
  FILE *f;
  gzFile gzf;
  int read_only;
  int len;
  struct {
    int  head;
    int  cylinder;
    int  sector;
    int  count;
    long offset;
  } offset[5000];
} dsk_data_t;

static const char * loader_22dsk_get_name(void);
static int loader_22dsk_open(libdisk_prop_t *prop, const char *path);
static void loader_22dsk_close(libdisk_prop_t *prop);
static int loader_22dsk_read_sector(libdisk_prop_t *prop, unsigned char *buf, int len);
static int loader_22dsk_write_sector(libdisk_prop_t *prop, unsigned char *buf, int len);

static libdisk_loader_t loader = {
  loader_22dsk_get_name,
  loader_22dsk_open,
  loader_22dsk_close,
  loader_22dsk_read_sector,
  loader_22dsk_write_sector
};

static gzFile
try_open_gzip(const char *path)
{
  gzFile f;
  char *filename;

  f = gzopen(path, "rb");
  if (f == NULL)
    {
      filename = (char *) malloc(strlen(path) + 4);
      if (filename == NULL)
        return NULL;

      strcpy(filename, path);
      strcat(filename, ".gz");
      f = gzopen(filename, "rb");
      free(filename);
    }

  return f;
}

static dsk_data_t *
do_open(const char *path)
{
  int ro;
  FILE *f;
  int b1, b2;
  gzFile *gzf;
  dsk_data_t *data;

  f = NULL;
  gzf = NULL;

  ro = 0;
  f = fopen(path, "rb+");
  if (f == NULL)
    {
      ro = 1;
      f = fopen(path, "rb");
      if (f == NULL)
        return NULL;
    }

  /* check for gzip file header */
  b1 = fgetc(f);
  b2 = fgetc(f);
  fseek(f, 0, SEEK_SET);
  if ((b1 == 0x1f) && (b2 == 0x8b))
    {
      fclose(f);
      f = NULL;
      ro = 1;
      gzf = try_open_gzip(path);
      if (gzf == NULL)
        return NULL;
    }

  data = (dsk_data_t *)malloc(sizeof(dsk_data_t));
  if (data == NULL)
    return NULL;

  data->f = f;
  data->gzf = gzf;
  data->read_only = ro;
  
  return data;
}

static int
do_seek(dsk_data_t *data, long offset)
{
  if (data->gzf != NULL)
    return gzseek(data->gzf, offset, SEEK_SET);

  return fseek(data->f, offset, SEEK_SET);
}

static int
do_read(dsk_data_t *data, unsigned char *buf, int len)
{
  if (data->gzf != NULL)
    return gzread(data->gzf, buf, len);

  return fread(buf, 1, len, data->f);
}

static int
do_write(dsk_data_t *data, unsigned char *buf, int len)
{
  if (data->gzf != NULL)
    return gzwrite(data->gzf, buf, len);

  return fwrite(buf, 1, len, data->f);
}

static int
do_getc(dsk_data_t *data)
{
  if (data->gzf != NULL)
    return gzgetc(data->gzf);

  return fgetc(data->f);
}

static
int read_offsets(dsk_data_t *data)
{
  long idx;
  int a, acyl, asid, lcyl, lsid, lsec, llen, c, count;

  idx = 0;
  data->len = 0;
  while (242)
    {
      acyl = do_getc(data); idx++; if (acyl == EOF) return 0;
      asid = do_getc(data); idx++; if (asid == EOF) return -1;
      lcyl = do_getc(data); idx++; if (lcyl == EOF) return -1;
      lsid = do_getc(data); idx++; if (lsid == EOF) return -1;
      lsec = do_getc(data); idx++; if (lsec == EOF) return -1;
      llen = do_getc(data); idx++; if (llen == EOF) return -1;
      c    = do_getc(data); idx++; if (c    == EOF) return -1;
      count = c;
      c    = do_getc(data); idx++; if (c    == EOF) return -1;
      count |= (c << 8);

      data->offset[data->len].head     = lsid;
      data->offset[data->len].cylinder = lcyl;
      data->offset[data->len].sector   = lsec;
      data->offset[data->len].count    = count;
      data->offset[data->len].offset   = idx;

      data->len++;

      for (a = 0;a < count;a++)
        {
          c = do_getc(data);
          idx++;
          if (c == EOF)
	    return -1;
        }
    }

}

static const char *
loader_22dsk_get_name(void)
{
  return "disk loader for Sydex AnaDisk disk-dumps";
}

static int
loader_22dsk_open(libdisk_prop_t *prop, const char *path)
{
  dsk_data_t *data;

  if (prop == NULL)
    return -1;

  data = do_open(path);
  if (data == NULL)
      return -1;

  if (read_offsets(data) < 0)
    return -1;

  prop->head_count = 2;
  prop->cylinder_count = 80;
  prop->sector_size = 1024;
  prop->sectors_per_cylinder = 5;
  prop->data = data;
  prop->loader = &loader;
  prop->read_only = data->read_only;

  return 0;
}

static void
loader_22dsk_close(libdisk_prop_t *prop)
{
  dsk_data_t *data;

  if (prop == NULL)
    return;

  if (prop->data == NULL)
    return;

  data = (dsk_data_t *)prop->data;

  if (data->f != NULL)
   fclose(data->f);
  if (data->gzf != NULL)
    gzclose(data->gzf);

  free(data);

  prop->data = NULL;
}

static int
loader_22dsk_seek(libdisk_prop_t *prop)
{
  long offset;
  int a, h, c, s;
  dsk_data_t *data;

  if (prop == NULL)
    return -1;

  if (prop->data == NULL)
    return -1;

  h = prop->head;
  c = prop->cylinder;
  s = prop->sector;

  if ((h < 0) || (h > 1))
    return -1;

  if ((c < 0) || (c > 79))
    return -1;

  if ((s < 1) || (s > 5))
    return -1;

  data = (dsk_data_t *)prop->data;

  offset = -1;
  for (a = 0;a < data->len;a++)
    {
      if ((h == data->offset[a].head) &&
          (c == data->offset[a].cylinder) &&
          (s == data->offset[a].sector))
        {
	  offset = data->offset[a].offset;
	  break;
        }
    }

  if (offset == -1)
    return -1;

  if (do_seek(data, offset) < 0)
    return -1;

  return 0;
}

static int
loader_22dsk_read_sector(libdisk_prop_t *prop, unsigned char *buf, int len)
{
  int l;
  dsk_data_t *data;

  if (loader_22dsk_seek(prop) < 0)
    return -1;

  data = (dsk_data_t *)prop->data; // seek doesn't succeed if this would fail!

  l = do_read(data, buf, len);

  if (l != len)
    return -1;

  return l;
}

static int
loader_22dsk_write_sector(libdisk_prop_t *prop, unsigned char *buf, int len)
{
  int l;
  dsk_data_t *data;

  if (loader_22dsk_seek(prop) < 0)
    return -1;

  data = (dsk_data_t *)prop->data; // seek doesn't succeed if this would fail!

  l = do_write(data, buf, len);

  if (l != len)
    return -1;

  return l;
}

void
loader_22dsk_init(void)
{
  libdisk_register_loader(&loader);
}
