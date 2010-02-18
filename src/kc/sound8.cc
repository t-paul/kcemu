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

#include <stdio.h> // FIXME:
#include <iomanip>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/z80.h"
#include "kc/sound8.h"

#define SOUND_BUFFER_SIZE (1024)
#define SOUND_SAMPLE_FREQ (44100)

static FILE *_f;

void
Sound8::sound_callback(void *userdata, unsigned char *stream, int len)
{
  int a;
  sndop8 *op;
  long long c;
  static int xxx = 10;

  c = z80->getCounter() - 30000;

  if (_snd_cnt == 0)
    {
      _snd_cnt = c;
      return;
    }

  for (a = 0;a < len;a++)
    stream[a] = 0x080;

  if (_sndop_list.size() == 0)
    {
      if (--xxx < 0)
	_snd_cnt = c;
      return;
    }

  xxx = 10;

  op = 0;
  if (_sndop_list.size() > 0)
    op = _sndop_list.front();

  for (a = 0;a < len;a++)
    {
      if (op && op->_counter <= _snd_cnt)
	{
	  _val = op->_val;
	  _sndop_list.pop_front();
	  op = 0;
	  if (_sndop_list.size() > 0)
	    op = _sndop_list.front();
	}

      stream[a] = _val;
      _snd_cnt += 21;
    }

  fwrite(stream, 1, len, _f);
}

Sound8::Sound8(void)
{
  _f = fopen("/tmp/lc80.wav", "wb");
}

Sound8::~Sound8(void)
{
  delete _buf;
  fclose(_f);
}

void
Sound8::init(void)
{
  _cnt = 0;
  _val = 0;
  _snd_cnt = 0;
  _idx_r = _idx_w = 0;

  _buf = new long long[SND_BUF_SIZE];

  pio->register_callback_B_out(this);

  open_sound();
}

void
Sound8::start(void)
{
}

void
Sound8::stop(void)
{
}

int
Sound8::callback_A_in(void)
{
  return -1;
}

void
Sound8::callback_A_out(byte_t val)
{
}

int
Sound8::callback_B_in(void)
{
  return -1;
}

void
Sound8::callback_B_out(byte_t val)
{
  static int old_val = 0;
  int v = (val & 2) ? 128 + 50 : 128 - 50;

  if (old_val == v)
    return;

  if (_cnt == 0)
    {
      _cnt = z80->getCounter();
      return;
    }

  old_val = v;
  _cnt = z80->getCounter();

  lock();
  _sndop_list.push_back(new sndop8(_cnt, v));
  unlock();
}

void
Sound8::callback(void *data)
{
}
