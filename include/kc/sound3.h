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

#ifndef __kc_sound3_h
#define __kc_sound3_h

#include <list>

#include "kc/system.h"

#include "kc/pio.h"
#include "kc/sound.h"

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

class Sound3 : public Sound, public PIOCallbackInterface, public CTCCallbackInterface
{
 private:
  typedef std::list<sndop *>sndop_list_t;
  typedef sndop_list_t::iterator iterator;

 private:
  bool _playing;
  sndop_list_t _sndop_list;
  sndop *_last_sndop;
  sndop *_dummy_sndop;

 protected:
  void sound_callback(void *userdata, unsigned char *stream, int len);

 public:
  Sound3(void);
  virtual ~Sound3(void);

  virtual void init(void);
  virtual void start(void);
  virtual void stop(void);

  /*
   *  PIOCallbackInterface functions
   */
  virtual int callback_A_in(void) { return -1; }
  virtual int callback_B_in(void) { return -1; }
  virtual void callback_A_out(byte_t val) {}
  virtual void callback_B_out(byte_t val);

  /*
   * CTCCallbackInterface functions
   */
  virtual void ctc_callback_ZC(int channel) {};
  virtual void ctc_callback_TC(int channel, long tc);
  virtual void ctc_callback_start(int channel);
  virtual void ctc_callback_stop(int channel);
};

#endif /* __kc_sound3_h */
