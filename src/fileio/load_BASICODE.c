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
#include <string.h>
#include <stdlib.h>

#include "fileio/loadP.h"

static int
loader_BASICODE_check(const char *filename,
		      unsigned char *data,
		      long size)
{
  /*
   *  BASICODE
   */
  if (size >= 5)
    if (memcmp(data, "1000 ", 5) == 0)
      return 1;

  return 0;
}

static void
loader_BASICODE_set_filename(unsigned char *data, const char *filename)
{
  int a, c;
  const char *name;
  static const char *allowed_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ����0123456789+-()";

  name = strrchr(filename, '/');
  if (name == NULL)
    name = filename;

  a = 0;
  while (a < 8)
    {
      if (*name == '\0')
	break;
      c = toupper(*name++);
      if (c == '.')
	break;
      if (strchr(allowed_chars, c) != NULL)
	data[a++] = c;
    }
  data[a] = '\0';
}

static int
loader_BASICODE_load(const char *filename,
		     unsigned char *data,
		     long size,
		     fileio_prop_t **prop)
{
  int newline;
  long a, xsize;
  unsigned char c, *sptr, *dptr;

  *prop = (fileio_prop_t *)malloc(sizeof(fileio_prop_t));
  if (*prop == NULL)
    return -1;
  memset(*prop, 0, sizeof(fileio_prop_t));

  (*prop)->type = FILEIO_TYPE_BASICODE;
  (*prop)->valid = FILEIO_V_NONE;
  (*prop)->load_addr = 0;
  (*prop)->start_addr = 0;
  (*prop)->autostart  = 0;
  loader_BASICODE_set_filename((*prop)->name, filename);

  (*prop)->data = (unsigned char *)malloc(size);
  if ((*prop)->data == 0)
    return -1;

  sptr = data;
  dptr = (*prop)->data;
  newline = xsize = 0;
  for (a = 0;a < size;a++)
    {
      c = *sptr++;

      /*
       *  basicode newline is 0dh
       */
      if (c == 0x0a)
	c = 0x0d;

      /*
       *  remove multiple newlines
       */
      if (c == 0x0d)
	{
	  if (newline)
	    continue;
	  else
	    newline = 1;
	}
      else
	{
	  newline = 0;
	}

      *dptr = c;
      dptr++;
      xsize++;
    }

  (*prop)->size = xsize;
  return 0;
}

static const char *
loader_BASICODE_get_type(void)
{
  return "BASICODE";
}

static const char *
loader_BASICODE_get_name(void)
{
  return "loader for BASICODE files (ASCII)";
}

static file_loader_t loader = {
  loader_BASICODE_check,
  loader_BASICODE_load,
  loader_BASICODE_get_type,
  loader_BASICODE_get_name
};

void
loader_BASICODE_init(void)
{
  fileio_register_loader(&loader);
}
