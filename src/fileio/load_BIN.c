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

#include "fileio/loadP.h"

#define CHECK_SIG(s) \
  ((data[1] == (s)) && (data[2] == (s)) && (data[3] == (s)))
   
static int
loader_BIN_check(const char *filename,
		 unsigned char *data,
		 long size)
{
  int a;
  unsigned char crc;
  
  if (size >= 130)
    {
      if ((data[0] != 0x00) && (data[0] != 0x01))
	return 0;

      crc = 0;
      for (a = 1;a < 129;a++)
	crc += data[a];

      if (crc == data[129])
	return 1;
    }
  
  return 0;
}

static void
copy_blocks(unsigned char *dptr, const unsigned char *sptr, long size)
{
  while (size >= 130)
    {
      memcpy(dptr, sptr, 129);
      dptr += 129;
      sptr += 130;
      size -= 130;
    }
}

static int
loader_BIN_load(const char *filename,
		unsigned char *data,
		long size,
		fileio_prop_t **prop)
{
  int a;
  long xsize;

  *prop = (fileio_prop_t *)malloc(sizeof(fileio_prop_t));
  if (*prop == NULL)
    return -1;
  memset(*prop, 0, sizeof(fileio_prop_t));

  xsize = 129 * (size / 130);

  (*prop)->load_addr = 0;
  (*prop)->start_addr = 0;
  (*prop)->autostart  = 0;

  if (CHECK_SIG(0xd3))
    {
      (*prop)->type = FILEIO_TYPE_BAS;
      memcpy((*prop)->name, data + 4, 8);
      (*prop)->name[8] = '\0';
      for (a = 7;(a > 0) && (*prop)->name[a] == ' ';a--)
	(*prop)->name[a] = '\0';
    }
  else if (CHECK_SIG(0xd4))
    {
      (*prop)->type = FILEIO_TYPE_DATA;
      memcpy((*prop)->name, data + 4, 8);
      (*prop)->name[8] = '\0';
      for (a = 7;(a > 0) && (*prop)->name[a] == ' ';a--)
	(*prop)->name[a] = '\0';
    }
  else if (CHECK_SIG(0xd5))
    {
      (*prop)->type = FILEIO_TYPE_LIST;
      memcpy((*prop)->name, data + 4, 8);
      (*prop)->name[8] = '\0';
      for (a = 7;(a > 0) && (*prop)->name[a] == ' ';a--)
	(*prop)->name[a] = '\0';
    }
  else if (CHECK_SIG(0xd7))
    {
      (*prop)->type = FILEIO_TYPE_PROT_BAS;
      memcpy((*prop)->name, data + 4, 8);
      (*prop)->name[8] = '\0';
      for (a = 7;(a > 0) && (*prop)->name[a] == ' ';a--)
	(*prop)->name[a] = '\0';
    }
  else
    {
      (*prop)->type = FILEIO_TYPE_COM;
      (*prop)->valid = (FILEIO_V_LOAD_ADDR |
			FILEIO_V_START_ADDR |
			FILEIO_V_AUTOSTART);
      (*prop)->load_addr = data[18] | (data[19] << 8);
      (*prop)->start_addr = data[22] | (data[23] << 8);
      (*prop)->autostart  = (data[17] > 2);
      memcpy((*prop)->name, data + 1, 11);
      (*prop)->name[11] = '\0';

      /*
       *  strip trailing COM
       */
      if ((data[8] == 'C') && (data[9] == 'O') && (data[10] == 'M'))
	(*prop)->name[8] = '\0';
    }

  /*
   *  the kc85/1 method to disable autostart is to set
   *  the start address to 0xffff which contains 0xc9 (RET)
   *  of cause this will not catch all cases...
   */
  if ((*prop)->start_addr == 0xffff)
    (*prop)->autostart = 0;
  
  (*prop)->size = xsize;
  (*prop)->data = (unsigned char *)malloc(xsize);
  if ((*prop)->data == 0)
    return -1;

  copy_blocks((*prop)->data, data, size);

  return 0;
}

static const char *
loader_BIN_get_type(void)
{
  return "BIN";
}

static const char *
loader_BIN_get_name(void)
{
  return "loader for binary tape files (block size 130 bytes)";
}

static file_loader_t loader = {
  loader_BIN_check,
  loader_BIN_load,
  loader_BIN_get_type,
  loader_BIN_get_name
};

void loader_BIN_init(void)
{
  fileio_register_loader(&loader);
}
