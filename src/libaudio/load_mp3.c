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

#ifdef HAVE_LIBMP3LAME

#include <lame/lame.h>

#define FILE_BUF_SIZE (1024)

/*
 *  FIXME: don't know how to calculate a safe amount of
 *  FIXME: pcm buffer for the decoding routine
 *  FIXME: 32k samples seems to be enough for now...
 */
#define PCM_BUF_SIZE  (32 * 1024)

typedef struct mp3_data
{
  FILE          *f;
  int            eof; // 1: EOF in file, 2: EOF in decoded data
  int            pcm_idx;
  int            pcm_size;
  unsigned char  f_buf[FILE_BUF_SIZE];
  short          pcm_l[PCM_BUF_SIZE];
  short          pcm_r[PCM_BUF_SIZE];
  mp3data_struct mp3data;
} mp3_data_t;

static char loader_name[1024];
static libaudio_loader_t *self = NULL;

static int
loader_mp3_check(const char *filename, unsigned char *data, long size)
{
  if (size < 2)
    return 0;

  if (data[0] != 0xff)
    return 0;
  if ((data[1] & 0xe0) != 0xe0)
    return 0;

  return 1;
}

static libaudio_prop_t *
loader_mp3_open_prop(const char *filename, libaudio_prop_t *prop)
{
  FILE *f;
  int len, ret, ok;
  mp3_data_t *data;

  data = (mp3_data_t *)malloc(sizeof(mp3_data_t));
  if (data == NULL)
    return NULL;

  f = fopen(filename, "rb");
  if (f == NULL)
    {
      free(data);
      return NULL;
    }

  ok = 0;
  lame_decode_init();
  while (242)
    {
      len = fread(data->f_buf, 1, FILE_BUF_SIZE, f);
      ret = lame_decode1_headers(data->f_buf,
				 len,
				 data->pcm_l,
				 data->pcm_r,
				 &data->mp3data);

      if (len < FILE_BUF_SIZE)
	break;
      if (ret < 0)
	break;

      if (data->mp3data.header_parsed)
	{
	  ok = 1;
	  data->eof = 0;
	  data->pcm_idx = 0;
	  data->pcm_size = 2 * ret;
	  break;
	}
    }

  if (!ok)
    {
      fclose(f);
      free(data);
      return NULL;
    }

  data->f = f;

  prop->type = LIBAUDIO_TYPE_MP3;
  prop->loader = self;
  prop->loader_data = data;
  
  prop->sample_freq = data->mp3data.samplerate;
  prop->sample_size = 16;
  prop->channels = data->mp3data.stereo;

  return prop;
}

static libaudio_prop_t *
loader_mp3_open(const char *filename)
{
  libaudio_prop_t *prop, *ret;

  prop = (libaudio_prop_t *)malloc(sizeof(libaudio_prop_t));
  if (prop == NULL)
    return NULL;

  ret = loader_mp3_open_prop(filename, prop);
  if (ret == NULL)
    free(prop);

  return ret;
}

static void
loader_mp3_close(libaudio_prop_t *prop)
{
  mp3_data_t *data;

  assert(prop != NULL);
  assert(prop->type == LIBAUDIO_TYPE_MP3);
  assert(prop->loader_data != NULL);

  data = (mp3_data_t *)prop->loader_data;

  fclose(data->f);
  free(data);
}

static libaudio_prop_t *
loader_mp3_rewind(libaudio_prop_t *prop)
{
  assert(prop != NULL);
  assert(prop->type == LIBAUDIO_TYPE_MP3);
  assert(prop->loader_data != NULL);

  loader_mp3_close(prop);
  return loader_mp3_open_prop(prop->filename, prop);
}

static int
loader_mp3_read_sample(libaudio_prop_t *prop)
{
  int c, len, ret;
  mp3_data_t *data;

  assert(prop != NULL);
  assert(prop->type == LIBAUDIO_TYPE_MP3);
  assert(prop->loader_data != NULL);

  data = (mp3_data_t *)prop->loader_data;

  if (data->pcm_idx >= data->pcm_size)
    {
      if (data->eof > 0)
	{
	  data->eof = 2;
	  return EOF;
	}

      while (data->eof == 0)
	{
	  len = fread(data->f_buf, 1, FILE_BUF_SIZE, data->f);
	  ret = lame_decode(data->f_buf, len, data->pcm_l, data->pcm_r);

	  if (len < FILE_BUF_SIZE)
	    data->eof = 1;

	  if (ret > 0)
	    {
	      data->pcm_idx = 0;
	      data->pcm_size = 2 * ret;
	      break;
	    }

	  if (ret < 0)
	    {
	      data->eof = 2;
	      return EOF;
	    }
	}
    }

  if (data->pcm_idx < data->pcm_size)
    {
      if (data->pcm_idx % 1)
	c = data->pcm_r[data->pcm_idx / 2]; /* always 0 when mp3 file is mono */
      else
	c = data->pcm_l[data->pcm_idx / 2];

      data->pcm_idx++;
      if (data->mp3data.stereo == 1) /* skip second channel on mono files */
	data->pcm_idx++;

      return c + 32768;
    }
  
  return EOF;
}

static const char *
loader_mp3_get_type(void)
{
  return "MP3";
}

static const char *
loader_mp3_get_name(void)
{
  return loader_name;
}

static libaudio_loader_t loader = {
  loader_mp3_check,
  loader_mp3_open,
  loader_mp3_rewind,
  loader_mp3_close,
  loader_mp3_read_sample,
  loader_mp3_get_type,
  loader_mp3_get_name
};

void
loader_mp3_init(void)
{
  snprintf(loader_name,
	   sizeof(loader_name),
	   "loader for mp3 files (using libmp3lame %s)",
	   get_lame_short_version());

  if (libaudio_register_loader(&loader))
    self = &loader;
}

#else /* HAVE_LIBMP3LAME */

void
loader_mp3_init(void)
{
}

#endif /* HAVE_LIBMP3LAME */
