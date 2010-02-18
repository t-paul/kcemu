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

#ifdef HAVE_LIBSNDFILE

#include <sndfile.h>

#define MAX_CHANNELS (16)

static char loader_name[1024];
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

#if HAVE_LIBSNDFILE_V0
  sndfile = sf_open_read(filename, &info);
#else /* HAVE_LIBSNDFILE_V0 */
  sndfile = sf_open(filename, SFM_READ, &info);
#endif /* HAVE_LIBSNDFILE_V0 */
  if (sndfile == NULL)
    return 0;

#if HAVE_LIBSNDFILE_V0
  if ((info.pcmbitwidth != 8) && (info.pcmbitwidth != 16))
    return 0;
#else /* HAVE_LIBSNDFILE_V0 */
  switch (info.format & SF_FORMAT_SUBMASK)
    {
    case SF_FORMAT_PCM_U8:
    case SF_FORMAT_PCM_S8:
    case SF_FORMAT_PCM_16:
      break;
    default:
      return 0;
    }
#endif /* HAVE_LIBSNDFILE_V0 */

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

#if HAVE_LIBSNDFILE_V0
  sndfile = sf_open_read(filename, &info);
#else /* HAVE_LIBSNDFILE_V0 */
  sndfile = sf_open(filename, SFM_READ, &info);
#endif /* HAVE_LIBSNDFILE_V0 */
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
#if HAVE_LIBSNDFILE_V0
  prop->sample_size = info.pcmbitwidth; /* hmm, deprecated but quite useful... */
#else /* HAVE_LIBSNDFILE_V0 */
  switch (info.format & SF_FORMAT_SUBMASK)
    {
    case SF_FORMAT_PCM_U8:
    case SF_FORMAT_PCM_S8:
      prop->sample_size = 8;
      break;
    case SF_FORMAT_PCM_16:
      prop->sample_size = 16;
      break;
    default:
      /* should not be reached; blocked by loader_snd_check() */
      return NULL;
    }
#endif /* HAVE_LIBSNDFILE_V0 */

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
  return loader_name;
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
  char buffer[1024];

#if HAVE_LIBSNDFILE_V0
  sf_get_lib_version(buffer, sizeof(buffer));
#else /* HAVE_LIBSNDFILE_V0 */
  sf_command(NULL, SFC_GET_LIB_VERSION, buffer, sizeof(buffer));
#endif /* HAVE_LIBSNDFILE_V0 */

  snprintf(loader_name,
	   sizeof(loader_name),
	   "loader for audio files supported by %s",
	   buffer);

  if (libaudio_register_loader(&loader))
    self = &loader;
}

#else /* HAVE_LIBSNDFILE */

void
loader_snd_init(void)
{
}

#endif /* HAVE_LIBSNDFILE */
