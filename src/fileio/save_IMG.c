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
#include <stdlib.h>
#include <string.h>

#include "fileio/loadP.h"

int
fileio_save_img(const char *filename, const unsigned char *data, int size)
{
  int a;
  int len;
  FILE *out;

  out = fopen(filename, "wb");
  if (out == NULL)
    return 1;

  len = size;
  while (len > 0)
    {
      a = 128;
      if (len < a)
	a = len;

      data++;
      if (fwrite(data, 1, 128, out) != 128)
	return 1;
      data += 128;
      len -= 129;
    }
  
  fclose(out);

  return 0;
}

int
fileio_save_img_prop(const char *filename, fileio_prop_t *prop)
{
  return fileio_save_img(filename, prop->data, prop->size);
}
