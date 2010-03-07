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

static int
check_addr(unsigned char *data, long size)
{
  unsigned short load, end, start, x;
  
  /*
   *  finally look if specified addresses are plausible
   */
  x     = data[16];
  load  = data[17] | (data[18] << 8);
  end   = data[19] | (data[20] << 8);
  start = data[21] | (data[22] << 8);

  if (load >= end)
    return 0;

  if ((end - load - 1) > size)
    return 0;

  if (x > 2)
    if ((start < load) || (start >= end))
      return 0;

  return 1;
}

static int
loader_TAPE_check(const char *filename,
                  unsigned char *data,
                  long size)
{
  /*
   *  standard kc tape image (128 bytes/block no checksum)
   *  This is somewhat tricky because we don't have a simple
   *  signature to check against. Instead we test some
   *  constraints that should detect most of the tape images.
   */
  if (size > 128)
    if ((data[16] >= 2) && (data[16] <= 0x0a))
      return check_addr(data, size - 128);
  
  return 0;
}

static int
loader_TAPE_load(const char *filename,
                 unsigned char *data,
                 long size,
                 fileio_prop_t **prop)
{
  long xsize;

  *prop = (fileio_prop_t *)malloc(sizeof(fileio_prop_t));
  if (*prop == NULL)
    return -1;
  memset(*prop, 0, sizeof(fileio_prop_t));

  xsize = 129 * ((size + 127) / 128);
  
  (*prop)->type = FILEIO_TYPE_COM;
  (*prop)->valid = (FILEIO_V_LOAD_ADDR |
                    FILEIO_V_START_ADDR |
                    FILEIO_V_AUTOSTART);
  (*prop)->load_addr = data[17] | (data[18] << 8);
  (*prop)->start_addr = data[21] | (data[22] << 8);
  (*prop)->autostart  = (data[16] > 2);
  memcpy((*prop)->name, data, 11);
  (*prop)->name[11] = '\0';

  /*
   *  strip trailing COM
   */
  if ((data[8] == 'C') && (data[9] == 'O') && (data[10] == 'M'))
    (*prop)->name[8] = '\0';

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

  *(*prop)->data = 0; // block number

  /*
   *  FIXME: fill_header_COM() doesn't handle KC85/1 filenames correctly
   */
  //fill_header_COM((*prop)->data + 1, *prop);
  //fileio_copy_blocks((*prop)->data + 129, data + 128, size - 128, 1);

  switch (fileio_get_kctype())
    {
    case FILEIO_KC85_1:
      fileio_copy_blocks((*prop)->data, data, size, 0);
      break;
    case FILEIO_KC85_3:
      fileio_copy_blocks((*prop)->data, data, size, 1);
      break;
    case FILEIO_Z1013:
      return -1;
    }

  return 0;
}

static const char *
loader_TAPE_get_type(void)
{
  return "TAPE";
}

static const char *
loader_TAPE_get_name(void)
{
  return "loader for raw KC-Tape images";
}

static file_loader_t loader = {
  loader_TAPE_check,
  loader_TAPE_load,
  loader_TAPE_get_type,
  loader_TAPE_get_name
};

void loader_TAPE_init(void)
{
  fileio_register_loader(&loader);
}
