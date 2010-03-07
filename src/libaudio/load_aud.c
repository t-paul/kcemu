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

#ifdef HAVE_LIBAUDIOFILE

#include <audiofile.h>

#define BUF_SIZE (1024)

static int af_error;
static char loader_name[1024];
static libaudio_loader_t *self = NULL;

typedef struct audio_data
{
  int eof;
  int idx;
  int size;
  int load_size;
  unsigned short buf[BUF_SIZE];
  AFfilehandle afFile;
} aud_data_t;

static void
loader_aud_error_func(long code, const char *str)
{
  af_error = 1;
}

static int
loader_aud_check(const char *filename, unsigned char *data, long size)
{
  AFerrfunc func;
  AFfilehandle file;

  af_error = 0;
  func = afSetErrorHandler(loader_aud_error_func);
  file = afOpenFile(filename, "r", AF_NULL_FILESETUP);
  afSetErrorHandler(func);

  if (file == AF_NULL_FILEHANDLE)
    return 0;

  if (af_error != 0)
    return 0;

  afCloseFile(file);
  return 1;
}

static libaudio_prop_t *
loader_aud_open_prop(const char *filename, libaudio_prop_t *prop)
{
  int sampfmt;
  int sampwidth;
  aud_data_t *data;

  data = (aud_data_t *)malloc(sizeof(aud_data_t));
  if (data == NULL)
    return NULL;

  data->afFile = afOpenFile(filename, "r", AF_NULL_FILESETUP);
  if (data->afFile == AF_NULL_FILEHANDLE)
    {
      free(data);
      return NULL;
    }

  /*
   *  set the virtual sample format to 16 bit unsigned; this is
   *  exactly the format required by libaudio_read_sample()
   */
  afGetSampleFormat(data->afFile, AF_DEFAULT_TRACK, &sampfmt, &sampwidth);
  if (afSetVirtualSampleFormat(data->afFile,
			       AF_DEFAULT_TRACK,
			       AF_SAMPFMT_UNSIGNED,
			       16) != 0)
    {
      afCloseFile(data->afFile);
      free(data);
      return NULL;
    }

  prop->type = LIBAUDIO_TYPE_AUD;
  prop->loader = self;
  prop->loader_data = data;

  prop->sample_freq = (int)afGetRate(data->afFile, AF_DEFAULT_TRACK);
  prop->sample_size = sampwidth;
  prop->channels = afGetChannels(data->afFile, AF_DEFAULT_TRACK);

  data->eof = 0;
  data->idx = 0;
  data->load_size = BUF_SIZE / prop->channels;
  data->size = data->load_size;

  return prop;
}

static libaudio_prop_t *
loader_aud_open(const char *filename)
{
  libaudio_prop_t *prop, *ret;

  prop = (libaudio_prop_t *)malloc(sizeof(libaudio_prop_t));
  if (prop == NULL)
    return NULL;

  ret = loader_aud_open_prop(filename, prop);
  if (ret == NULL)
    free(prop);

  return ret;
}

static void
loader_aud_close(libaudio_prop_t *prop)
{
  aud_data_t *data;

  assert(prop != NULL);
  assert(prop->type == LIBAUDIO_TYPE_AUD);
  assert(prop->loader_data != NULL);

  data = (aud_data_t *)prop->loader_data;
  afCloseFile(data->afFile);
  free(data);
}

static libaudio_prop_t *
loader_aud_rewind(libaudio_prop_t *prop)
{
  assert(prop != NULL);
  assert(prop->type == LIBAUDIO_TYPE_AUD);
  assert(prop->loader_data != NULL);

  loader_aud_close(prop);
  return loader_aud_open_prop(prop->filename, prop);
}

static int
loader_aud_read_sample(libaudio_prop_t *prop)
{
  int c, len;
  aud_data_t *data;

  assert(prop != NULL);
  assert(prop->type == LIBAUDIO_TYPE_AUD);
  assert(prop->loader_data != NULL);

  data = (aud_data_t *)prop->loader_data;

  if (data->eof)
    return EOF;

  if (data->idx >= data->size)
    {
      len = afReadFrames(data->afFile, AF_DEFAULT_TRACK, data->buf, data->load_size);
      if (len == 0)
	{
	  data->eof = 1;
	  return EOF;
	}
      data->idx = 0;
      data->size = len * prop->channels;
    }

  c = ((unsigned short *)data->buf)[data->idx];

  data->idx++;
  return c;
}

static const char *
loader_aud_get_type(void)
{
  return "AUDIOFILE";
}

static const char *
loader_aud_get_name(void)
{
  return loader_name;
}

static libaudio_loader_t loader = {
  loader_aud_check,
  loader_aud_open,
  loader_aud_rewind,
  loader_aud_close,
  loader_aud_read_sample,
  loader_aud_get_type,
  loader_aud_get_name
};

void
loader_aud_init(void)
{
  snprintf(loader_name,
	   sizeof(loader_name),
	   "loader for audio files supported by libaudiofile v%d.%d",
	   LIBAUDIOFILE_MAJOR_VERSION,
	   LIBAUDIOFILE_MINOR_VERSION);

  if (libaudio_register_loader(&loader))
    self = &loader;
}

#else /* HAVE_LIBAUDIOFILE */

void
loader_aud_init(void)
{
}

#endif /* HAVE_LIBAUDIOFILE */
