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

int
check_file_type(char *name, char *type, unsigned char *buf);

static int _verbose = 0;

void
do_rename(const char *filename)
{
  FILE *f;
  int a, len;
  unsigned char crc;
  unsigned char buf[130];
  char name[100], type[100];

  f = fopen(filename, "rb");
  if (f == NULL)
    return;

  len = fread(buf, 1, 130, f);
  if (len != 130)
    {
      printf("# i/o error reading first block of file '%s'\n", filename);
      return;
    }

  crc = 0;
  for (a = 1;a < 129;a++)
    crc += buf[a];

  if (crc != buf[129])
    {
      printf("# crc error: in file %02xh, calculated %02xh\n", buf[129], crc);
      return;
    }

  if (check_file_type(name, type, buf))
    printf("mv '%s' '%s' # type = %s\n", filename, name, type);
  else
    printf("# can't determine type of file '%s'\n", filename);
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
	  _verbose++;
	  continue;
	}

      do_rename(argv[a]);
    }

  return 0;
}
