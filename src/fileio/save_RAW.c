/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: save_RAW.c,v 1.1 2002/10/31 00:48:52 torsten_paul Exp $
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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fileio/loadP.h"

char *
make_filename(const char *filename, int load, int start)
{
  int a;
  char *buf, *ptr;

  if ((load < 0) && (start >= 0))
    load = 0;

  buf = (char *)malloc(strlen(filename) + 20);
  ptr = strrchr(filename, '.');
  if (ptr == NULL)
    {
      strcpy(buf, filename);
      if (load >= 0)
	{
	  strcat(buf, "_0x");
	  snprintf(buf + strlen(buf), 5, "%04x", load);
	}
      if (start >= 0)
	{
	  strcat(buf, "_0x");
	  snprintf(buf + strlen(buf), 5, "%04x", start);
	}
      strcat(buf, ".img");
    }
  else
    {
      a = ptr - filename;
      strncpy(buf, filename, a);
      buf[a] = '\0';
      if (load >= 0)
	{
	  strcat(buf, "_0x");
	  snprintf(buf + strlen(buf), 5, "%04x", load);
	}
      if (start >= 0)
	{
	  strcat(buf, "_0x");
	  snprintf(buf + strlen(buf), 5, "%04x", start);
	}
      strcat(buf, ptr);
    }
  
  return buf;
}

int
fileio_save_raw(const char *filename,
		const unsigned char *data,
		int size,
		int load,
		int start)
{
  int a;
  int len;
  FILE *out;
  char *name;

  name = make_filename(filename, load, start);
  out = fopen(name, "wb");
  free(name);

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
fileio_save_raw_prop(const char *filename, fileio_prop_t *prop)
{
  int load = -1;
  int start = -1;

  if (prop->valid & FILEIO_V_LOAD_ADDR)
    load = prop->load_addr;

  if (prop->valid & FILEIO_V_START_ADDR)
    start = prop->start_addr;

  return fileio_save_raw(filename, prop->data, prop->size, load, start);
}
