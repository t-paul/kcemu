/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2002 Torsten Paul
 *
 *  $Id: libaudioP.h,v 1.1 2002/10/31 00:57:22 torsten_paul Exp $
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

#ifndef __libaudio_libaudiop_h
#define __libaudio_libaudiop_h

#include "kc/config.h"

#include "libaudio/libaudio.h"

#define MAX_LIBAUDIO_LOADERS (10)

typedef struct libaudio_loader
{
  int (*check)(const char *filename, unsigned char *data, long size);
  libaudio_prop_t * (*open)(const char *filename);
  libaudio_prop_t * (*rewind)(libaudio_prop_t *prop);
  void (*close)(libaudio_prop_t *prop);
  int (*read_sample)(libaudio_prop_t *prop);
  const char * (*get_type)(void);
  const char * (*get_name)(void);
} libaudio_loader_t;

struct libaudio_prop
{
  libaudio_type_t type;
  char *filename;

  int sample_freq;
  int sample_size;
  int channels;

  libaudio_loader_t *loader;
  void *loader_data;
};

int libaudio_register_loader(libaudio_loader_t *loader);

void loader_oss_init(void);
void loader_als_init(void);
void loader_snd_init(void);
void loader_aud_init(void);
void loader_mp3_init(void);
void loader_ogg_init(void);
void loader_flc_init(void);
void loader_wav_init(void);
void loader_raw_init(void);

#endif /* __libaudio_libaudiop_h */
