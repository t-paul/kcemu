/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: sound.h,v 1.6 2002/06/09 14:24:32 torsten_paul Exp $
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
