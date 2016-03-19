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

#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

#include "kc/system.h"
#include "kc/prefs/prefs.h"

#include "kc/z80.h"
#include "kc/pio.h"
#include "kc/wav.h"

#include "sys/sysdep.h"

#include "ui/status.h"

#include "libdbg/dbg.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class CMD_kc_play_wav : public CMD
{
private:
  WavPlayer *_p;
  static const char * _path;

public:
  CMD_kc_play_wav(WavPlayer *p) : CMD("kc-wav-open")
    {
      _p = p;
      register_cmd("kc-wav-open", 0);
      register_cmd("kc-wav-play", 2);
      register_cmd("kc-wav-record", 3);
      register_cmd("kc-wav-stop", 4);
      register_cmd("kc-wav-close", 5);
    }
  
  virtual ~CMD_kc_play_wav(void)
    {
    }

  virtual void execute(CMD_Args *args, CMD_Context context)
    {
      const char *filename = 0;

      if (args)
	filename = args->get_string_arg("filename");
      else
	args = new CMD_Args;

      switch (context)
        {
          /*
           *  kc-wav-open
           */
        case 0:
	  if (filename == 0)
	    {
	      args->set_string_arg("ui-file-select-title",
				   _("KCemu: Select File..."));
	      if (_path)
		args->set_string_arg("ui-file-select-path", _path);
	      args->add_callback("ui-file-select-CB-ok", this, 1);
	      CMD_EXEC_ARGS("ui-file-select", args);
	      break;
	    }
	  /* fall through */

	case 1:
          if (filename == 0)
            return;

	  if (_p->open(filename))
	    {
	      _path = filename;
	      CMD_EXEC_ARGS("ui-wav-file-selected", args);
	    }
	  break;

          /*
           *  kc-wav-play
           */
	case 2:
	  _p->play();
	  break;

          /*
           *  kc-wav-record
           */
        case 3:
          _p->record();
          break;
          /*
           *  kc-wav-stop
           */
        case 4:
          _p->stop();
          break;
	case 5:
	  _p->close();
	  CMD_EXEC("ui-wav-file-closed");
	  break;
        }
    }
};

const char * CMD_kc_play_wav::_path = NULL;

WavPlayer::WavPlayer(int bit_0, int bit_1, int bit_s) : Callback("WavPlayer")
{
  _bit_0 = bit_0;
  _bit_1 = bit_1;
  _bit_s = bit_s;

  _cmd = new CMD_kc_play_wav(this);
  _th_low = -10;
  _th_high = 10;
  _val = 0;
  _prop = NULL;

  _stopped = true;
}

WavPlayer::~WavPlayer(void)
{
}


void
WavPlayer::init_filter_kernel(void)
{
  int a;
  float amp;
  float sum;
  float lp_FC;
  float hp_FC;
  float lp_kernel[FILTER_SIZE + 1];
  float hp_kernel[FILTER_SIZE + 1];

  amp = 1.0;

  for (int a = 0;a <= FILTER_SIZE;a++)
    _buf[a] = 0;

  lp_FC = 26000.0 / _sample_freq;
  hp_FC =  400.0 / _sample_freq;

  /*
   *  low pass
   */
  for (a = 0;a <= 100;a++)
    {
      if ((a - FILTER_SIZE / 2) == 0)
	lp_kernel[a] = 2 * M_PI * lp_FC;
      else
	lp_kernel[a] = sin(2 * M_PI * lp_FC * (a - FILTER_SIZE / 2)) / (a - FILTER_SIZE / 2);

      lp_kernel[a] = lp_kernel[a] * (0.54 - 0.46 * cos(2 * M_PI * a / FILTER_SIZE));
    }

  sum = 0;
  for (a = 0;a <= 100;a++)
    sum += lp_kernel[a];
  sum /= amp;

  for (a = 0;a <= 100;a++)
    lp_kernel[a] = lp_kernel[a] / sum;

  /*
   *  high pass
   */
  for (a = 0;a <= 100;a++)
    {
      if ((a - FILTER_SIZE / 2) == 0)
	hp_kernel[a] = 2 * M_PI * hp_FC;
      else
	hp_kernel[a] = sin(2 * M_PI * hp_FC * (a - FILTER_SIZE / 2)) / (a - FILTER_SIZE / 2);

      hp_kernel[a] = hp_kernel[a] * (0.54 - 0.46 * cos(2 * M_PI * a / FILTER_SIZE));
    }

  sum = 0;
  for (a = 0;a <= 100;a++)
    sum += hp_kernel[a];
  sum /= amp;

  for (a = 0;a <= 100;a++)
    hp_kernel[a] = hp_kernel[a] / sum;

  for (a = 0;a <= 100;a++)
    hp_kernel[a] = -hp_kernel[a];
  hp_kernel[FILTER_SIZE / 2] += 1;

  /*
   *  band reject
   */
  for (a = 0;a <= 100;a++)
    _kernel[a] = lp_kernel[a] + hp_kernel[a];

  /*
   *  band pass
   */
  for (a = 0;a <= 100;a++)
    _kernel[a] = -_kernel[a];
  _kernel[FILTER_SIZE / 2] += 1;

  /*
   *  invert output
   */
  for (a = 0;a <= 100;a++)
    _kernel[a] = -_kernel[a];
}

bool
WavPlayer::open(const char *filename)
{
  DBG(1, form("KCemu/WavPlayer",
              "WavPlayer::open(): `%s'\n",
	      filename));

  if (_prop != NULL)
    close();

  _prop = libaudio_open(filename);
  if (_prop == NULL)
    return false;

  return true;
}

