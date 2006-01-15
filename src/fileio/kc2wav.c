/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: kc2wav.c,v 1.1 2002/10/31 00:48:52 torsten_paul Exp $
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fileio/load.h"

int
main(int argc, char **argv)
{
  int arg_idx = 1;
  fileio_prop_t *prop;

  if ((argc < 3) || (argc > 4))
    {
      printf("usage: %s [-1] infile outfile\n", argv[0]);
      exit(1);
    }

  fileio_init();

  if (strcmp(argv[1], "-1") == 0)
    {
      fileio_set_kctype(FILEIO_KC85_1);
      arg_idx++;
    }

  if (fileio_load_file(argv[arg_idx], &prop) < 0)
    {
      printf("can't load file '%s'\n", argv[arg_idx]);
      return 1;
    }

  if (fileio_save_wav_prop(argv[arg_idx + 1], prop) < 0)
    {
      printf("can't save file '%s'\n", argv[arg_idx + 1]);
      return 1;
    }

  return 0;
}
