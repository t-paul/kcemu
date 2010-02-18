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
#include <unistd.h>

typedef struct data
{
  FILE *f;
  const char *filename;
  unsigned char buf[130];
  struct data *next;
} data_t;

static FILE *out = NULL;
static data_t *data = NULL;

int
check(const unsigned char *buf)
{
  int a;
  unsigned char crc;

  crc = 0;
  for (a = 1;a < 129;a++)
    crc += buf[a];

  if (crc == buf[129])
    return 1;

  return 0;
}

int
read_block(void)
{
  int len;
  int block;
  int written;
  data_t *ptr;

  block = -1;
  written = 0;

  for (ptr = data;ptr != NULL;ptr = ptr->next)
    {
      len = fread(ptr->buf, 1, 130, ptr->f);
      if (len == 0)
	return 1;

      if (len != 130)
	{
	  printf("i/o error\n");
	  exit(1);
	}

      if (block < 0)
	{
	  block = ptr->buf[0];
	  printf("BLOCK %3d (%02x) ", block, block);
	}

      if (block != ptr->buf[0])
	{
	  printf("block %d not found in file %s\n", block, ptr->filename);
	  exit(1);
	}

      if (!written && check(ptr->buf))
	{
	  printf("%s", ptr->filename);
	  written = 1;
	  fwrite(ptr->buf, 1, 130, out);
	}
    }

  if (!written)
    {
      printf("no block found with valid crc, using file %s", data->filename);
      fwrite(data->buf, 1, 130, out);
    }

  printf("\n");

  return 0;
}

int
main(int argc, char **argv)
{
  int a;
  data_t *ptr;

  if (argc < 3)
    {
      printf("usage: %s output file [file ...]\n", argv[0]);
      exit(1);
    }

  if (access(argv[1], F_OK) == 0)
    {
      printf("output file already exists, abort.\n");
      exit(1);
    }

  out = fopen(argv[1], "wb");
  if (out == NULL)
    exit(1);

  for (a = 2;a < argc;a++)
    {
      ptr = (data_t *)malloc(sizeof(data_t));
      if (ptr == NULL)
	exit(1);

      ptr->f = fopen(argv[a], "rb");
      ptr->filename = argv[a];
      ptr->next = data;
      data = ptr;
    }

  while (!read_block());

  return 0;
}
