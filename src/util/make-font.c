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

static unsigned char __font[2048];

int
reverse(int byte)
{
  int a, ret;

  ret = 0;
  for (a = 0;a < 8;a++)
    {
      ret <<= 1;
      ret |= byte & 1;
      byte >>= 1;
    }

  return ret;
}

void
print_char(int c)
{
  int a;

  printf("STARTCHAR char%d\n", c);
  printf("ENCODING %d\n", c);
  printf("SWIDTH 1000 0\n");
  printf("DWIDTH 8 0\n");
  printf("BBX 8 8 0 -2\n");
  printf("BITMAP\n");
  for (a = 0;a < 8;a++)
    {
      //printf("%02X\n", reverse(__font[8 * c + a]));
      printf("%02X\n", __font[8 * c + a]);
    }
  printf("ENDCHAR\n");
}

int
main(int argc, char **argv)
{
  FILE *f;
  int a, size, b1, b2;

  size = 8;

  if (argc < 3)
    {
      printf("usage: %s <font file> <font name> <size>\n", argv[0]);
      exit(1);
    }

  f = fopen(argv[1], "rb");
  if (f == NULL)
    {
      printf("can't open file `%s'!\n", argv[1]);
      exit(1);
    }

  if (fread(__font, 1, 2048, f) != 2048)
    {
      printf("could not read 2048 bytes from font file!\n");
      exit(1);
    }

  if (argc >= 4)
    size = atoi(argv[3]);
  if (size < 8)
    size = 8;

  b1 = (size - 8) / 2;
  b2 = (size - 8) - b1;

  printf("STARTFONT 2.1\n"
	 "FONT %s\n"
	 "SIZE %d 72 72\n"
	 "FONTBOUNDINGBOX 8 %d 0 -2\n"
	 "STARTPROPERTIES 9\n"
	 "POINT_SIZE 8\n"
	 "PIXEL_SIZE 1\n"
	 "RESOLUTION_X 72\n"
	 "RESOLUTION_Y 72\n"
	 "FONT_ASCENT %d\n"
	 "FONT_DESCENT %d\n"
	 "AVERAGE_WIDTH 8\n"
	 "SPACING \"M\"\n"
	 "DEFAULT_CHAR 32\n"
	 "ENDPROPERTIES\n"
	 "CHARS 256\n",
	 argv[2], size, size, 7 + b1, 1 + b2);

  for (a = 0;a < 256;a++)
    print_char(a);

  printf("ENDFONT\n");

  return 0;
}
