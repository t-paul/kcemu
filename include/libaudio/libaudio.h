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

#ifndef __libaudio_libaudio_h
#define __libaudio_libaudio_h

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
  LIBAUDIO_TYPE_ALL     = 0xffffffff,
  LIBAUDIO_TYPE_UNKNOWN = (1 << 0),
  LIBAUDIO_TYPE_OSS     = (1 << 1),
  LIBAUDIO_TYPE_ALS     = (1 << 2),
  LIBAUDIO_TYPE_SND     = (1 << 3),
  LIBAUDIO_TYPE_AUD     = (1 << 4),
  LIBAUDIO_TYPE_MP3     = (1 << 5),
  LIBAUDIO_TYPE_OGG     = (1 << 6),
  LIBAUDIO_TYPE_FLC     = (1 << 7),
  LIBAUDIO_TYPE_WAV     = (1 << 8),
} libaudio_type_t;
  
typedef struct libaudio_prop libaudio_prop_t;

/*
 *  initialize loader modules. the loaders parameter
 *  may specify which loaders will be enabled. if the
 *  parameter is LIBAUDIO_TYPE_ALL all loaders will
 *  be enabled.
 */
void libaudio_init(int loaders);

/*
 *  open the specified file by trying the available
 *  loaders -- the first loader that thinks it's able
 *  to read the file will be choosen
 */
libaudio_prop_t * libaudio_open(const char *filename);

/*
 *  rewind to beginning of file (if possible)
 */
void libaudio_rewind(libaudio_prop_t *prop);

/*
 *  close file
 */
void libaudio_close(libaudio_prop_t *prop);

/*
 *  returns an unsigned sample in the range 0..65535
 *  when the audio source is stereo the samples will
 *  come interleaved
 */
int libaudio_read_sample(libaudio_prop_t *prop);

/*
 *  returns a short string with the type of the
 *  audio source (e.g. WAV or MP3)
 */
const char * libaudio_get_type(libaudio_prop_t *prop);

/*
 *  returns the name of the loader which reads the
 *  data from the audio source
 */
const char * libaudio_get_name(libaudio_prop_t *prop);

/*
 *  returns the name of the opened audio source as
 *  given in the libaudio_open() function
 */
const char * libaudio_get_filename(libaudio_prop_t *prop);

/*
 *  returns the sample frequency of data that is
 *  delivered by libaudio_read_sample() -- this can
 *  be different from the actual file as the loader
 *  may change the format
 */
int libaudio_get_sample_freq(libaudio_prop_t *prop);

/*
 *  returns the size of the samples (8 or 16 for now)
 */
int libaudio_get_sample_size(libaudio_prop_t *prop);

/*
 *  returns the number of channels
 */
int libaudio_get_channels(libaudio_prop_t *prop);

/*
 *  dump configuration to stdout
 */    
void libaudio_show_config(void);

#ifdef __cplusplus
}
#endif

#endif /* __libaudio_libaudio_h */
