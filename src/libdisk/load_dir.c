/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: load_dir.c,v 1.2 2002/10/31 00:51:53 torsten_paul Exp $
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

#include <ctype.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "libdisk/libdiskP.h"


/*
 *  DISK PARAMETER
 */
#define BSH (4)                                                   // block shift
#define BLM (15)                                                  // block mask
#define DSM (3200)                                                // total number of (128 byte) disk records
#define DRM (128)                                                 // number of directory entries

#define DIR_ENTRY_SIZE (32)                                       // size of one directory entry
#define RECORD_SIZE    (128)                                      // CP/M record size is 128
#define DIR_SIZE       (DIR_ENTRY_SIZE * DRM)                     // directory size in bytes
#define AU_SIZE        (RECORD_SIZE << BSH)                       // size of allocation unit
#define SIZE_TO_AU(x)  (((x) + (AU_SIZE - 1)) / AU_SIZE)
#define SIZE_TO_REC(x) (((x) + (RECORD_SIZE - 1)) / RECORD_SIZE)

#define CHS_TO_SECT(c,h,s)      (((s) - 1) + 5 * (h) + 10 * (c))
#define CHS_TO_AU(c,h,s)        (CHS_TO_SECT(c,h,s) / 2)
#define AU_TO_OFFSET(au,c,h,s)  (CHS_TO_SECT(c,h,s) - 2 * (au))
#define FIRST_AU                (2)

/*
 *  $au = allocation unit
 *
 *  $a = 2 * $au;
 *  $s = ($a % 5) + 1;
 *  $h = ($a % 10) / 5;
 *  $c = ($a / 10);
 *
 *
 *  ($s - 1) + $h * 5 + $c * 10
 *
 */

typedef struct {
  char *path;
  char *filename[DRM];
  long size[DRM];
  long au[DRM];
  unsigned char dir[DIR_SIZE];
} dir_data_t;

static const char * loader_dir_get_name(void);
static int loader_dir_open(libdisk_prop_t *prop, const char *path);
static void loader_dir_close(libdisk_prop_t *prop);
static int loader_dir_read_sector(libdisk_prop_t *prop, unsigned char *buf, int len);
static int loader_dir_write_sector(libdisk_prop_t *prop, unsigned char *buf, int len);

static libdisk_loader_t loader = {
  loader_dir_get_name,
  loader_dir_open,
  loader_dir_close,
  loader_dir_read_sector,
  loader_dir_write_sector
};

static void
set_filename(char *buf, char *filename)
{
  int a;
  char *ptr;
  
  memset(buf, ' ', 11);

  for (a = 0;a < 8;a++)
    {
      if (filename[a] == '\0')
	break;
      if (filename[a] == '.')
	break;
      buf[a] = toupper(filename[a]);
    }

  ptr = strrchr(filename, '.');
  if (ptr == NULL)
    return;
  
  ptr++;
  for (a = 0;a < 3;a++)
    {
      if (ptr[a] == '\0')
	break;
      buf[8 + a] = toupper(ptr[a]);
    }
}

static int
set_directory_entry(dir_data_t *data, int idx, int start_au, long size, char *filename)
{
  int a;
  int au;
  int rec;
  int extend;

  au = 0;
  extend = 0;
  rec = SIZE_TO_REC(size);
  do
    {
      memset(&data->dir[32 * idx], 0, 32);

      //printf("%d: %s - %d\n", extend, filename, rec);
      set_filename(&data->dir[32 * idx + 1], filename);

      data->dir[32 * idx] = 0; // user
      data->dir[32 * idx + 12] = extend;
      data->dir[32 * idx + 13] = 0;
      data->dir[32 * idx + 14] = 0;
      data->dir[32 * idx + 15] = (rec > 0x80) ? 0x80 : rec;

      for (a = 0;a < 8;a++)
	if (au < SIZE_TO_AU(size))
	  {
	    data->dir[32 * idx + 2 * a + 16] = (start_au + au) & 0xff;
	    data->dir[32 * idx + 2 * a + 17] = (start_au + au) >> 8;
	    au++;
	  }

      rec -= 0x80;
      extend++;
      idx++;
    }
  while (rec > 0);

  return extend;
}

static const char *
loader_dir_get_name(void)
{
  return "disk loader for unix directories";
}

