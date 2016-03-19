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
#include <stdlib.h>
#include <string.h>
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

  printf("writing to file '%s'\n", filename);

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

int
c(int byte)
{
  return isprint(byte) ? byte : '.';
}

void
write_byte(int byte)
{
  if (f != NULL)
    fputc(byte, f);

  if (_verbose > 0)
    {
      if (byte == 0x0d)
	printf("\n");
      else
	printf("%c", isprint(byte) ? byte : '.');
      fflush(stdout);
    }

  if (_verbose > 1)
    printf("BYTE: %02x '%c' (%d)\n",
	   byte,
	   isprint(byte) ? byte : '.',
	   _byte_idx);
}

void
handle_byte(int byte)
{
  static int idx = 0;
  static int crc = 0;
  static int state = 0;

  switch (state)
    {
    case 0:
      if (byte == 0x02)
	{
	  idx = 1;
	  state = 1;
	  crc = (0x80 ^ byte);
	  write_byte(byte);
	}
      break;
    case 1:
      idx++;
      crc ^= (0x80 ^ byte);
      write_byte(byte);
      if ((byte == 0x03) && (idx > 2))
	state = 2;
      break;
    case 2:
      state = 0;
      write_byte(byte);
      if (crc == byte)
	printf("\nCRC ok.\n");
      else
	printf("\nCRC ERROR: calculated %02x, in file %02x\n", crc, byte);
      set_state(STATE_NO_SYNC);
      exit(0);
      new_file();
      return;
    }

  set_state(STATE_BIT_SYNC);
}

void
handle_NO_SYNC(float val)
{
  int a;
  float v, p;
  static int idx = 0, cnt = 0;
  static float sum = 0;
  static float buf[10] = { 0.0, };

  v = 0;
  for (a = 0;a < 10;a++)
    v += buf[a];
  v /= 10;

  buf[idx++] = val;
  if (idx >= 10)
    idx = 0;

  p = (100.0 * (v - val)) / v;

  if ((p > 30.0) || (p < -30))
    {
      cnt = 0;
      sum = 0;
    }
  else
    {
      cnt++;
      sum += val;
    }

  if (v > 20 * _lock)
    new_file();
    
  if (_verbose > 1)
    printf("NO SYNC: %6.2f, %6.2f, %10.2f, %d\n", val, v, p, cnt);

  if (cnt > 1000)
    {
      idx = 0;
      cnt = 0;
      for (a = 0;a < 10;a++)
	buf[a] = 0.0;

      _lock = sum / 1000;
      printf(".");
      fflush(stdout);      
      if (_verbose > 1)
	printf("LOCK: %.2f\n", _lock);
      set_state(STATE_SYNC);
    }

#if 0
  if ((4 * val) < 80)
    {
      for (a = 0;a < (4 * v);a++)
	printf(" ");
      printf("#\n");
    }
#endif
}

void
handle_SYNC(float val)
{
  float p;
  static int cnt = 0;

  p = (100.0 * (val - _lock)) / _lock;

  if (_verbose > 1)
    printf("SYNC: %6.2f, %6.2f => %6.2f\n", val, _lock, p);

  if (p > 50.0)
    {
      cnt++;
    }
  else if (p < -30.0)
    {
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

  if (p > 50)
    cnt++;

  if (cnt == 2)
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
      cnt++;
      sum += val;
    }

  l = 2.0 * _lock;
  p = (100.0 * (sum - l)) / l;

  bit = -1;
  if ((cnt == 2) && (p > 50))
    {
      if (_verbose > 1)
	printf("BIT 0: %6.2f %6.2f, %6.2f (%d)\n", val, sum, p, idx);
      cnt = 0;
      bit = 0;
    }
  if (cnt == 4)
    {
      if (_verbose > 1)
	printf("BIT 1: %6.2f %6.2f, %6.2f (%d)\n", val, sum, p, idx);
      cnt = 0;
      bit = 1;
    }

  if (bit < 0)
    return;

  byte >>= 1;
  byte = (byte & 0x1ff) | (bit << 9);

  idx++;
  if (idx == 10)
    {
      idx = 0;
      handle_byte((byte & 0xff) ^ 0x80);
      if ((byte & 0x300) != 0x300)
	printf("BYTE ERROR\n");
    }
}


void
handle_zero_transition(void)
{
  float c = _counter;

  // printf("COUNTER: %d\n", _counter);

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

  _counter = 0;
}

void
handle_sample(long sample)
{
#define LENGTH (3)
  long v;
  int a, b, c;
  static int x = -1;
  static int idx = 0;
  static long buf[LENGTH] = { 0, };

  if (sample < -32000)
    sample = 32000;
  if (sample > 32000)
    sample = 32000;

  buf[idx] = sample;
  idx = (idx + 1) % LENGTH;

  v = 0;
  for (a = 0;a < LENGTH;a++)
    v += buf[a];
  v /= LENGTH;

  v = sample;

  if (_verbose > 2)
    {
      b = 0;
      for (a = 0;a < 78;a++)
	{
	  c = (v + 32768) / (65536 / 78);
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
	  if (b == 2)
	    break;

	  printf(" ");
	}
      printf("\n");
    }

  _counter++;

  if (x < 0)
    {
      if (v > 0)
	{
	  handle_zero_transition();
	  x = 1;
	}
    }
  else
    {
      if (v < 0)
	{
	  handle_zero_transition();
	  x = -1;
	}
    }	  
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
