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
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/z80.h"
#include "kc/ctc.h"
#include "kc/sound3.h"

#include "libdbg/dbg.h"

#define SOUND_BUFFER_SIZE (1024)
#define SOUND_SAMPLE_FREQ (44100)
#define SYNC_COUNTER (10)

void
Sound3::sound_callback(void *userdata, unsigned char *stream, int len)
{
  sndop *op = NULL;
  Sound3 *self = (Sound3 *)userdata;
  long long cur, diff, idx;
  static long long counter = 0;
  static int val = 1;
  static int xxx = 0;

  static int freq = 0;

  cur = z80->getCounter();

  if (counter == 0)
    {
      counter = cur;
      memset(stream, 0, len);
      return;
    }

  diff = cur - counter;

  int a = 0;

  if (!self->_sndop_list.empty())
    op = *(self->_sndop_list.begin());

  while (a < len)
    {
      idx =  counter + (a * diff) / len;

      if (op && (idx > op->_counter))
	{
	  freq = (int)op->_freq;
	  if (freq > SOUND_SAMPLE_FREQ)
	    {
	      xxx = 0;
	      freq = 0;
	      val = -val;
	    }

	  //cout << a << " - " << op->_counter << " / " << op->_freq << endl;
	  self->_sndop_list.pop_front();
	  op = NULL;
	  if (self->_sndop_list.size() > 0)
	    op = *(self->_sndop_list.begin());

	  if (xxx == 0)
	    if (freq != 0)
	      xxx = SOUND_SAMPLE_FREQ / freq;
	}

      if (freq == 0)
	stream[a] = 128;
      else
	stream[a] = 128 + 80 * val;

      if (xxx > 0)
	if (--xxx == 0)
	  {
	    val = -val;
	    if (freq != 0)
	      xxx = SOUND_SAMPLE_FREQ / freq;
	  }

      a++;
    }
  
  counter = cur;
}

Sound3::Sound3(void)
{
}

Sound3::~Sound3(void)
{
  delete _dummy_sndop;
}

void
Sound3::init()
{
  open_sound();

  ctc->register_callback(0, this);
  pio->register_callback_B_out(this);

  _playing = true;
  _dummy_sndop = new sndop(0, 0);
  _last_sndop = _dummy_sndop;
}

void
Sound3::start()
{
  double freq = (1750000.0 / 2) / ctc->getTimerValue(0);

  if (_last_sndop->_freq != freq)
    {
      //cout << "sound: start [1]: " << z80->getCounter() << endl;
      _last_sndop = new sndop(z80->getCounter(), freq);
      _sndop_list.push_back(_last_sndop);
    }
}

void
Sound3::stop()
{
  //cout << "sound: stop [1]: " << z80->getCounter() << endl;
  if (_last_sndop->_freq != 0)
    {
      //cout << "sound: stop [2]: " << z80->getCounter() << endl;
      _last_sndop = new sndop(z80->getCounter(), 0);
      _sndop_list.push_back(_last_sndop);
    }
}

void
Sound3::callback_B_out(byte_t val)
{
  if (((val & 1) == 1) && ((val & 0x0e) != 0x0e) && !ctc->isReset(0))
    start();
  else
    stop();
}

void
Sound3::ctc_callback_start(int channel)
{
  if (channel != 0)
    return;

  if (ctc->getTimerValue(channel) == 0)
    return;

  //printf("Sound3::ctc_callback_start()\n");
  start();
}

void
Sound3::ctc_callback_stop(int channel)
{
  if (channel != 0)
    return;

  stop();
}

void
Sound3::ctc_callback_TC(int channel, long tc)
{
  double freq;

  switch (channel)
    {
    case 0:
      freq = (1750000.0 / 2) / tc;
      //printf("%04xh: Sound3::cb_TC() - %10Ld - %12.2f\n", z80->getPC(), z80->getCounter(), freq);
      if (_last_sndop->_freq != freq)
	{
	  _last_sndop = new sndop(z80->getCounter(), freq);
	  _sndop_list.push_back(_last_sndop);
	}
      break;
    default:
      break;
    }
}
