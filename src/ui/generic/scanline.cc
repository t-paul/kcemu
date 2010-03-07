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

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/z80.h"
#include "kc/prefs/prefs.h"

#include "ui/generic/scanline.h"

#include "libdbg/dbg.h"

static inline void set_bit(int nr, void *addr)
{
  ((unsigned short *)addr)[nr >> 4] |= (1UL << (nr & 15));
}

static inline void clear_bit(int nr, void *addr)
{
  ((unsigned short *)addr)[nr >> 4] &= ~(1UL << (nr & 15));
}

static inline int test_bit(int nr, const void * addr)
{
  return ((1UL << (nr & 15)) & (((const unsigned short *)addr)[nr >> 4])) != 0;
}

Scanline::Scanline(void)
{
  reset(true);
}

Scanline::~Scanline(void)
{
}

void
Scanline::update(void)
{
  int c;

  _count++;
  if (_count == 3)
    _count = 0;

  if (_idx < 255)
    {
      c = _val;
      if (_idx != 0)
        c = test_bit(_idx - 1, _v3);

      if (c)
	{
	  while (_idx < 255)
	    set_bit(_idx++, _v3);
	}
      else
	{
	  while (_idx < 255)
	    clear_bit(_idx++, _v3);
	}
    }

  /*
   *  new frame time will be set when the first call to trigger()
   *  occurs this is needed to synchronize the flash frequency
   *  with the screen update
   */
  _frame_time = 0;

  _idx = 0;
  switch (_count)
    {
    case 0:
      _v1 = &_vec[0];
      _v2 = &_vec[16];
      _v3 = &_vec[32];
      break;
    case 1:
      _v1 = &_vec[16];
      _v2 = &_vec[32];
      _v3 = &_vec[0];
      break;
    case 2:
      _v1 = &_vec[32];
      _v2 = &_vec[0];
      _v3 = &_vec[16];
      break;
    default:
      break;
    }
}

/*
 *  Linefrequency: 15.625 kHz => 64�s/line => 112 cycles/line
 *
 *  This function is called by the CTC channel 2.
 */
void
Scanline::trigger(bool enable)
{
  /*
   *  time (clock counter) of the previous call to this function
   */
  static long long t = 0;
  /*
   *  floating time offset to fine tune the scrolling effect
   *  that is caused by the interference between the flash
   *  frequency and the crt refresh
   */
  static long long offset = 0;
  /*
   *  config value that is added to offset each time this function
   *  is called
   */
  static long long o = Preferences::instance()->get_int_value("flash_offset", 50);
  /*
   *  difference between the current and the previous call to
   *  this function in clock ticks
   */
  long long diff;

  if (!enable)
    {
      _enabled = true; // force reset of flash variables
      reset(false);
      return;
    }

  _val = !_val;

  _time = z80->getCounter();
  if (_frame_time == 0)
    {
      offset += o;
      diff = _time - t;
      /*
       *  synchronize flash only if flash frequency is high enough
       *  to change the flash value twice per frame
       *  (128 lines * 112 tics/line)
       */
      if (diff < 14336)
        _val = 0;
      /*
       *  reset flash scrolling offset
       */
      if (offset > 2 * diff)
        offset = 0;
      _frame_time = _time + offset;
    }

  while (242)
    {
      if (_idx >= 255)
	break;
      if ((112 * _idx) > (_time - _frame_time))
	break;
      if (_val)
        set_bit(_idx++, _v3);
      else
        clear_bit(_idx++, _v3);
    }

  t = _time;
}

int
Scanline::get_value(int y)
{
  return test_bit(y, _v1);
}

void
Scanline::reset(bool enable)
{
  _enabled = enable;

  _val = 0;
  _idx = 0;
  _time = 0;
  _count = 0;
  _frame_time = 0;

  for (int a = 0;a < 48;a++)
    _vec[a] = 0;

  _v1 = &_vec[0];
  _v2 = &_vec[16];
  _v3 = &_vec[32];
}
