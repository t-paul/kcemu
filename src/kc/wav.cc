/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-1998 Torsten Paul
 *
 *  $Id: wav.cc,v 1.2 2000/06/24 03:39:48 tp Exp $
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

#include <fcntl.h>
#include <unistd.h>
#include <iostream.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>

#include "kc/config.h"
#include "kc/system.h"

#include "kc/z80.h"
#include "kc/pio.h"
#include "kc/wav.h"

#include "libdbg/dbg.h"

class CMD_kc_play_wav : public CMD
{
private:
  WavPlayer *_p;

public:
  CMD_kc_play_wav(WavPlayer *p) : CMD("kc-wav-play")
    {
      _p = p;
      register_cmd("kc-wav-play", 0);
      register_cmd("kc-wav-record", 1);
      register_cmd("kc-wav-stop", 2);
    }
  
  virtual ~CMD_kc_play_wav(void)
    {
    }

  virtual void execute(CMD_Args *args, CMD_Context context)
    {
      ifstream *is;
      const char *filename;

      switch (context)
        {
          /*
           *  kc-wav-play
           */
        case 0:
          filename = 0;
          if (args)
            filename = args->get_string_arg("filename");
          if (filename == 0)
            return;
          
          printf("CMD_kc_play_wav::execute(): '%s'\n", filename);
          
          is = new ifstream();
          is->open(filename, ios::in | ios::bin);
          if (!(*is))
            return;
          _p->play(is);
          break;
          /*
           *  kc-wav-record
           */
        case 1:
          _p->record();
          break;
          /*
           *  kc-wav-stop
           */
        case 2:
          _p->stop();
          break;
        }
    }
};

WavPlayer::WavPlayer(void) : Callback("WavPlayer")
{
  _cmd = new CMD_kc_play_wav(this);
  _th_low = -5;
  _th_high = 5;
  _val = 0;
  _audio_fd = -1;
}

WavPlayer::~WavPlayer(void)
{
}

void
WavPlayer::play(istream *is)
{
  DBG(1, form("KCemu/WavPlayer/play",
              "WavPlayer::play()\n"));

  _is = is;
  z80->addCallback(0, this, (void *)0);
}

void
WavPlayer::record(void)
{
  int speed, bits, stereo;

  printf("WavPlayer::record(void)\n");

  speed = 48000;
  bits = 8;
  stereo = 0;
  _audio_fd = open("/dev/dsp", O_RDONLY);
  if (_audio_fd < 0)
    return;

  if (ioctl (_audio_fd, SNDCTL_DSP_SYNC, NULL) < 0)
    return;
  if (ioctl(_audio_fd, SNDCTL_DSP_GETFMTS, &_fmt_mask) < 0)
    return;
  if (ioctl(_audio_fd, SNDCTL_DSP_SPEED, &speed) < 0)
    return;
  if (ioctl(_audio_fd, SNDCTL_DSP_SAMPLESIZE, &bits) < 0)
    return;
  if (ioctl(_audio_fd, SNDCTL_DSP_STEREO, &stereo) < 0)
    return;
  
  printf("WavPlayer::record(void): adding callback\n");
  z80->addCallback(0, this, (void *)1);
}

void
WavPlayer::stop(void)
{
  DBG(1, form("KCemu/WavPlayer/play",
              "WavPlayer::stop()\n"));
  if (_audio_fd >= 0)
    close(_audio_fd);

  _audio_fd = -1;
}

void
WavPlayer::callback(void *data)
{
  int d = (int)data;

  switch (d)
    {
    case 0:
      do_play();
      break;
    case 1:
      do_record();
      break;
    }
}

void
WavPlayer::do_play(void)
{
  int c, old_val;
  int byte_cnt;
  double v;
  
  old_val = 0;
  byte_cnt = 0;

  pio->strobe_A();

  while (242)
    {
      c = _is->get();
      byte_cnt++;
      if (c == EOF)
        {
          stop();
          return;
        }
      c = c - 128;

      // cout.form("* %+d : %+d | %+d\n", old_val, _val, c);
      old_val = _val;
      if (_val == 0)
        {
          if (c > _th_high)
            _val = 1;
        }
      else
        {
          if (c < _th_low)
            _val = 0;
        }
  
      if (old_val !=_val)
        break;
    }
  
  v = (1458.0 * byte_cnt) / 48.0;
#if 0
  z80->addCallback(v, this, (void *)0);
#else
  if (v < 500)
    {
      z80->addCallback(364, this, (void *)0);
    }
  else if (v > 1000)
    {
      z80->addCallback(1458, this, (void *)0);
    }
  else
    {
      z80->addCallback(729, this, (void *)0);
    }
#endif
}
 
void
WavPlayer::do_record(void)
{
#define BLEN (65536)
  static int idx = BLEN;
  unsigned char buf[BLEN];
  int c, old_val;
  int byte_cnt, len;
  double v;
  
  old_val = 0;
  byte_cnt = 0;

  if (_audio_fd < 0)
    return;

  pio->strobe_A();

  while (242)
    {
      if (idx >= BLEN)
        {
          idx = 0;
          len = read(_audio_fd, buf, BLEN);
          if (len != BLEN)
            printf("read - len = %d\n", len);
        }
      c = buf[idx++];
      byte_cnt++;
      c = c - 128;

      // cout.form("* %+d : %+d | %+d\n", old_val, _val, c);
      old_val = _val;
      if (_val == 0)
        {
          if (c > _th_high)
            _val = 1;
        }
      else
        {
          if (c < _th_low)
            _val = 0;
        }
  
      if (old_val !=_val)
        break;
    }
  
  v = (1458.0 * byte_cnt) / 48.0;
#if 0
  z80->addCallback(v, this, (void *)1);
#else
  if (v < 500)
    {
      z80->addCallback(364, this, (void *)1);
    }
  else if (v > 1000)
    {
      z80->addCallback(1458, this, (void *)1);
    }
  else
    {
      z80->addCallback(729, this, (void *)1);
    }
#endif
}
