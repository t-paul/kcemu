/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2002 Torsten Paul
 *
 *  $Id: load_snd.c,v 1.1 2002/10/31 00:57:22 torsten_paul Exp $
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "libaudio/libaudioP.h"

#ifdef HAVE_LIBSNDFILE

#include <sndfile.h>

#define MAX_CHANNELS (16)

static libaudio_loader_t *self = NULL;

typedef struct snd_data
{
  int eof;
  int idx;
  int size;
  short buf[MAX_CHANNELS];
  SNDFILE *sndfile;
} snd_data_t;

static int
loader_snd_check(const char *filename, unsigned char *data, long size)
{
  SF_INFO info;
  SNDFILE *sndfile;

  sndfile = sf_open_read(filename, &info);
  if (sndfile == NULL)
    return 0;

  if (info.channels > MAX_CHANNELS)
    return 0;

  sf_close(sndfile);
  return 1;
}

static libaudio_prop_t *
loader_snd_open_prop(const char *filename, libaudio_prop_t *prop)
{
  SF_INFO info;
  SNDFILE *sndfile;
  snd_data_t *data;

  data = (snd_data_t *)malloc(sizeof(snd_data_t));
  if (data == NULL)
    return NULL;

  sndfile = sf_open_read(filename, &info);
  if (sndfile == NULL)
    {
      free(data);
      return NULL;
    }

  data->eof = 0;
  data->idx = 0;
  data->size = 0;
  data->sndfile = sndfile;

  prop->type = LIBAUDIO_TYPE_SND;
  prop->loader = self;
  prop->loader_data = data;

  prop->sample_freq = info.samplerate;
  prop->sample_size = info.pcmbitwidth; /* hmm, deprecated but quite useful... */
  prop->channels = info.channels;

  return prop;
}

static libaudio_prop_t *
loader_snd_open(const char *filename)
{
  libaudio_prop_t *prop, *ret;

  prop = (libaudio_prop_t *)malloc(sizeof(libaudio_prop_t));
  if (prop == NULL)
    return NULL;

  ret = loader_snd_open_prop(filename, prop);
  if (ret == NULL)
    free(prop);

  return ret;
}

static void
loader_snd_close(libaudio_prop_t *prop)
{
  snd_data_t *data;

  assert(prop != NULL);
  assert(prop->type == LIBAUDIO_TYPE_SND);
  assert(prop->loader_data != NULL);
  
  data = (snd_data_t *)prop->loader_data;
  sf_close(data->sndfile);
  free(data);
}

static libaudio_prop_t *
loader_snd_rewind(libaudio_prop_t *prop)
{
  assert(prop != NULL);
  assert(prop->type == LIBAUDIO_TYPE_SND);
  assert(prop->loader_data != NULL);

  loader_snd_close(prop);
  return loader_snd_open_prop(prop->filename, prop);
}

static int
loader_snd_read_sample(libaudio_prop_t *prop)
{
  int c, len;
  snd_data_t *data;

  assert(prop != NULL);
  assert(prop->type == LIBAUDIO_TYPE_SND);
  assert(prop->loader_data != NULL);

  data = (snd_data_t *)prop->loader_data;

  if (data->eof)
    return EOF;

  if (data->idx >= data->size)
    {
      len = sf_readf_short(data->sndfile, data->buf, 1);
      if (len == 0)
	{
	  data->eof = 1;
	  return EOF;
	}

      data->idx = 0;
      data->size = prop->channels;
    }

  c = data->buf[data->idx];
  data->idx++;

  if (prop->sample_size == 8)
    return 256 * (128 + c);
  
  if (prop->sample_size == 16)
    return 32768 + c;

  return EOF;
}

static const char *
loader_snd_get_type(void)
{
  return "SNDFILE";
}

static const char *
loader_snd_get_name(void)
{
  return "loader for audio files supported by libsndfile";
}

static libaudio_loader_t loader = {
  loader_snd_check,
  loader_snd_open,
  loader_snd_rewind,
  loader_snd_close,
  loader_snd_read_sample,
  loader_snd_get_type,
  loader_snd_get_name
};

void
loader_snd_init(void)
{
  if (libaudio_register_loader(&loader))
    self = &loader;
}

#else /* HAVE_LIBSNDFILE */

void
loader_snd_init(void)
{
}

#endif /* HAVE_LIBSNDFILE */
