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
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "libdisk/libdiskP.h"

static libdisk_loader_t *loaders[MAX_DISK_LOADERS];

void
libdisk_init(void)
{
  memset(loaders, 0, sizeof(loaders));

  /*
   *  order is importend
   *  first check for types with unique signatures
   *  unsafe guesses come last
   */
  loader_dir_init();
  loader_cdm_init();
  loader_td0_init();
  loader_22dsk_init();
}

int
libdisk_register_loader(libdisk_loader_t *loader)
{
  int a;

  for (a = 0;a < MAX_DISK_LOADERS;a++)
    if (loaders[a] == NULL)
      {
        loaders[a] = loader;
        return 1;
      }
  return 0;
}

int
libdisk_open(libdisk_prop_t **prop, const char *path)
{
  int a;

  if (path == NULL)
    return -1;
  
  if (prop == NULL)
    return -1;

  *prop = (libdisk_prop_t *)malloc(sizeof(libdisk_prop_t));
  if (*prop == NULL)
    return -1;

  for (a = 0;a < MAX_DISK_LOADERS;a++)
    {
      if (loaders[a] == NULL)
	continue;

      if (loaders[a]->open(*prop, path) == 0)
	{
	  //printf("loader: %s\n", loaders[a]->get_name());
	  return 0;
	}
    }

  free(*prop);
  *prop = 0; // ensure pointer to be NULL on failure!
  
  return -1;
}

void
libdisk_close(libdisk_prop_t **prop)
{
  libdisk_prop_t *p;

  if (prop == NULL)
    return;

  p = (*prop);
  if (p == NULL)
    return;

  if (p->loader != NULL)
    if (p->loader->close != NULL)
      p->loader->close(p);

  free(p);
  *prop = 0;
}

int
libdisk_seek(libdisk_prop_t **prop, int head, int cylinder, int sector)
{
  libdisk_prop_t *p;

  if (prop == NULL)
    return -1;

  p = (*prop);
  p->head = head;
  p->cylinder = cylinder;
  p->sector = sector;

  return 0;
}

int
libdisk_read_sector(libdisk_prop_t **prop, unsigned char *buf, int len)
{
  libdisk_prop_t *p;

  if (prop == NULL)
    return -1;

  p = (*prop);
  if (p)
    if (p->loader != NULL)
      if (p->loader->read_sector != NULL)
	return p->loader->read_sector(p, buf, len);

  return 0;
}

int
libdisk_write_sector(libdisk_prop_t **prop, unsigned char *buf, int len)
{
  libdisk_prop_t *p;

  if (prop == NULL)
    return -1;

  p = (*prop);
  if (p)
    if (p->loader != NULL)
      if (p->loader->write_sector != NULL)
	return p->loader->write_sector(p, buf, len);
  
  return 0;  
}

void
libdisk_show_config(void)
{
  int a;

  printf("available libdisk plugins:\n");
  for (a = 0;a < MAX_DISK_LOADERS;a++)
    {
      if (loaders[a])
	printf("  %s\n", loaders[a]->get_name());
    }
}
