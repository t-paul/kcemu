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
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>

#include "libaudio/libaudio.h"

#define STATE_NO_SYNC  (0)
#define STATE_SYNC     (1)
#define STATE_BIT      (2)
#define STATE_BIT_SYNC (3)

static int _state;
static int _counter;
static float _lock = 1e20;
static FILE *f = NULL;
static char filename[100];
static int filename_idx = 0;

static int _verbose = 0;
static int _byte_idx;
static unsigned char _buf[130];

void
new_file(void)
{
  while (242)
    {
      snprintf(filename, 100, "output-%04d.img", filename_idx++);
      if (access(filename, F_OK) != 0)
	break;

      if (filename_idx > 9999)
	{
	  printf("out of filenames!\n");
	  exit(1);
	}
    }

  if (f != NULL)
    fclose(f);

  f = fopen(filename, "wb");
  if (f == NULL)
    {
      printf("can't write file '%s'\n", filename);
      exit(1);
    }

  printf("- writing to file '%s'\n", filename);

  _lock = 1e20;
}

void
set_state(int state)
{
  if (_verbose > 1)
    printf("set_state(): %d\n", state);

  switch (state)
    {
    case STATE_NO_SYNC:
      _byte_idx = 0;
      memset(_buf, 0, 130);
      _buf[129] = 0xff;
      break;
    case STATE_SYNC:
      break;
    case STATE_BIT:
      break;
    case STATE_BIT_SYNC:
      break;
    }
  _state = state;
}

void
handle_block(void)
{
  int a, b;
  unsigned char crc;
  char name[100], type[100];

  if (_byte_idx == 0)
    return;

  if (f != NULL)
    {
      fwrite(_buf, 1, 130, f);
      fflush(f);
    }

  crc = 0;
  for (a = 1;a < 129;a++)
    crc += _buf[a];
  
  if ((_buf[0] == 0) || (_buf[0] == 1))
    if (check_file_type(name, type, _buf))
      printf("*\n* FILE: '%s' [%s]\n*\n", name, type);

  printf("+ BLOCK: %3d (%02x) [%3d], CRC = %02x / %02x%s%s\n",
	 _buf[0],
	 _buf[0],
	 _byte_idx,
	 _buf[129],
	 crc,
	 (_buf[129] != crc) ? " *** CRC ERROR ***" : "",
	 (_byte_idx != 130) ? " *** INCOMPLETE DATA ***" : "");

  if (_verbose > 0)
    for (a = 0;a < 8;a++)
      {
	printf("%02x:", 16 * a);
	for (b = 0;b < 16;b++)
	  {
	    printf(" %02x", _buf[16 * a + b + 1]);
	    if (b == 7)
	      printf("  ");
	  }
	printf(" | ");
	for (b = 0;b < 16;b++)
	  {
	    printf("%c", isprint(_buf[16 * a + b + 1]) ? _buf[16 * a + b + 1] : '.');
	    if (b == 7)
	      printf(" ");
	  }
	printf("\n");
      }

  _byte_idx = 0;
}

void
handle_byte(int byte)
{
  if (_verbose > 1)
    printf("BYTE: %02x '%c' (%d)\n",
	   byte,
	   isprint(byte) ? byte : '.',
	   _byte_idx);
  
  _buf[_byte_idx++] = byte;

  if (_byte_idx == 130)
    {
      handle_block();
      set_state(STATE_NO_SYNC);
    }
  else
    set_state(STATE_BIT_SYNC);
}

void
handle_NO_SYNC(float val)
{
  int a;
  float v, p;
  static int idx = 0, cnt = 0;
  static float buf[10] = { 0.0, };

  v = 0;
  for (a = 0;a < 10;a++)
    v += buf[a];
  v /= 10;

  buf[idx++] = val;
  if (idx >= 10)
    idx = 0;

  p = (100.0 * (v - val)) / v;

  if (p > 25.0)
    cnt = 0;
  else if (p < -25.0)
    cnt = 0;
  else
    cnt++;

  if (v > 20 * _lock)
    new_file();
    
  if (_verbose > 1)
    printf("NO SYNC: %6.2f, %6.2f, %10.2f, %d\n", val, v, p, cnt);

  if (cnt > 100)
    {
      idx = 0;
      cnt = 0;
      for (a = 0;a < 10;a++)
	buf[a] = 0.0;

      _lock = v;
      if (_verbose > 1)
	printf("LOCK: %.2f\n", _lock);
      set_state(STATE_SYNC);
    }
}

void
handle_SYNC(float val)
{
  float p;
  static int cnt = 0;

  p = (100.0 * (val - _lock)) / _lock;

  if (_verbose > 1)
    printf("SYNC: %6.2f, %6.2f => %6.2f\n", val, _lock, p);

  if (p > 30.0)
    {
      cnt++;
    }
  else if (p < -30.0)
    {
      handle_block();
      set_state(STATE_NO_SYNC);
    }
  else
    {
      cnt = 0;
    }

  if (cnt == 2)
    {
      cnt = 0;
      set_state(STATE_BIT);
      return;
    }
}

