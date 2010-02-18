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

static char loader_name[1024];
static libaudio_loader_t *self = NULL;

static int
loader_raw_check(const char *filename, unsigned char *data, long size)
{
  if (data == NULL)
    return 0;
  if (size == 0)
    return 0;

  return 1;
}

static libaudio_prop_t *
loader_raw_open_prop(const char *filename, libaudio_prop_t *prop)
{
  FILE *f;

  f = fopen(filename, "rb");
  if (f == NULL)
    return NULL;

  prop->type = LIBAUDIO_TYPE_UNKNOWN;
  prop->loader = self;
  prop->loader_data = f;

  prop->sample_freq = 44100;
  prop->sample_size = 8;
  prop->channels = 1;

  return prop;
}

static libaudio_prop_t *
loader_raw_open(const char *filename)
{
  libaudio_prop_t *prop, *ret;

  prop = (libaudio_prop_t *)malloc(sizeof(libaudio_prop_t));
  if (prop == NULL)
    return NULL;
  
  ret = loader_raw_open_prop(filename, prop);
  if (ret == NULL)
    free(prop);

  return ret;
}

static void
loader_raw_close(libaudio_prop_t *prop)
{
  FILE *f;

  assert(prop != NULL);
  assert(prop->type == LIBAUDIO_TYPE_UNKNOWN);
  assert(prop->loader_data != NULL);

  f = (FILE *)(prop->loader_data);
  fclose(f);
}

static libaudio_prop_t *
loader_raw_rewind(libaudio_prop_t *prop)
{
  assert(prop != NULL);
  assert(prop->type == LIBAUDIO_TYPE_UNKNOWN);
  assert(prop->loader_data != NULL);

  loader_raw_close(prop);
  return loader_raw_open_prop(prop->filename, prop);
}

static int
loader_raw_read_sample(libaudio_prop_t *prop)
{
  FILE *f;

  assert(prop != NULL);
  assert(prop->type == LIBAUDIO_TYPE_UNKNOWN);
  assert(prop->loader_data != NULL);

  f = (FILE *)(prop->loader_data);

  return fgetc(f);
}

static const char *
loader_raw_get_type(void)
{
  return "RAW";
}

static const char *
loader_raw_get_name(void)
{
  return loader_name;
}

static libaudio_loader_t loader = {
  loader_raw_check,
  loader_raw_open,
  loader_raw_rewind,
  loader_raw_close,
  loader_raw_read_sample,
  loader_raw_get_type,
  loader_raw_get_name
};

void
loader_raw_init(void)
{
  snprintf(loader_name,
	   sizeof(loader_name),
	   "loader for raw files (44100kHz 8bit mono samples)");

  if (libaudio_register_loader(&loader))
    self = &loader;
}
