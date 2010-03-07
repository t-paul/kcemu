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

#include "kc/system.h"

#if ENABLE_SOUND
#include <SDL.h>
#endif /* ENABLE_SOUND */

#include "kc/sound.h"

#define SOUND_BUFFER_SIZE (1024)
#define SOUND_SAMPLE_FREQ (44100)

void
Sound::sdl_sound_callback(void *userdata, unsigned char *stream, int len)
{
  Sound *self = (Sound *)userdata;
  self->sound_callback(userdata, stream, len);
}

Sound::Sound(void)
{
}

Sound::~Sound(void)
{
}

void
Sound::open_sound(void)
{
#if ENABLE_SOUND
  int ret;
  SDL_AudioSpec wanted, obtained;
  
  wanted.freq = SOUND_SAMPLE_FREQ;
  wanted.format = AUDIO_U8;
  wanted.channels = 1;
  wanted.samples = SOUND_BUFFER_SIZE;
  wanted.callback = sdl_sound_callback;
  wanted.userdata = this;

  ret = SDL_OpenAudio(&wanted, &obtained);

#if 0
  printf("ret = %d\n", ret);

  printf("obtained.freq     = %d\n", obtained.freq);
  printf("obtained.format   = %d\n", obtained.format);
  printf("obtained.channels = %d\n", obtained.channels);
  printf("obtained.samples  = %d\n", obtained.samples);
  printf("obtained.silence  = %d\n", obtained.silence);
  printf("obtained.size     = %d\n", obtained.size);
#endif

  SDL_PauseAudio(0);
#endif /* ENABLE_SOUND */
}

void
Sound::lock(void)
{
#if ENABLE_SOUND
  SDL_LockAudio();
#endif
}

void
Sound::unlock(void)
{
#if ENABLE_SOUND
  SDL_UnlockAudio();
#endif
}
