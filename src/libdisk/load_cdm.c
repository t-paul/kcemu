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
 *  CDM is the disk manager program which comes with the
 *  yaze CP/M emulator.
 *
 *  parameters for CDM create command:
 *
 *  $> create disk.img 819200 -b1024 -d127 -o0 -s5
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "libdisk/libdiskP.h"

typedef struct {
  FILE *f;
} cdm_data_t;

static const char * loader_cdm_get_name(void);
static int loader_cdm_open(libdisk_prop_t *prop, const char *path);
static void loader_cdm_close(libdisk_prop_t *prop);
static int loader_cdm_read_sector(libdisk_prop_t *prop, unsigned char *buf, int len);
static int loader_cdm_write_sector(libdisk_prop_t *prop, unsigned char *buf, int len);

static libdisk_loader_t loader = {
  loader_cdm_get_name,
  loader_cdm_open,
  loader_cdm_close,
  loader_cdm_read_sector,
  loader_cdm_write_sector
};

static const char *
loader_cdm_get_name(void)
{
  return "disk loader for Yaze CP/M files";
}

static int
loader_cdm_open(libdisk_prop_t *prop, const char *path)
{
  int ro;
  FILE *f;
  char buf[128];
  cdm_data_t *data;

  if (prop == NULL)
    return -1;

  ro = 0;
  f = fopen(path, "rb+");
  if (f == NULL)
    {
      ro = 1;
      f = fopen(path, "rb");
      if (f == NULL)
	return -1;
    }

  if (fread(buf, 1, 128, f) != 128)
    return -1;

  if (strncmp(buf, "<CPM_Disk>", 10) != 0)
    return -1;

  data = (cdm_data_t *)malloc(sizeof(cdm_data_t));
  if (data == NULL)
      return -1;

  data->f = f;

  prop->read_only = ro;
  prop->head_count = 2;
  prop->cylinder_count = 80;
  prop->sector_size = 1024;
  prop->sectors_per_cylinder = 5;
  prop->data = data;
  prop->loader = &loader;

  return 0;
}

static void
loader_cdm_close(libdisk_prop_t *prop)
{
  cdm_data_t *data;

  if (prop == NULL)
    return;

  if (prop->data == NULL)
    return;

  data = (cdm_data_t *)prop->data;

  fclose(data->f);
  free(data);

  prop->data = NULL;
}

static int
loader_cdm_seek(libdisk_prop_t *prop)
{
  long offset;
  int h, c, s;
  cdm_data_t *data;

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

  s--;

  offset = (10 * c + 5 * h + s) * 1024 + 128;
  data = (cdm_data_t *)prop->data;

  if (fseek(data->f, offset, SEEK_SET) < 0)
    return -1;

  return 0;
}

static int
loader_cdm_read_sector(libdisk_prop_t *prop, unsigned char *buf, int len)
{
  int l;
  cdm_data_t *data;

  if (loader_cdm_seek(prop) < 0)
    return -1;

  data = (cdm_data_t *)prop->data; // seek doesn't succeed if this would fail!

  l = fread(buf, 1, len, data->f);
  if (l != len)
    return -1;

  return l;
}

static int
loader_cdm_write_sector(libdisk_prop_t *prop, unsigned char *buf, int len)
{
  int l;
  cdm_data_t *data;

  if (loader_cdm_seek(prop) < 0)
    return -1;

  data = (cdm_data_t *)prop->data; // seek doesn't succeed if this would fail!

  l = fwrite(buf, 1, len, data->f);
  if (l != len)
    return -1;

  return l;
}

void
loader_cdm_init(void)
{
  libdisk_register_loader(&loader);
}
