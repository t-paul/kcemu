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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "libaudio/libaudioP.h"

#ifdef HAVE_OSS

#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include OSS_HEADER

#define OSS_BUF_SIZE (1024)

static char loader_name[1024];
static libaudio_loader_t *self = NULL;

typedef struct oss_data
{
  int fd;
  int eof;
  int idx;
  unsigned char buf[OSS_BUF_SIZE];
} oss_data_t;

static int
loader_oss_check(const char *filename, unsigned char *data, long size)
{
  if (strcmp(filename, "/dev/dsp") == 0)
    return 1;

  return 0;
}

static libaudio_prop_t *
loader_oss_open(const char *filename)
{
  oss_data_t *data;
  libaudio_prop_t *prop;
  int speed, bits, stereo;

  prop = (libaudio_prop_t *)malloc(sizeof(libaudio_prop_t));
  if (prop == NULL)
    return NULL;

  data = (oss_data_t *)malloc(sizeof(oss_data_t));
  if (data == NULL)
    {
      free(prop);
      return NULL;
    }

  data->eof = 0;
  data->idx = 0;
  data->fd = open(filename, O_RDONLY);
  if (data->fd < 0)
    {
      free(prop);
      free(data);
      return NULL;
    }

  speed = 44100;
  bits = 8;
  stereo = 0;

  if (ioctl(data->fd, SNDCTL_DSP_SYNC, NULL) < 0)
    return NULL;
  if (ioctl(data->fd, SNDCTL_DSP_SPEED, &speed) < 0)
    return NULL;
  if (ioctl(data->fd, SNDCTL_DSP_SAMPLESIZE, &bits) < 0)
    return NULL;
  if (ioctl(data->fd, SNDCTL_DSP_STEREO, &stereo) < 0)
    return NULL;

  prop->type = LIBAUDIO_TYPE_OSS;
  prop->loader = self;
  prop->loader_data = data;

  prop->sample_freq = 44100;
  prop->sample_size = 8;
  prop->channels = 1;

  return prop;
}

static libaudio_prop_t *
loader_oss_rewind(libaudio_prop_t *prop)
{
  assert(prop != NULL);
  assert(prop->type == LIBAUDIO_TYPE_OSS);
  assert(prop->loader_data != NULL);

  /*
   *  can't do anything sensible here, just checking if called
   *  on the appropriate stream...
   */

  return prop;
}

static void
loader_oss_close(libaudio_prop_t *prop)
{
  oss_data_t *data;

  assert(prop != NULL);
  assert(prop->type == LIBAUDIO_TYPE_OSS);
  assert(prop->loader_data != NULL);

  data = (oss_data_t *)(prop->loader_data);

  if (data->fd >= 0)
    close(data->fd);

  free(data);
}

static int
loader_oss_read_sample(libaudio_prop_t *prop)
{
  int c;
  oss_data_t *data;

  assert(prop != NULL);
  assert(prop->type == LIBAUDIO_TYPE_OSS);
  assert(prop->loader_data != NULL);

  data = (oss_data_t *)(prop->loader_data);

  if (data->eof)
    return EOF;

  if (data->idx >= OSS_BUF_SIZE)
    {
      if (read(data->fd, data->buf, OSS_BUF_SIZE) != OSS_BUF_SIZE)
	{
	  data->eof = 1;
	  close(data->fd);
	  data->fd = -1;
	  return EOF;
	}
      data->idx = 0;
    }

  c = 256 * data->buf[data->idx];
  data->idx++;

  return c;
}

static const char *
loader_oss_get_type(void)
{
  return "OSS";
}

static const char *
loader_oss_get_name(void)
{
  return loader_name;
}

static libaudio_loader_t loader = {
  loader_oss_check,
  loader_oss_open,
  loader_oss_rewind,
  loader_oss_close,
  loader_oss_read_sample,
  loader_oss_get_type,
  loader_oss_get_name
};

void
loader_oss_init(void)
{
  snprintf(loader_name,
	   sizeof(loader_name),
	   "loader for Open Sound System line input (/dev/dsp)");

  if (libaudio_register_loader(&loader))
    self = &loader;
}

#else /* HAVE_OSS */

void
loader_oss_init(void)
{
}

#endif /* HAVE_OSS */
