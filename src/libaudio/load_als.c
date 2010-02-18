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

#include "libaudio/libaudioP.h"

#ifdef HAVE_LIBASOUND

#include <alsa/asoundlib.h>

#define ALS_BUF_SIZE (1024)

static char loader_name[1024];
static libaudio_loader_t *self = NULL;

typedef struct als_data
{  
  int eof;
  int idx;
  snd_pcm_t *capture_handle;
  unsigned char buf[ALS_BUF_SIZE];
} als_data_t;

static int
loader_als_check(const char *filename, unsigned char *data, long size)
{
  if (strcmp(filename, "/dev/snd") == 0)
    return 1;

  return 0;
}

static libaudio_prop_t *
loader_als_open(const char *filename)
{
  int err;
  als_data_t *data;
  libaudio_prop_t *prop;
  snd_pcm_hw_params_t *hw_params;

  prop = (libaudio_prop_t *)malloc(sizeof(libaudio_prop_t));
  if (prop == NULL)
    return NULL;

  data = (als_data_t *)malloc(sizeof(als_data_t));
  if (data == NULL)
    {
      free(prop);
      return NULL;
    }

  data->eof = 0;
  data->idx = 0;

  err = snd_pcm_open(&data->capture_handle, "plughw:0,0", SND_PCM_STREAM_CAPTURE, 0);
  if (err < 0)
    return NULL;

  err = snd_pcm_hw_params_malloc(&hw_params);
  if (err < 0)
    return NULL;

  err = snd_pcm_hw_params_any(data->capture_handle, hw_params);
  if (err < 0)
    return NULL;
  
  err = snd_pcm_hw_params_set_access(data->capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
  if (err < 0)
    return NULL;

  err = snd_pcm_hw_params_set_format(data->capture_handle, hw_params, SND_PCM_FORMAT_S8);
  if (err < 0)
    return NULL;

  err = snd_pcm_hw_params_set_rate_near(data->capture_handle, hw_params, 44100, 0);
  if (err < 0)
    return NULL;

  err = snd_pcm_hw_params_set_channels(data->capture_handle, hw_params, 1);
  if (err < 0)
    return NULL;

  err = snd_pcm_hw_params(data->capture_handle, hw_params);
  if (err < 0)
    return NULL;

  snd_pcm_hw_params_free(hw_params);

  err = snd_pcm_prepare(data->capture_handle);
  if (err < 0)
    return NULL;

  prop->type = LIBAUDIO_TYPE_ALS;
  prop->loader = self;
  prop->loader_data = data;

  prop->sample_freq = 44100;
  prop->sample_size = 8;
  prop->channels = 1;

  return prop;
}

static libaudio_prop_t *
loader_als_rewind(libaudio_prop_t *prop)
{
  assert(prop != NULL);
  assert(prop->type == LIBAUDIO_TYPE_ALS);
  assert(prop->loader_data != NULL);

  /*
   *  can't do anything sensible here, just checking if called
   *  on the appropriate stream...
   */

  return prop;
}

static void
loader_als_close(libaudio_prop_t *prop)
{
  als_data_t *data;

  assert(prop != NULL);
  assert(prop->type == LIBAUDIO_TYPE_ALS);
  assert(prop->loader_data != NULL);

  data = (als_data_t *)(prop->loader_data);

  snd_pcm_close(data->capture_handle);

  free(data);
}

static int
loader_als_read_sample(libaudio_prop_t *prop)
{
  int c, err;
  als_data_t *data;

  assert(prop != NULL);
  assert(prop->type == LIBAUDIO_TYPE_ALS);
  assert(prop->loader_data != NULL);

  data = (als_data_t *)(prop->loader_data);

  if (data->eof)
    return EOF;

  if (data->idx >= ALS_BUF_SIZE)
    {
      err = snd_pcm_readi(data->capture_handle, data->buf, ALS_BUF_SIZE);
      if (err < 0)
	{
	  snd_pcm_prepare(data->capture_handle);
	  fprintf(stderr, "audio error (%s)\n", snd_strerror(err));
	}

      //if (snd_pcm_readi(data->capture_handle, data->buf, ALS_BUF_SIZE) != ALS_BUF_SIZE)
	{
	  //data->eof = 1;
	  //return EOF;
	}
      data->idx = 0;
    }

  c = 256 * data->buf[data->idx];
  data->idx++;

  return c;
}

static const char *
loader_als_get_type(void)
{
  return "ALSA";
}

static const char *
loader_als_get_name(void)
{
  return loader_name;
}

static libaudio_loader_t loader = {
  loader_als_check,
  loader_als_open,
  loader_als_rewind,
  loader_als_close,
  loader_als_read_sample,
  loader_als_get_type,
  loader_als_get_name
};

void
loader_als_init(void)
{
  snprintf(loader_name,
	   sizeof(loader_name),
	   "loader for ALSA line input (/dev/snd)");

  if (libaudio_register_loader(&loader))
    self = &loader;
}

#else /* HAVE_LIBASOUND */

void
loader_als_init(void)
{
}

#endif /* HAVE_LIBASOUND */
