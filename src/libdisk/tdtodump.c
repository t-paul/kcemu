/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2002 Torsten Paul
 *
 *  $Id$
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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdlib.h>

#include "libdisk/teledisk.h"

int
main(int argc, char **argv)
{
  FILE *out;
  int c, h, s;
  teledisk_prop_t *prop;

  if (argc != 3)
    {
      printf("usage: %s <input file> <output file>\n", argv[0]);
      exit(1);
    }

  prop = teledisk_open(argv[1]);
  if (prop == NULL)
    {
      fprintf(stderr, "can't open `%s'\n", argv[1]);
      exit(1);
    }

  out = fopen(argv[2], "wb");
  if (out == NULL)
    {
      fprintf(stderr, "can't write `%s'\n", argv[2]);
      exit(1);
    }

  printf("%s -> %s\n", argv[1], argv[2]);
  if (prop->comment != NULL)
    printf("\n%s\n\n", prop->comment);

  for (c = 0;c < 80;c++)
    for (h = 0;h < 2;h++)
      for (s = 1;s < 6;s++)
	if (teledisk_read_sector(prop, c, h, s) == 0)
	  {
	    fputc(c, out);
	    fputc(h, out);
	    fputc(c, out);
	    fputc(h, out);
	    fputc(s, out);
	    fputc(3, out);
	    fputc(0, out);
	    fputc(4, out);
	    fwrite(prop->buf, 1, 1024, out);
	  }

  fclose(out);
  teledisk_close(prop);

  return 0;
}