bool
WavPlayer::play()
{
  char buf[1000];
  char *shortname;
  const char *filename, *fmt;

  DBG(1, form("KCemu/WavPlayer",
              "WavPlayer::play()\n"));

  if (_prop == NULL)
    return false;

  filename = libaudio_get_filename(_prop);
  if (filename == NULL)
    return false;

  shortname = sys_basename(filename);

  fmt = libaudio_get_type(_prop);

  _sample_freq = libaudio_get_sample_freq(_prop);
  _sample_size = libaudio_get_sample_size(_prop);
  _channels = libaudio_get_channels(_prop);

  snprintf(buf, sizeof(buf), _("Reading %s `%s' (%d/%d/%d)."),
	  fmt, shortname, _sample_freq, _sample_size, _channels);
  Status::instance()->setMessage(buf);
  free(shortname);

  init_filter_kernel();

  _eof = 0;
  _stopped = false;
  z80->addCallback(0, this, (void *)0);

  return true;
}

void
WavPlayer::record(void)
{
}

void
WavPlayer::stop(void)
{
  DBG(1, form("KCemu/WavPlayer",
              "WavPlayer::stop()\n"));

  if (_prop == NULL)
    return;

  libaudio_rewind(_prop);
  _stopped = true;
}

void
WavPlayer::close()
{
  DBG(1, form("KCemu/WavPlayer",
              "WavPlayer::close()\n"));

  if (_prop != NULL)
    libaudio_close(_prop);

  _prop = NULL;
}

void
WavPlayer::callback(void *data)
{
  long d = (long)data;

  if (_stopped)
    return;

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

/*
 *  algorithm copied from audacity TrackArtist::GetWaveYPos()
 */
int
WavPlayer::get_y(float value)
{
  int ret;
  float sign = (value >= 0 ? 1 : -1);

  if (value == 0)
    return 0;

  float db = 10 * log10(fabs(value) / 32768);
  // The smallest value we will see is -45.15 (10*log10(1/32768))
  float val = (db + 45.0) / 45.0;
  if (val < 0.0)
    val = 0.0;
  if (val > 1.0)
    val = 1.0;

  ret = (int)(sign * (32767 * val + 0.5));
  // printf("%8.2f - %8d\n", value, ret);
  return ret;
}

void
WavPlayer::do_play(void)
{
  int a, c;
  int byte_cnt;
  double v;
  float y;
  
  byte_cnt = 0;

  pio->strobe_A();

  /*
   *  the digital filter causes some delay in the audio signal
   *  so we need to do some padding on the end of the input
   */
  if (_eof > 0)
    {
      _eof--;
      if (_eof == 0)
	{
	  stop();
	  return;
	}
    }

  while (242)
    {
      c = 0;
      if (_eof == 0)
	{
	  c = libaudio_read_sample(_prop);
	  if (c == EOF)
	    {
	      _eof = 200;
	      c = 32768;
	    }
	  c -= 32768;
	  
	  if (_channels == 2)
	    {
	      if (libaudio_read_sample(_prop) == EOF)
		_eof = 200;
	    }
	}

      byte_cnt++;

      /*
       *  convert wavform
       */
#if 0
      c = get_y((float)c);
#endif

      /*
       *  apply band pass filter
       */
#if 1
      _buf[100] = c;
      y = 0;
      for (a = 0;a <= 100;a++)
	y = y + _buf[100 - a] * _kernel[a];
      
      for (a = 1;a <= 100;a++)
	_buf[a - 1] = _buf[a];

      /*
       *  filter amplifies the signal, we just cut at
       *  arbitrary values because we are only interrested
       *  in the points where the signal changes sign
       */
      c = (int)y;
      if (c > 32000)
	c = 32000;
      if (c < -32000)
	c = -32000;
#endif

      if (_val == 0)
        {
          if (c > _th_high)
	    {
	      _val = 1;
	      break;
	    }
        }
      else
        {
          if (c < _th_low)
	    {
	      _val = 0;
	      break;
	    }
        }
  
      v = (1750000.0 * byte_cnt) / _sample_freq;
      if (v > 2000)
	break;
    }

  if (Preferences::instance()->get_kc_type() & KC_TYPE_85_2_CLASS)
    {
      v = (1750000.0 * byte_cnt) / _sample_freq;
    }
  else
    {
      v = (2500000.0 * byte_cnt) / _sample_freq;
    }

  v = v * 1.2;
  if (v > 2000)
    v = 2000;

  _info_args.set_long_arg("gap", (int)v);
  CMD_EXEC_ARGS("ui-wav-info", &_info_args);

#if 1
  printf("%d / %d / %d - %ld\n", _bit_0, _bit_1, _bit_s, (long)v);
  z80->addCallback((long long)v, this, (void *)0);
#else
  if (v < 550)
    {
      z80->addCallback(_bit_0, this, (void *)0);
    }
  else if (v > 1100)
    {
      z80->addCallback(_bit_s, this, (void *)0);
    }
  else
    {
      z80->addCallback(_bit_1, this, (void *)0);
    }
#endif
}
 
void
WavPlayer::do_record(void)
{
#if 0
#define BLEN (65536)
  static int idx = BLEN;
  unsigned char buf[BLEN];
  int c, old_val;
  int byte_cnt, len;
  double v;
  
  old_val = 0;
  byte_cnt = 0;

  pio->strobe_A();

  while (242)
    {
      if (idx >= BLEN)
        {
          idx = 0;
          //len = read(_audio_fd, buf, BLEN);
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
#endif
}
