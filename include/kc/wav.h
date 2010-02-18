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

#ifndef __kc_wav_h
#define __kc_wav_h

#include <fstream>

#include "kc/system.h"

#include "kc/cb.h"

#include "cmd/cmd.h"

#include "libaudio/libaudio.h"

class WavPlayer : public Callback
{
 private:
  enum {
    FILTER_SIZE = 100,
  };

 private:
  CMD *_cmd;
  libaudio_prop_t *_prop;

  bool _stopped;

  int _val;
  int _th_low;
  int _th_high;
  int _sample_freq;
  int _sample_size;
  int _channels;
  int _bit_0;
  int _bit_1;
  int _bit_s;

  int   _eof;
  float _kernel[FILTER_SIZE + 1];
  int   _buf[FILTER_SIZE + 1];

  CMD_Args _info_args;

 protected:
  int get_y(float value);
  void init_filter_kernel(void);

 public:
  WavPlayer(int bit_0, int bit_1, int bit_s);
  virtual ~WavPlayer(void);
  
  virtual bool open(const char *filename);
  virtual bool play(void);
  virtual void stop(void);
  virtual void close(void);
  virtual void record(void);
  
  virtual void callback(void *data);
  virtual void do_play(void);
  virtual void do_record(void);
};

#endif /* __kc_wav_h */
