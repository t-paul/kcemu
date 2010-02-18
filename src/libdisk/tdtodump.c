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
#include <unistd.h>

#include "libdisk/teledisk.h"

static void
print_underline(const char *text, int c)
{
  int a;
  
  fprintf(stderr, "%s\n", text);
  for (a = 0;a < strlen(text);a++)
    fputc(c, stderr);
  fputc('\n', stderr);
}

static void
usage(const char *program)
{
  fprintf(stderr, "usage: %s [-o filename] [-s] teledisk-dump\n", program);
  exit(1);
}

static void
dump(const char *image_name, const char *out_name, int by_sector)
{
  FILE *out;
  int c, h, s;
  char buf[2048];
  teledisk_prop_t *prop;

  prop = teledisk_open(image_name);
  if (prop == NULL)
    {
      fprintf(stderr, "can't open `%s'\n", image_name);
      exit(1);
    }

  if (!by_sector)
    {
      out = out_name == NULL ? stdout : fopen(out_name, "wb");
      if (out == NULL)
        {
          fprintf(stderr, "can't write `%s'\n", out_name);
          exit(1);
        }
    }

  snprintf(buf, sizeof(buf), "Teledisk file `%s'", prop->filename);
  print_underline(buf, '=');
  fprintf(stderr, "Id           : %s\n", prop->id);
  fprintf(stderr, "Version      : %d.%d\n", prop->version_major, prop->version_minor);
  fprintf(stderr, "Volume       : %d\n", prop->volume);
  fprintf(stderr, "Check-Byte   : %d (%02xh)\n", prop->signature, prop->signature);
  fprintf(stderr, "Density      : %s\n", prop->density);
  fprintf(stderr, "Drive-Type   : %s\n", prop->drive_type);
  fprintf(stderr, "Dos-Mode     : %s\n", prop->dos_mode ? "Yes" : "No");
  fprintf(stderr, "Heads        : %d\n", prop->heads);
  fprintf(stderr, "Cylinders    : %d\n", prop->cylinders);
  fprintf(stderr, "Sectors      : %d\n", prop->sectors);

  if (prop->sector_size < 0)
    fprintf(stderr, "Sector-Size  : variable\n");
  else
    fprintf(stderr, "Sector-Size  : %d\n", prop->sector_size);

  if (prop->comment != NULL)
    {
      fprintf(stderr, "Comment-Date : %s\n", prop->comment_date);
      fprintf(stderr, "Comment      : %s\n", prop->comment);
    }
  fprintf(stderr, "\n");

  int old_size = 0;
  for (c = 0; c < prop->cylinders; c++)
    {
      for (h = 0; h < prop->heads; h++)
        {
          for (s = 1; s < prop->sectors; s++)
            {
              int size = teledisk_read_sector(prop, c, h, s);
              if (size < 0)
                {
                  continue;
                }
              else if ((old_size > 0) && (old_size != size))
                {
                  fprintf(stderr, "Sector Size Change %d -> %d at %d/%d/%d (C/H/S)\n", old_size, size, c, h, s);
                }
              old_size = size;

              if (by_sector)
                {
                  if (out_name)
                    snprintf(buf, sizeof(buf), "%s_c%02dh%02ds%02d", out_name, c, h, s);
                  out = out_name == NULL ? stdout : fopen(buf, "wb");
                }
              //fputc(c, out);
              //fputc(h, out);
              //fputc(c, out);
              //fputc(h, out);
              //fputc(s, out);
              //fputc(3, out);
              //fputc(0, out);
              //fputc(4, out);
              fwrite(prop->buf, 1, size, out);

              if (by_sector)
                {
                  fclose(out);
                  out = NULL;
                }
            }
        }
    }
  fprintf(stderr, "\n");
  fprintf(stderr, "\n");

  if (out != NULL)
    fclose(out);
  teledisk_close(prop);
}

int
main(int argc, char **argv)
{
  int opt;
  int by_sector = 0;
  char *out_name = NULL;

  while ((opt = getopt(argc, argv, "so:")) != -1)
    {
      switch (opt)
        {
        case 's':
          by_sector = 1;
          break;
        case 'o':
          out_name = strdup(optarg);
          break;
        default:
          usage(argv[0]);
        }
    }

  if ((optind + 1) != argc)
    usage(argv[0]);

  dump(argv[optind], out_name, by_sector);

  return 0;
}
