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

/*
 *  raw basic files as found on www.robotron-net.de
 *  in fact there are simply the first 11 bytes missing
 *  those contain the basic marker bytes (3 times 0xd3) and
 *  the filename (8 bytes padded with spaces)
 */
static int
loader_SSS_check(const char *filename,
		 unsigned char *data,
		 long size)
{
  int len;
  const char *name;

  len = data[0] | (data[1] << 8);

  /* too restricting...
  if (size != ((len + 127) & ~127))
    return 0;
  */

  if (len > size)
    return 0;
  if ((len + 129) < size)
    return 0;

  if (data[3] != 4)
    return 0;

  len = strlen(filename);
  if (len >= 4)
    {
      name = filename + len - 4;
      if (strcmp(name, ".sss") == 0)
	return 1;
      if (strcmp(name, ".SSS") == 0)
	return 1;
    }

  return 0;
}

static void
loader_SSS_set_filename(unsigned char *data, const char *filename)
{
  int a, c;
  const char *name;
  static const char *allowed_chars =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ����0123456789+-()";

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
loader_SSS_load(const char *filename,
		unsigned char *data,
		long size,
		fileio_prop_t **prop)
{
  int b;
  long xsize, len;
  unsigned char *sptr, *dptr;

  *prop = (fileio_prop_t *)malloc(sizeof(fileio_prop_t));
  if (*prop == NULL)
    return -1;
  memset(*prop, 0, sizeof(fileio_prop_t));

  size += 11;
  xsize = size + (size + 127) / 128;
  
  (*prop)->type = FILEIO_TYPE_BAS;
  (*prop)->valid = FILEIO_V_NONE;
  (*prop)->load_addr = 0;
  (*prop)->start_addr = 0;
  (*prop)->autostart  = 0;
  (*prop)->size = xsize;
  loader_SSS_set_filename((*prop)->name, filename);

  (*prop)->data = (unsigned char *)malloc(xsize);
  if ((*prop)->data == 0)
    return -1;

  sptr = data;
  dptr = (*prop)->data;

  b = 1;
  while (size > 0)
    {
      *dptr++ = b++;
      len = (size > 128) ? 128 : size;

      if (b == 2) // first block as b is incremented above !!!
	{
	  memset(dptr, 0xd3, 3);
	  memset(dptr + 3, 0x20, 8);
	  memcpy(dptr + 3, (*prop)->name, strlen((char *)(*prop)->name));
	  dptr += 11;
	  len -= 11;
	}

      memcpy(dptr, sptr, len);
      dptr += len;
      sptr += len;
      size -= 128;
    }

  return 0;
}

static const char *
loader_SSS_get_type(void)
{
  return "BASIC";
}

static const char *
loader_SSS_get_name(void)
{
  return "loader for raw BASIC files (*.sss)";
}

static file_loader_t loader = {
  loader_SSS_check,
  loader_SSS_load,
  loader_SSS_get_type,
  loader_SSS_get_name
};

void loader_SSS_init(void)
{
  fileio_register_loader(&loader);
}
