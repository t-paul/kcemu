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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libdisk/teledisk.h"
#include "libdisk/libdiskP.h"

static const char * loader_td0_get_name(void);
static int loader_td0_open(libdisk_prop_t *prop, const char *path);
static void loader_td0_close(libdisk_prop_t *prop);
static int loader_td0_read_sector(libdisk_prop_t *prop, unsigned char *buf, int len);
static int loader_td0_write_sector(libdisk_prop_t *prop, unsigned char *buf, int len);

static libdisk_loader_t loader = {
  loader_td0_get_name,
  loader_td0_open,
  loader_td0_close,
  loader_td0_read_sector,
  loader_td0_write_sector
};

static const char *
loader_td0_get_name(void)
{
  return "disk loader for Sydex TeleDisk images";
}

static int
loader_td0_open(libdisk_prop_t *prop, const char *path)
{
  teledisk_prop_t *data;

  if (prop == NULL)
    return -1;

  data = teledisk_open(path);
  if (data == NULL)
    return -1;

  prop->head_count = 2;
  prop->cylinder_count = 80;
  prop->sector_size = 1024;
  prop->sectors_per_cylinder = 5;
  prop->data = data;
  prop->loader = &loader;
  prop->read_only = 1;

  return 0;
}

static void
loader_td0_close(libdisk_prop_t *prop)
{
  teledisk_prop_t *data;

  if (prop == NULL)
    return;

  if (prop->data == NULL)
    return;

  data = (teledisk_prop_t *)prop->data;
  teledisk_close(data);

  prop->data = NULL;
}

static int
loader_td0_read_sector(libdisk_prop_t *prop, unsigned char *buf, int len)
{
  teledisk_prop_t *data;

  if (prop == NULL)
    return -1;

  if (prop->data == NULL)
    return -1;

  if (len != 1024)
    return -1;

  data = (teledisk_prop_t *)prop->data;
  if (teledisk_read_sector(data, prop->cylinder, prop->head, prop->sector) < 0)
    return -1;

  memcpy(buf, data->buf, len);
  return len;
}

static int
loader_td0_write_sector(libdisk_prop_t *prop, unsigned char *buf, int len)
{
  return -1;
}

void
loader_td0_init(void)
{
  libdisk_register_loader(&loader);
}
