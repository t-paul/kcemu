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
#include <assert.h>

#include "libaudio/libaudioP.h"

#ifdef HAVE_LIBVORBIS

#include <vorbis/vorbisfile.h>

static char loader_name[1024];
static libaudio_loader_t *self = NULL;

#define BUF_SIZE (4096)

typedef struct ogg_data
{
  int eof;
  int idx;
  int size;
  unsigned char buf[BUF_SIZE];
  OggVorbis_File file;
} ogg_data_t;

static int
loader_ogg_check(const char *filename, unsigned char *data, long size)
{
  FILE *f;
  OggVorbis_File file;

  f = fopen(filename, "rb");
  if (f == NULL)
    return 0;

  if (ov_test(f, &file, NULL, 0) != 0)
    {
      fclose(f);
      return 0;
    }

  ov_clear(&file);

  return 1;
}

static libaudio_prop_t *
loader_ogg_open_prop(const char *filename, libaudio_prop_t *prop)
{
  FILE *f;
  ogg_data_t *data;
  vorbis_info *info;

  data = (ogg_data_t *)malloc(sizeof(ogg_data_t));
  if (data == NULL)
    return NULL;

  f = fopen(filename, "rb");
  if (f == NULL)
    {
      free(data);
      return NULL;
    }

  if (ov_open(f, &data->file, NULL, 0) != 0)
    {
      free(data);
      fclose(f);
      return NULL;
    }

  data->eof = 0;
  data->idx = 0;
  data->size = 0;

  prop->type = LIBAUDIO_TYPE_OGG;
  prop->loader = self;
  prop->loader_data = data;

  info = ov_info(&data->file, -1);

  prop->sample_freq = info->rate;
  prop->sample_size = 16;
  prop->channels = info->channels;

  return prop;
}

static libaudio_prop_t *
loader_ogg_open(const char *filename)
{
  libaudio_prop_t *prop, *ret;

  prop = (libaudio_prop_t *)malloc(sizeof(libaudio_prop_t));
  if (prop == NULL)
    return NULL;

  ret = loader_ogg_open_prop(filename, prop);
  if (ret == NULL)
    free(prop);

  return ret;
}

static void
loader_ogg_close(libaudio_prop_t *prop)
{
  ogg_data_t *data;

  assert(prop != NULL);
  assert(prop->type == LIBAUDIO_TYPE_OGG);
  assert(prop->loader_data != NULL);

  data = (ogg_data_t *)prop->loader_data;

  ov_clear(&data->file);
  free(data);
}

static libaudio_prop_t *
loader_ogg_rewind(libaudio_prop_t *prop)
{
  assert(prop != NULL);
  assert(prop->type == LIBAUDIO_TYPE_OGG);
  assert(prop->loader_data != NULL);

  loader_ogg_close(prop);
  return loader_ogg_open_prop(prop->filename, prop);
}

static int
loader_ogg_read_sample(libaudio_prop_t *prop)
{
  int c, len;
  int bitstream;
  ogg_data_t *data;

  assert(prop != NULL);
  assert(prop->type == LIBAUDIO_TYPE_OGG);
  assert(prop->loader_data != NULL);

  data = (ogg_data_t *)prop->loader_data;

  if (data->eof)
    return EOF;

  if (data->idx >= data->size)
    {
      len = ov_read(&data->file, (char *)data->buf, BUF_SIZE, 0, 2, 1, &bitstream);
      if (len <= 0)
	{
	  data->eof = 1;
	  return EOF;
	}

      data->idx = 0;
      data->size = len;
    }

  c = 256 * data->buf[data->idx + 1] + data->buf[data->idx];
  data->idx += 2;

  return 32768 + c;
}

static const char *
loader_ogg_get_type(void)
{
  return "OggVorbis";
}

static const char *
loader_ogg_get_name(void)
{
  return loader_name;
}

static libaudio_loader_t loader = {
  loader_ogg_check,
  loader_ogg_open,
  loader_ogg_rewind,
  loader_ogg_close,
  loader_ogg_read_sample,
  loader_ogg_get_type,
  loader_ogg_get_name
};

void
loader_ogg_init(void)
{
  snprintf(loader_name,
	   sizeof(loader_name),
	   "loader for OggVorbis audio files");

  if (libaudio_register_loader(&loader))
    self = &loader;
}

#else /* HAVE_LIBVORBIS */

void
loader_ogg_init(void)
{
}

#endif /* HAVE_LIBVORBIS */