static int
loader_dir_open(libdisk_prop_t *prop, const char *path)
{
  DIR *d;
  int a, idx, ro, au;
  char pwd[4096]; // FIXME: fix getcwd()!
  dir_data_t *data;
  struct stat statbuf;
  struct dirent *dirent;
  
  if (prop == NULL)
    return -1;

  if (getcwd(pwd, 4096) == NULL)
    return -1;

  if (chdir(path) < 0)
    return -1;

  ro = 1;
  d = opendir(path);
  if (d == NULL)
    return -1;

  data = (dir_data_t *)malloc(sizeof(dir_data_t));
  if (data == NULL)
      return -1;

  data->path = strdup(path);

  memset(data->dir, 0xe5, DIR_SIZE);
  memset(data->au, 0, DRM * sizeof(long));
  memset(data->filename, 0, DRM * sizeof(char *));

  a = 0;
  while (242)
    {
      dirent = readdir(d);
      if (dirent == NULL)
	break;

      if (stat(dirent->d_name, &statbuf) < 0)
	continue;

      if (!S_ISREG(statbuf.st_mode))
	continue;

      data->size[a] = statbuf.st_size;
      data->filename[a] = strdup(dirent->d_name);
      a++;
    }
  
  closedir(d);

  if (chdir(pwd) < 0)
    printf("can't restore working directory!\n"); // FIXME:

  idx = 0;
  au = FIRST_AU;
  for (a = 0;(a < DRM) && (data->filename[a] != NULL);a++)
    {
      data->au[a] = au;
      idx += set_directory_entry(data,
				 idx,
				 au,
				 data->size[a],
				 data->filename[a]);
      
      au += SIZE_TO_AU(data->size[a]);
    }

#if 0
  {
    FILE *f;
    f = fopen("/tmp/dir", "wb");
    fwrite(data->dir, 1, DIR_SIZE, f);
    fclose(f);
  }
#endif
  
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
loader_dir_close(libdisk_prop_t *prop)
{
  dir_data_t *data;

  if (prop == NULL)
    return;

  if (prop->data == NULL)
    return;

  data = (dir_data_t *)prop->data;

  free(data);

  prop->data = NULL;
}

static int
read_block(dir_data_t *data, char *buf, int idx, long offset, int len)
{
  FILE *f;
  char pwd[4096]; // FIXME: fix getcwd()!

  //printf("read_block(): '%s' [%d]\n", data->filename[idx], offset);

  if (getcwd(pwd, 4096) == NULL)
    return -1;

  if (chdir(data->path) < 0)
    return -1;

  f = fopen(data->filename[idx], "rb");
  if (f == NULL)
    return -1;

  if (fseek(f, offset, SEEK_SET) < 0)
    return -1;

  memset(buf, '\0', len);
  fread(buf, 1, len, f);
  fclose(f);

  if (chdir(pwd) < 0)
    printf("can't restore working directory!\n"); // FIXME:

  return len;
}

static int
loader_dir_read_sector(libdisk_prop_t *prop, unsigned char *buf, int len)
{
  int h, c, s;
  dir_data_t *data;

  if (prop == NULL)
    return -1;

  if (prop->data == NULL)
    return -1;

  h = prop->head;
  c = prop->cylinder;
  s = prop->sector;

  data = (dir_data_t *)prop->data;

  if ((h == 0) && (c == 0) && (s < 5))
    {
      switch (s) {
      case 1:
	memcpy(buf, &data->dir[0], len);
	break;
      case 2:
	memcpy(buf, &data->dir[1024], len);
	break;
      case 3:
	memcpy(buf, &data->dir[2048], len);
	break;
      case 4:
	memcpy(buf, &data->dir[3072], len);
	break;
      case 5:
	memcpy(buf, &data->dir[4096], len);
	break;
      }
    }
  else
    {
      int a, au;
      au = CHS_TO_AU(c,h,s);

      for (a = DRM - 1;a >= 0;a--)
	{
	  if (data->au[a] == 0)
	    continue;
	  if (data->au[a] <= au)
	    break;
	}

      if (a < 0)
	{
	  printf("*** not found ***\n");
	  return -1;
	}

      au = AU_TO_OFFSET(data->au[a], c, h, s);
      return read_block(data, buf, a, 1024 * au, len);
    }

  return len;
}

static int
loader_dir_write_sector(libdisk_prop_t *prop, unsigned char *buf, int len)
{
  return 0;
}

void
loader_dir_init(void)
{
  libdisk_register_loader(&loader);
}