void
handle_BIT_SYNC(float val)
{
  float p;
  static int cnt = 0;

  p = (100.0 * (val - _lock)) / _lock;

  if (_verbose > 1)
    printf("BIT SYNC: %6.2f, %6.2f => %6.2f\n", val, _lock, p);

  if (p < -30.0)
    {
      handle_block();
      set_state(STATE_NO_SYNC);
      return;
    }

  if (cnt == 0)
    {
      cnt++;
    }
  else
    {
      cnt = 0;
      set_state(STATE_BIT);
    }
}

void
handle_BIT(float val)
{
  int bit;
  float p, l;
  static float sum;
  static int cnt = 0;
  static int idx = 0;
  static int byte = 0;

  if (cnt == 0)
    {
      sum = val;
      cnt++;
      return;
    }
  else
    {
      sum += val;
      cnt = 0;
    }

  l = 2.0 * _lock;
  p = (100.0 * (sum - l)) / l;

  if (p < -18.0)
    bit = 0;
  else if (p > 60.0)
    bit = -1; // STOP-BIT
  else
    bit = 128;

  if (_verbose > 1)
    printf("BIT: %6.2f, %6.2f => %3d (%d)\n", sum, p, bit, idx);

  byte >>= 1;
  byte = (byte & 0x7f) | (bit & 0x80);

  idx++;
  if (idx == 8)
    {
      idx = 0;
      handle_byte(byte);
    }
}

void
handle_zero_transition(int tics)
{
  static float c = 0;
  
  c = tics;

  switch (_state)
    {
    case STATE_NO_SYNC:
      handle_NO_SYNC(c);
      break;
    case STATE_SYNC:
      handle_SYNC(c);
      break;
    case STATE_BIT:
      handle_BIT(c);
      break;
    case STATE_BIT_SYNC:
      handle_BIT_SYNC(c);
      break;
    }
}

void
show_sample(int sample, int fill, const char *text)
{
  int a, b, c;

  b = 0;
  for (a = 0;a < 78;a++)
    {
      c = (sample + 32768) / (65536 / 78);
      if (a == c)
	{
	  printf("#");
	  b++;
	  continue;
	}
      if (a == (78 / 2))
	{
	  printf("|");
	  b++;
	  continue;
	}
      if (!fill && (b == 2))
	break;
      
      printf(" ");
    }
  printf(text);
}

int
find_zero_transition(int *buf, int len, int min, int max)
{
  int a;
  int zero = (min + max) / 2;

  if (min == 40000)
    return 0;

  for (a = 0;a < len;a++)
    {
      if ((buf[a] <= zero) && (buf[a + 1] > zero))
	return len - a;
      if ((buf[a] >= zero) && (buf[a + 1] < zero))
	return len - a;
    }

  return len / 2;
}

void
handle_sample(int sample)
{
#define LENGTH (3)
  long v;
  int a;
  static int x = -1;
  static int idx = 0;
  static long buf[LENGTH] = { 0, };

  if (sample < -32000)
    sample = -32000;
  if (sample > 32000)
    sample = 32000;

  buf[idx] = sample;
  idx = (idx + 1) % LENGTH;

  v = 0;
  for (a = 0;a < LENGTH;a++)
    v += buf[a];
  v /= LENGTH;

  if (_verbose > 2)
    show_sample(v, 0, "\n");

  _counter++;

  if (x < 0)
    {
      if (v > 0)
	{
	  handle_zero_transition(_counter);
	  x = 1;
	  _counter = 0;
	}
    }
  else
    {
      if (v < 0)
	{
	  handle_zero_transition(_counter);
	  x = -1;
	  _counter = 0;
	}
    }	  
}

void
handle_sample2(int sample)
{
#define LEN (1000)

  int a, z;
  int tics = -1;
  static int x = 1;
  static int idx = 0;
  static int cnt = 0;
  static int min =  40000;
  static int max = -40000;
  static int counter = 0;
  static int old_counter = 0;

  static int buf[LEN];

  counter++;

  for (a = LEN - 1;a > 0;a--)
    buf[a] = buf[a - 1];
  buf[0] = sample;

  if (_verbose > 2)
    show_sample(sample, 1, "");
  if (_verbose > 1)
    printf("%8d, %8d, %8d, %8d (%8d) - %d %8d / %8d ", cnt, old_counter, counter, sample, idx, x, min, max);

  idx++;
  if (x)
    {
      if (sample > max)
	{
	  cnt = 0;
	  max = sample;
	}
      else
	{
	  cnt++;
	  if ((max - sample) > 20000)
	    {
	      z = find_zero_transition(buf, idx, min, max);
	      tics = 0;

	      if (_verbose > 1)
		printf("max %d, %d", tics, z);
	      min = 40000;
	      idx = 0;
	      x = 0;
	    }
	}
    }
  else
    {
      if (sample < min)
	{
	  cnt = 0;
	  min = sample;
	}
      else
	{
	  cnt++;
	  if ((sample - min) > 20000)
	    {
	      z = find_zero_transition(buf, idx, min, max);
	      tics = 0;

	      if (_verbose > 1)
		printf("min %d, %d", tics, z);
	      max = -40000;
	      idx = 0;
	      x = 1;
	    }
	}
    }

  if (_verbose > 1)
    printf("\n");

  //if (tics > 0)
  //handle_zero_transition(tics);
}

