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

static long _x = 0;
static long buf[40];
static long diff[40];

void
x()
{
  int a, z;
  static int zz = 0;
  static float val = 0;

  z = 0;
  for (a = 1;a < 10;a++)
    {
      z += diff[20 - a];
      z += diff[20 + a];
    }

  val += z - zz;
  val *= .8;
      
  printf("%ld %ld %.2f\n", _x++, 0, val);
}

void
handle_sample2(long sample)
{
  int a;

  for (a = 1;a < 40;a++)
    buf[a - 1] = buf[a];
  
  buf[39] = sample;

  for (a = 1;a < 40;a++)
    diff[a - 1] = buf[a] - buf[a - 1];

  x();
}

int
main(int argc, char **argv)
{
  int a, c, channels;
  libaudio_prop_t *prop;

  libaudio_init(LIBAUDIO_TYPE_ALL);

  for (a = 1;a < argc;a++)
    {
      prop = libaudio_open(argv[a]);
      while (242)
	{
	  c = libaudio_read_sample(prop);
	  if (c == EOF)
	    break;
	  
	  handle_sample2(c - 32768);
	}
      libaudio_close(prop);
    }
  
  return 0;
}
