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

#include "fileio/load.h"

int
main(int argc, char **argv)
{
  fileio_prop_t *prop;

  if (argc != 3)
    {
      printf("usage: %s infile outfile\n", argv[0]);
      exit(1);
    }

  fileio_init();

  if (fileio_load_file(argv[1], &prop) < 0)
    {
      printf("can't load file '%s'\n", argv[1]);
      return 1;
    }

  if (fileio_save_img_prop(argv[2], prop) < 0)
    {
      printf("can't save file '%s'\n", argv[2]);
      return 1;
    }

  return 0;
}
