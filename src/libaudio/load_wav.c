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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "libaudio/libaudioP.h"

typedef struct wav_header
{
  uint32_t MainChunkID;                      // it will be 'RIFF'
  uint32_t Length;
  uint32_t ChunkTypeID;                      // it will be 'WAVE'
  uint32_t SubChunkID;                       // it will be 'fmt '
  uint32_t SubChunkLength;
  uint16_t AudioFormat;
  uint16_t NumChannels;
  uint32_t SampleFreq;
  uint32_t BytesPerSec;
  uint16_t BytesPerSample;
  uint16_t BitsPerSample;
  uint32_t DataChunkID;                      // it will be 'data'
  uint32_t DataLength;
} wav_header_t;

static char loader_name[1024];
static libaudio_loader_t *self = NULL;

static int
loader_wav_check(const char *filename, unsigned char *data, long size)
{
  if (data == NULL)
    return 0;
  if (size < 44)
    return 0;
  
  if (memcmp(&data[0], "RIFF", 4) != 0)
    return 0;
  if (memcmp(&data[8], "WAVE", 4) != 0)
    return 0;
  if (memcmp(&data[12], "fmt ", 4) != 0)
    return 0;

  return 1;
}

static libaudio_prop_t *
loader_wav_open_prop(const char *filename, libaudio_prop_t *prop)
{
  FILE *f;
  wav_header_t wav_header;

  f = fopen(filename, "rb");
  if (f == NULL)
    return NULL;

  if (fread(&wav_header, 1, sizeof(wav_header_t), f) != sizeof(wav_header_t))
    return NULL;

  prop->type = LIBAUDIO_TYPE_WAV;
  prop->loader = self;
  prop->loader_data = f;

  prop->sample_freq = wav_header.SampleFreq;
  prop->sample_size = wav_header.BitsPerSample;
  prop->channels = wav_header.NumChannels;

  return prop;
}

static libaudio_prop_t *
loader_wav_open(const char *filename)
{
  libaudio_prop_t *prop, *ret;

  prop = (libaudio_prop_t *)malloc(sizeof(libaudio_prop_t));
  if (prop == NULL)
    return NULL;

  ret = loader_wav_open_prop(filename, prop);
  if (ret == NULL)
    free(prop);

  return ret;
}

static void
loader_wav_close(libaudio_prop_t *prop)
{
  FILE *f;

  assert(prop != NULL);
  assert(prop->type == LIBAUDIO_TYPE_WAV);
  assert(prop->loader_data != NULL);

  f = (FILE *)(prop->loader_data);
  fclose(f);
}

static libaudio_prop_t *
loader_wav_rewind(libaudio_prop_t *prop)
{
  assert(prop != NULL);
  assert(prop->type == LIBAUDIO_TYPE_WAV);
  assert(prop->loader_data != NULL);

  loader_wav_close(prop);
  return loader_wav_open_prop(prop->filename, prop);
}

static int
loader_wav_read_sample(libaudio_prop_t *prop)
{
  FILE *f;
  int c1, c2;

  assert(prop != NULL);
  assert(prop->type == LIBAUDIO_TYPE_WAV);
  assert(prop->loader_data != NULL);

  f = (FILE *)(prop->loader_data);

  c1 = fgetc(f);
  if (c1 == EOF)
    return EOF;

  if (prop->sample_size == 8)
    return 256 * c1;

  if (prop->sample_size == 16)
    {
      c2 = fgetc(f);
      if (c2 == EOF)
	return EOF;

      c1 = 256 * c2 + c1;
      if (c1 > 32767)
	c1 = c1 - 65536;

      return c1 + 32768;
    }

  return EOF;
}

static const char *
loader_wav_get_type(void)
{
  return "WAV";
}

static const char *
loader_wav_get_name(void)
{
  return loader_name;
}

static libaudio_loader_t loader = {
  loader_wav_check,
  loader_wav_open,
  loader_wav_rewind,
  loader_wav_close,
  loader_wav_read_sample,
  loader_wav_get_type,
  loader_wav_get_name
};

void
loader_wav_init(void)
{
  snprintf(loader_name,
	   sizeof(loader_name),
	   "loader for wav files");

  if (libaudio_register_loader(&loader))
    self = &loader;
}
