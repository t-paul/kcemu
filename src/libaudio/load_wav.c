/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2002 Torsten Paul
 *
 *  $Id: load_wav.c,v 1.1 2002/10/31 00:57:22 torsten_paul Exp $
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

typedef struct wav_header
{
  unsigned long  MainChunkID;                      // it will be 'RIFF'
  unsigned long  Length;
  unsigned long  ChunkTypeID;                      // it will be 'WAVE'
  unsigned long  SubChunkID;                       // it will be 'fmt '
  unsigned long  SubChunkLength;
  unsigned short AudioFormat;
  unsigned short NumChannels;
  unsigned long  SampleFreq;
  unsigned long  BytesPerSec;
  unsigned short BytesPerSample;
  unsigned short BitsPerSample;
  unsigned long  DataChunkID;                      // it will be 'data'
  unsigned long  DataLength;
} wav_header_t;

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
  return "loader for wav files";
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
  if (libaudio_register_loader(&loader))
    self = &loader;
}
