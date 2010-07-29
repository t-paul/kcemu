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

int
check_file_type(char *name, char *type, unsigned char *buf);

int
check(const char *filename)
{
  int a;
  int len;
  int ret;
  FILE *f;
  int error;
  int block = -1;
  int first = -1;
  int blocks;
  int blocks_ok;
  long offset;
  unsigned char crc;
  unsigned char buf[130];
  char name[100], type[100];

  ret = 1;
  offset = 0;
  blocks = 0;
  blocks_ok = 0;

  f = fopen(filename, "rb");
  if (f == NULL)
    return 0;

  printf("\n* checking file '%s'...\n", filename);

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

      blocks++;
      if (buf[129] != crc)
	ret = 0;
      else
	blocks_ok++;

      error = 0;
      if (block > 0)
	if ((((block + 1) & 0xff) != buf[0]) && (buf[0] != 0xff))
	  {
	    ret = 0;
	    error = 1;
	  }

      block = buf[0];

      if ((block == 0) || (block == 1))
	if (check_file_type(name, type, buf))
	  printf("\n  FILE: '%s' [%s]\n", name, type);

      if (first < 0)
	first = block;

      if ((buf[129] != crc) || error || verbose)
	printf("  BLOCK %3d (%02x), CRC = %02x / %02x [%8ld]%s%s\n",
	       block,
	       block,
	       buf[129],
	       crc,
	       offset,
	       (buf[129] != crc) ? " *** CRC ERROR ***" : "",
	       error ? " *** BLOCK ERROR ***" : "");

      offset += len;
    }

  if (blocks == 0)
    {
      printf("  empty file.\n");
    }
  else
    {
      printf("  first block: %3d (%02x)\n", first, first);
      printf("  last  block: %3d (%02x)\n", block, block);
      printf("  %d blocks, %d ok.\n", blocks, blocks_ok);
    }

  return ret;
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

      if (check(argv[a]))
	printf("  all ok.\n");
      else
	printf("  has errors!\n");
    }

  return 0;
}
