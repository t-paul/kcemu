/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-1998 Torsten Paul
 *
 *  $Id: load_BASIC.c,v 1.2 2000/07/09 21:09:24 tp Exp $
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
#include <string.h>

#include "fileio/loadP.h"

static int
loader_BASIC_check(const char *filename,
                   unsigned char *data,
                   long size)
{
  /*
   *  BASIC
   */
  if (size > 12)
    if ((data[0] == 0xd3) && (data[1] == 0xd3) && (data[2] == 0xd3))
      return 1;
  
  /*
   *  protected BASIC
   */
  if (size > 12)
    if ((data[0] == 0xd7) && (data[1] == 0xd7) && (data[2] == 0xd7))
      return 1;

  return 0;
}

static int
loader_BASIC_load(const char *filename,
                  unsigned char *data,
                  long size,
                  fileio_prop_t **prop)
{
  int b;
  long xsize, len;
  char *sptr, *dptr;

  *prop = (fileio_prop_t *)malloc(sizeof(fileio_prop_t));
  if (*prop == NULL)
    return -1;
  memset(*prop, 0, sizeof(fileio_prop_t));

  switch (*data)
    {
    case 0xd3: (*prop)->type = FILEIO_TYPE_BAS; break;
    case 0xd7: (*prop)->type = FILEIO_TYPE_PROT_BAS; break;
    default: return -1;
    }

  xsize = size + (size + 127) / 128;
  
  (*prop)->valid = FILEIO_V_NONE;
  (*prop)->load_addr = 0;
  (*prop)->start_addr = 0;
  (*prop)->autostart  = 0;
  (*prop)->size = xsize;
  memcpy((*prop)->name, data + 3, 8);
  (*prop)->name[8] = '\0';

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
      memcpy(dptr, sptr, len);
      dptr += 128;
      sptr += 128;
      size -= 128;
    }

  return 0;
}

static const char *
loader_BASIC_get_name(void)
{
  return "loader for standard BASIC files";
}

static file_loader_t loader = {
  loader_BASIC_check,
  loader_BASIC_load,
  loader_BASIC_get_name
};

void loader_BASIC_init(void)
{
  fileio_register_loader(&loader);
}