void
fastloader(int counter)
{
  int bit;
  float v;
  static int x = 0;
  static int cnt = 0;
  static int state = 0;
  static int crc = 0xff;
  static int byte = 0;
  static int byte_cnt = 0;
  static int lock = 0;
  static int old_counter = 0;
  static float sum = 0;
  static int block = 0;

  bit = counter >= lock ? : 0;

  if (bit == 1)
    {
      x = 0;
      if (_verbose > 2)
	printf(" => %d | %4d | %d", state, counter, bit);
    }
  else
    {
      if (x == 0)
	{
	  if (_verbose > 2)
	    printf(" => %d | %4d | ?", state, counter);
	  x = counter;
	  return;
	}

      if (_verbose > 2)
	printf(" => %d | %4d | %d", state, x + counter, bit);

      x = 0;
    }

  switch (state)
    {
    case 0:
      v = (counter - old_counter);
      v = (100.0 * v) / old_counter;
      old_counter = counter;

      if (v < 20)
	{
	  cnt++;
	  sum += counter;
	}
      else
	{
	  cnt = 0;
	  sum = 0;
	}
      
      if (cnt > 200)
	{
	  cnt = 0;
	  state = 1;
	  block = 0;
	  lock = (int)((3 * (sum / 200.0)) / 4 + 0.5);
	  printf("LOCK: %.2f => %d\n", sum / 200.0, lock);
	  sum = 0;
	}
      break;
    case 1:
      if ((cnt > 0) && (bit == 1))
	{
	  cnt = 0;
	  state = 0;
	}
      else
	{
	  if (bit == 0)
	    cnt++;
	}

      if (cnt == 2)
	{
	  cnt = 0;
	  byte = 0;
	  state = 2;
	  byte_cnt = 0;
	}
      break;
    case 2:
      cnt++;
      byte >>= 1;
      byte |= (bit == 1) ? 256 : 0;
      if (cnt == 9)
	{
	  byte_cnt++;
	  if ((byte_cnt > 1) && (((byte_cnt - 1) % 17) == 0))
	    {
	      printf("CRC = %02x, calculated: %02x (byte_cnt = %d, block = %d) %s\n",
		     byte / 2,
		     crc,
		     byte_cnt,
		     block,
		     (byte / 2) == crc ? "OK" : "*** ERROR ***");
	      if (byte_cnt == 137)
		{
		  block++;
		  if (f != NULL)
		    {
		      fputc((crc + 1) % 0xff, f);
		      fflush(f);
		    }
		  state = 1;
		  crc = 0xff;
		}
	    }
	  else
	    {
	      if (byte_cnt > 1)
		crc = (crc + (byte / 2)) & 0xff;
	      if (f != NULL)
		{
		  fputc((byte / 2) & 0xff, f);
		  fflush(f);
		}
	      
	      printf(" BYTE %02x (%d)", (byte / 2) & 0xff, byte_cnt);
	      printf(" %c", isprint((byte / 2) & 0xff) ? (byte / 2) & 0xff : '.');
	      if (byte & 1)
		printf(" [ERROR]");
	      printf("\n");
	    }
	  cnt = 0;
	  byte = 0;
	}
      break;
    }
}

void
handle_sample3(int sample)
{
  int a;
  float v;
  static float vv;
  static int x = -1;
  static int idx = 0;
  static long buf[LENGTH] = { 0, };
  static int old_sample = 0;

  //vv += (sample - old_sample);
  //vv *= 0.8;

  v = sample;
  if (v > 30000)
    v = 30000;
  if (v < -30000)
    v = -30000;

  if (_verbose > 2)
    {
      show_sample((int)v, 1, "");
      printf("%10.2f", v);
    }

  _counter++;

  if (x < 0)
    {
      if (v > 0)
	{
	  fastloader(_counter);
	  x = 1;
	  _counter = 0;
	}
    }
  else
    {
      if (v < 0)
	{
	  fastloader(_counter);
	  x = -1;
	  _counter = 0;
	}
    }

  old_sample = sample;

  if (_verbose > 2)
    printf("\n");
}

int
main(int argc, char **argv)
{
  int a, c, channels;
  libaudio_prop_t *prop;

  libaudio_init(LIBAUDIO_TYPE_ALL);

  new_file();

  _counter = 0;
  set_state(STATE_NO_SYNC);

  for (a = 1;a < argc;a++)
    {
      if (strcmp("-v", argv[a]) == 0)
	{
	  _verbose++;
	  continue;
	}

      prop = libaudio_open(argv[a]);
      channels = libaudio_get_channels(prop);
      while (242)
	{
	  c = libaudio_read_sample(prop);
	  if (c == EOF)
	    break;
	  
	  if (channels == 2)
	    {
	      c += libaudio_read_sample(prop);
	      c /= 2;
	    }

	  handle_sample(c - 32768);
	}
      libaudio_close(prop);
    }
  
  return 0;
}
