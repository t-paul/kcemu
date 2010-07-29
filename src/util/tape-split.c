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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int verbose = 0;

void
split(const char *filename)
{
  int a;
  int len;
  FILE *f, *f2;
  unsigned char crc;
  unsigned char buf[130];
  char fbuf[1000];

  f = fopen(filename, "rb");
  if (f == NULL)
    return;

  printf("\n* splitting file '%s'...\n", filename);

  while (242)
    {
      len = fread(buf, 1, 130, f);
      if (len == 0)
	break;

      if (len != 130)
	{
	  printf("  i/o error\n");
	  exit(1);
	}
      
      crc = 0;
      for (a = 1;a < 129;a++)
	crc += buf[a];

      if (buf[129] != crc)
	continue;

      snprintf(fbuf, 1000, "%04d_%s", buf[0], filename);
      printf("  BLOCK %3d (%02x), CRC = %02x => %s\n",
	     buf[0],
	     buf[0],
	     buf[129],
	     fbuf);

      f2 = fopen(fbuf, "wb");
      if (f2 != NULL)
	{
	  fwrite(buf, 1, 130, f2);
	  fclose(f2);
	}
    }

  fclose(f);
}

int
main(int argc, char **argv)
{
  int a;

  if (argc == 1)
    {
      printf("usage: %s file [file ...]\n", argv[0]);
      exit(1);
    }

  for (a = 1;a < argc;a++)
    {
      if (strcmp("-v", argv[a]) == 0)
	{
	  verbose = 1;
	  continue;
	}

      split(argv[a]);
    }

  return 0;
}
