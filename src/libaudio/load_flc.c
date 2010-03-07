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

#ifdef HAVE_LIBFLAC

#include <FLAC/file_decoder.h>

static char loader_name[1024];
static libaudio_loader_t *self = NULL;

typedef struct flc_data
{
  int eof;
  int idx;
  int size;
  const FLAC__int32 *buf_l;
  const FLAC__int32 *buf_r;
  FLAC__FileDecoder *decoder;
  unsigned int sample_rate;
  unsigned int channels;
  unsigned int bits_per_sample;
} flc_data_t;

static int
loader_flc_check(const char *filename, unsigned char *data, long size)
{
  if (size < 4)
    return 0;

  if (data[0] != 0x66)
    return 0;
  if (data[1] != 0x4c)
    return 0;
  if (data[2] != 0x61)
    return 0;
  if (data[3] != 0x43)
    return 0;

  return 1;
}

static FLAC__StreamDecoderWriteStatus
flac_write_callback(const FLAC__FileDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data)
{
  flc_data_t *data = (flc_data_t *)client_data;

  data->size = frame->header.blocksize;
  data->buf_l = buffer[0];
  if (data->channels >= 2)
    data->buf_r = buffer[1];

  return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static void
flac_metadata_callback(const FLAC__FileDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
{
  flc_data_t *data = (flc_data_t *)client_data;

  if (metadata->type != FLAC__METADATA_TYPE_STREAMINFO)
    return;

  data->sample_rate = metadata->data.stream_info.sample_rate;
  data->channels = metadata->data.stream_info.channels;
  data->bits_per_sample = metadata->data.stream_info.bits_per_sample;
}

static void
flac_error_callback(const FLAC__FileDecoder *decoder, const FLAC__StreamDecoderErrorStatus status, void *client_data)
{
  flc_data_t *data = (flc_data_t *)client_data;

  data->eof = 1;
}

static FLAC__bool
flac_init(FLAC__FileDecoder *decoder, flc_data_t *data, const char *filename)
{
  if (!FLAC__file_decoder_set_md5_checking(decoder, 0))
    return 0;
  if (!FLAC__file_decoder_set_filename(decoder, filename))
    return 0;
  if (!FLAC__file_decoder_set_client_data(decoder, data))
    return 0;
  if (!FLAC__file_decoder_set_write_callback(decoder, flac_write_callback))
    return 0;
  if (!FLAC__file_decoder_set_metadata_callback(decoder, flac_metadata_callback))
    return 0;
  if (!FLAC__file_decoder_set_error_callback(decoder, flac_error_callback))
    return 0;

  return 1;
}

static libaudio_prop_t *
loader_flc_open_prop(const char *filename, libaudio_prop_t *prop)
{
  flc_data_t *data;
  FLAC__FileDecoder *decoder;

  data = (flc_data_t *)malloc(sizeof(flc_data_t));
  if (data == NULL)
    return NULL;

  decoder = FLAC__file_decoder_new();
  if (decoder == NULL)
    return NULL;

  if (!flac_init(decoder, data, filename))
    {
      FLAC__file_decoder_delete(decoder);
      free(data);
      return NULL;
    }

  if (FLAC__file_decoder_init(decoder) != FLAC__FILE_DECODER_OK)
    {
      FLAC__file_decoder_delete(decoder);
      free(data);
      return NULL;
    }

  /*
   *  The metadata callback should fill the sample_rate, channels
   *  and bits_per_sample values in our data structure.
   */
  data->sample_rate = 0;
  if (!FLAC__file_decoder_process_metadata(decoder))
    {
      FLAC__file_decoder_delete(decoder);
      free(data);
      return NULL;
    }

  if (data->sample_rate == 0)
    {
      /*
       *  We got no stream info while processing the metadata, odd.
       */
      FLAC__file_decoder_delete(decoder);
      free(data);
      return NULL;
    }

  data->eof = 0;
  data->idx = 0;
  data->size = 0;
  data->decoder = decoder;

  prop->type = LIBAUDIO_TYPE_FLC;
  prop->loader = self;
  prop->loader_data = data;

  prop->sample_freq = data->sample_rate;
  prop->sample_size = data->bits_per_sample;
  prop->channels = data->channels;

  return prop;
}

static libaudio_prop_t *
loader_flc_open(const char *filename)
{
  libaudio_prop_t *prop, *ret;

  prop = (libaudio_prop_t *)malloc(sizeof(libaudio_prop_t));
  if (prop == NULL)
    return NULL;

  ret = loader_flc_open_prop(filename, prop);
  if (ret == NULL)
    free(prop);

  return ret;
}

static void
loader_flc_close(libaudio_prop_t *prop)
{
  flc_data_t *data;

  assert(prop != NULL);
  assert(prop->type == LIBAUDIO_TYPE_FLC);
  assert(prop->loader_data != NULL);
  
  data = (flc_data_t *)prop->loader_data;
  FLAC__file_decoder_delete(data->decoder);
  free(data);
}

static libaudio_prop_t *
loader_flc_rewind(libaudio_prop_t *prop)
{
  flc_data_t *data;

  assert(prop != NULL);
  assert(prop->type == LIBAUDIO_TYPE_FLC);
  assert(prop->loader_data != NULL);

  data = (flc_data_t *)prop->loader_data;
  FLAC__file_decoder_seek_absolute(data->decoder, 0);

  data->eof = 0;
  data->idx = 0;
  data->size = 0;

  return prop;
}

static int
loader_flc_read_sample(libaudio_prop_t *prop)
{
  int c;
  flc_data_t *data;

  assert(prop != NULL);
  assert(prop->type == LIBAUDIO_TYPE_FLC);
  assert(prop->loader_data != NULL);

  data = (flc_data_t *)prop->loader_data;

  if (data->eof)
    return EOF;

  if (data->idx >= data->size)
    {
      /*
       *  Let the FLAC library process one frame of the audio data
       *  and call the flac_write_callback() which fills the
       *  buffer and the size variable.
       */
      data->size = 0;
      FLAC__file_decoder_process_one_frame(data->decoder);
      if (data->size == 0)
	{
	  data->eof = 1;
	  return EOF;
	}
      data->idx = 0;
      if (prop->channels >= 2)
	data->size *= 2;
    }

  if (prop->channels >= 2)
    {
      if ((data->idx & 1) == 0)
	c = data->buf_l[data->idx / 2];
      else
	c = data->buf_r[data->idx / 2];
    }
  else
    c = data->buf_l[data->idx];

  data->idx++;

  if (prop->sample_size == 8)
    return 256 * (128 + c);

  if (prop->sample_size == 16)
    return 32768 + c;

  return EOF;
}

static const char *
loader_flc_get_type(void)
{
  return "FLAC";
}

static const char *
loader_flc_get_name(void)
{
  return loader_name;
}

static libaudio_loader_t loader = {
  loader_flc_check,
  loader_flc_open,
  loader_flc_rewind,
  loader_flc_close,
  loader_flc_read_sample,
  loader_flc_get_type,
  loader_flc_get_name
};

void
loader_flc_init(void)
{
  snprintf(loader_name,
	   sizeof(loader_name),
	   "loader for FLAC compressed audio files");

  if (libaudio_register_loader(&loader))
    self = &loader;
}

#else /* HAVE_LIBFLAC */

void
loader_flc_init(void)
{
}

#endif /* HAVE_LIBFLAC */
