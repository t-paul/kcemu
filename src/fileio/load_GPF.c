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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fileio/loadP.h"

static int
loader_GPF_check(const char *filename,
                 unsigned char *data,
                 long size)
{
  /*
   *  files from the GEMINI Emulator
   */
  if (size > 128)
    if ((data[2] == 'B') && (data[3] == ':') && (data[7] == 'E') &&
        (data[12] == 'S'))
      {
        return 1;
      }
  
  return 0;
}

static int
loader_GPF_load(const char *filename,
                unsigned char *data,
                long size,
                fileio_prop_t **prop)
{
  int a;
  const char *ptr;

  *prop = (fileio_prop_t *)malloc(sizeof(fileio_prop_t));
  if (*prop == NULL)
    return -1;
  memset(*prop, 0, sizeof(fileio_prop_t));
  
  (*prop)->type = FILEIO_TYPE_COM;
  (*prop)->valid = (FILEIO_V_LOAD_ADDR |
                    FILEIO_V_START_ADDR |
                    FILEIO_V_AUTOSTART);
  (*prop)->load_addr = data[ 4] | (data[ 5] << 8);
  (*prop)->start_addr = data[14] | (data[15] << 8);
  (*prop)->autostart  = 1;

  ptr = strrchr(filename, '/');
  if (ptr)
    ptr++;
  else
    ptr = filename;
  memcpy((*prop)->name, ptr, 11);
  (*prop)->name[11] = '\0';
  for (a = 0;a < 11;a++)
    (*prop)->name[a] = toupper((*prop)->name[a]);

  (*prop)->size = size;
  (*prop)->data = (unsigned char *)malloc(size);
  if ((*prop)->data == 0)
    return -1;

  fill_header_COM((*prop)->data, *prop);
  memcpy((*prop)->data + 128, data + 128, size - 128);

  return 0;
}

static const char *
loader_GPF_get_type(void)
{
  return "GEMINI";
}

static const char *
loader_GPF_get_name(void)
{
  return "loader for GEMINI-Emulator files";
}

static file_loader_t loader = {
  loader_GPF_check,
  loader_GPF_load,
  loader_GPF_get_type,
  loader_GPF_get_name
};

void loader_GPF_init(void)
{
  fileio_register_loader(&loader);
}
