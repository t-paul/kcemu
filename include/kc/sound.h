/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: sound.h,v 1.4 2002/02/12 17:24:14 torsten_paul Exp $
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

#include "kc/config.h"
#include "kc/system.h"

#if HAVE_LIBSDL

#include "kc/pio.h"

class sndop
{
 public:
  long long _counter;
  double _freq;

  sndop(long long counter, double freq)
    {
      //cout << "new sndop(): counter = " << counter << ", freq = " << freq << endl;
      _counter = counter;
      _freq = freq;
    }
};

class Sound : public PIOCallbackInterface, public CTCCallbackInterface
{
 private:
  typedef list<sndop *>sndop_list_t;
  typedef sndop_list_t::iterator iterator;

 private:
  bool _playing;
  sndop_list_t _sndop_list;
  sndop *_last_sndop;
  sndop *_dummy_sndop;

 protected:
  static void sound_callback(void *userdata, unsigned char *stream, int len);

 public:
  Sound(void);
  virtual ~Sound(void);

  virtual void init(void);
  virtual void start(void);
  virtual void stop(void);

  /*
   *  PIOCallbackInterface functions
   */
  virtual void callback_A_in(void) {}
  virtual void callback_A_out(byte_t val) {}
  virtual void callback_B_in(void) {}
  virtual void callback_B_out(byte_t val);

  /*
   * CTCCallbackInterface functions
   */
  virtual void ctc_callback_ZC(int channel) {};
  virtual void ctc_callback_TC(int channel, long tc);
  virtual void ctc_callback_start(int channel);
  virtual void ctc_callback_stop(int channel);
};

#endif /* HAVE_LIBSDL */

#endif /* __kc_sound_h */
