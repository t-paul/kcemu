/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: sound8.h,v 1.2 2002/06/09 14:24:32 torsten_paul Exp $
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

#ifndef __kc_sound8_h
#define __kc_sound8_h

#include <list>

#include "kc/system.h"

#include "kc/cb.h"
#include "kc/pio.h"
#include "kc/sound.h"

class sndop8
{
 public:
  int _val;
  long long _counter;
  
  sndop8(long long counter, int val)
    {
      _val = val;
      _counter = counter;
    }
};

class Sound8 : public Sound, public PIOCallbackInterface, public Callback
{
 private:
  typedef std::list<sndop8 *>sndop_list_t;

 public:
  enum {
    SND_BUF_SIZE = 0x8000,
    SND_BUF_MASK = 0x7FFF
  };

 private:
  byte_t _val;
  long long _cnt;
  long long _snd_cnt;
  long long *_buf;
  int _idx_r, _idx_w;
  sndop_list_t _sndop_list;
  
 protected:
  virtual void sound_callback(void *userdata, unsigned char *stream, int len);

 public:
  Sound8(void);
  virtual ~Sound8(void);

  virtual void init(void);
  virtual void start(void);
  virtual void stop(void);

  /*
   *  PIOCallbackInterface functions
   */
  virtual void callback_A_in(void);
  virtual void callback_A_out(byte_t val);
  virtual void callback_B_in(void);
  virtual void callback_B_out(byte_t val);

  /*
   *  Callback
   */
  void callback(void *data);
};

#endif /* __kc_sound8_h */
