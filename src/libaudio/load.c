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
#include <assert.h>
#include <sys/stat.h>

#include "libaudio/libaudioP.h"

static libaudio_loader_t *loaders[MAX_LIBAUDIO_LOADERS];

void
libaudio_init(int l)
{
  memset(loaders, 0, sizeof(loaders));

  /*
   *  order is importend
   *  first check for types with unique signatures
   *  unsafe guesses come last
   */
  if ((l & LIBAUDIO_TYPE_OSS) != 0)
    loader_oss_init();
  if ((l & LIBAUDIO_TYPE_ALS) != 0)
    loader_als_init();
  if ((l & LIBAUDIO_TYPE_SND) != 0)
    loader_snd_init();
  if ((l & LIBAUDIO_TYPE_AUD) != 0)
    loader_aud_init();
  if ((l & LIBAUDIO_TYPE_FLC) != 0)
    loader_flc_init();
  if ((l & LIBAUDIO_TYPE_WAV) != 0)
    loader_wav_init();
  if ((l & LIBAUDIO_TYPE_OGG) != 0)
    loader_ogg_init();
  if ((l & LIBAUDIO_TYPE_MP3) != 0)
    loader_mp3_init();
  if ((l & LIBAUDIO_TYPE_UNKNOWN) != 0)
    loader_raw_init(); /* fallback */
}

int
libaudio_register_loader(libaudio_loader_t *loader)
{
  int a;

  for (a = 0;a < MAX_LIBAUDIO_LOADERS;a++)
    if (loaders[a] == NULL)
      {
        loaders[a] = loader;
        return 1;
      }
  return 0;
}

void
libaudio_free_prop(libaudio_prop_t *prop)
{
  free(prop->filename);
  free(prop);
}

libaudio_prop_t *
libaudio_open(const char *filename)
{
  FILE *f;
  int a;
  long size;
  unsigned char *data;
  struct stat stat_buf;
  libaudio_prop_t *retval;

  size = 0;
  data = NULL;

  if (filename == NULL)
    return NULL;
  
  if (stat(filename, &stat_buf) != 0)
    return NULL;

  if (S_ISREG(stat_buf.st_mode))
    {
      size = 65536;
      if (stat_buf.st_size < size)
	size = stat_buf.st_size;

      /* prevent malloc(0) */
      if (size == 0)
	size++;

      data = (unsigned char *)malloc(size);
      if (data == 0)
	return NULL;
      
      f = fopen(filename, "rb");
      if (f == NULL)
	{
	  free(data);
	  return NULL;
	}
      
      if (fread(data, 1, size, f) != size)
	{
	  free(data);
	  return NULL;
	}
      
      fclose(f);
    }

  retval = NULL;
  for (a = 0;a < MAX_LIBAUDIO_LOADERS;a++)
    {
      if (loaders[a])
        if (loaders[a]->check)
          {
	    /* printf("trying %s...\n", loaders[a]->get_name()); */
            if (loaders[a]->check(filename, data, size))
              {
	        /* printf("using %s.\n", loaders[a]->get_name()); */
                retval = loaders[a]->open(filename);
                break;
              }
          }
    }

  if (data)
    free(data);

  if (retval != NULL)
    {
      retval->filename = (char *)malloc(strlen(filename) + 1);
      strcpy(retval->filename, filename);
    }

  return retval;
}

void
libaudio_rewind(libaudio_prop_t *prop)
{
  libaudio_prop_t *ret;

  if (prop == NULL)
    return;

  if (prop->loader == NULL)
    return;

  ret = prop->loader->rewind(prop);
  if (ret == NULL)
    libaudio_free_prop(prop);
}

void
libaudio_close(libaudio_prop_t *prop)
{
  if (prop == NULL)
    return;

  if (prop->loader == NULL)
    return;

  prop->loader->close(prop);
  libaudio_free_prop(prop);
}

int
libaudio_read_sample(libaudio_prop_t *prop)
{
  if (prop == NULL)
    return EOF;

  if (prop->loader == NULL)
    return EOF;

  return prop->loader->read_sample(prop);
}

const char *
libaudio_get_type(libaudio_prop_t *prop)
{
  if (prop == NULL)
    return "?";

  if (prop->loader == NULL)
    return "?";

  return prop->loader->get_type();
}

const char *
libaudio_get_name(libaudio_prop_t *prop)
{
  if (prop == NULL)
    return "?";

  if (prop->loader == NULL)
    return "?";

  return prop->loader->get_name();
}

const char *
libaudio_get_filename(libaudio_prop_t *prop)
{
  if (prop == NULL)
    return "";

  return prop->filename;
}

int
libaudio_get_sample_freq(libaudio_prop_t *prop)
{
  assert(prop != NULL);

  return prop->sample_freq;
}

int
libaudio_get_sample_size(libaudio_prop_t *prop)
{
  assert(prop != NULL);

  return prop->sample_size;
}

int
libaudio_get_channels(libaudio_prop_t *prop)
{
  assert(prop != NULL);

  return prop->channels;
}

void
libaudio_show_config(void)
{
  int a;

  printf("available libaudio plugins:\n");
  for (a = 0;a < MAX_LIBAUDIO_LOADERS;a++)
    {
      if (loaders[a])
	printf("  %-10s %s\n", loaders[a]->get_type(), loaders[a]->get_name());
    }
}
