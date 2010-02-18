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

#ifndef __kc_sound_h
#define __kc_sound_h

#include "kc/system.h"

class Sound
{
 private:
  static void sdl_sound_callback(void *userdata, unsigned char *stream, int len);

 protected:
  virtual void open_sound(void);
  virtual void sound_callback(void *userdata, unsigned char *stream, int len) = 0;

 public:
  Sound(void);
  virtual ~Sound(void);

  virtual void init(void) = 0;
  virtual void start(void) = 0;
  virtual void stop(void) = 0;

  virtual void lock(void);
  virtual void unlock(void);
};

#endif /* __kc_sound_h */
