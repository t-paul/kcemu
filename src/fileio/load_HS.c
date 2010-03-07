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

static void
calc_check_sum(unsigned char *data)
{
  int a;
  int sum;

  sum = 0;
  for (a = 0;a < 34;a += 2)
    sum += (data[a + 1] << 8) | data[a];
  data[34] = sum & 0xff;
  data[35] = (sum >> 8) & 0xff;
}

static int
loader_HS_check(const char *filename,
		unsigned char *data,
		long size)
{
  int load, end;

  /*
   *  HeaderSave:
   *
   *    00h - 01h: load address
   *    02h - 03h: end address
   *    04h - 05h: start address
   *    06h - 0bh: free for additional info
   *          0ch: file type
   *    0dh - 0fh: signature 0d3h, 0d3h, 0d3h
   *    10h - 1fh: file name
   *
   *  file type:
   *
   *    C     - COM file (with autostart)
   *    T     - TEXT file
   *    B     - BASIC program
   *    M     - COM file (no autostart)
   *    S     - SOURCE file (assembler)
   *    P     - PASCAL program
   *    D     - DATA file
   *    I     - introduction, information
   *    E     - EPROM data
   *    SPACE - no type
   *
   */
  if ((data[0x0d] != 0xd3) || (data[0x0e] != 0xd3) || (data[0x0f] != 0xd3))
    return 0;

  load = (data[1] << 8) | data[0];
  end  = (data[3] << 8) | data[2];
  if (end <= load)
    return 0;

  return 1;
}

static int
loader_HS_load(const char *filename,
                 unsigned char *data,
                 long size,
                 fileio_prop_t **prop)
{
  int a, addr, len;
  fileio_prop_t *p;
  unsigned char *src, *dst;

  *prop = (fileio_prop_t *)malloc(sizeof(fileio_prop_t));
  if (*prop == NULL)
    return -1;

  p = *prop;
  memset(p, 0, sizeof(fileio_prop_t));

  p->autostart = 0;
  p->valid = FILEIO_V_LOAD_ADDR;
  p->load_addr  = (data[1] << 8) | data[0];
  switch (data[0x0c])
    {
    case 'C':
      p->autostart = 1;
      /* fall through */
    case 'M':
    case 'X':
    case 'Y':
    case 'Z':
      p->type = FILEIO_TYPE_COM;
      p->valid |= FILEIO_V_START_ADDR | FILEIO_V_AUTOSTART;
      p->start_addr = (data[5] << 8) | data[4];
      break;
    case 'B':
      p->type = FILEIO_TYPE_BAS;
      break;
    default:
      p->type = FILEIO_TYPE_UNKNOWN;
      break;
    }
  memcpy(p->name, &data[0x10], 11);
  p->name[11] = '\0';

  for (a = 10;(a > 0) && (p->name[a] == ' ');a--)
    p->name[a] = '\0';

  p->size = 36 * ((size + 31) / 32);
  p->data = (unsigned char *)malloc(p->size);
  if (p->data == 0)
    return -1;
  memset(p->data, 0, p->size);
  
  src = data;
  dst = p->data;
  dst[0] = 0xe0;
  dst[1] = 0x00;
  memcpy(&dst[2], src, 32);
  calc_check_sum(dst);
  size -= 32;

  addr = p->load_addr;
  while (size > 0)
    {
      src += 32;
      dst += 36;
      dst[0] = addr & 0xff;
      dst[1] = (addr >> 8) & 0xff;
      len = (size < 32) ? size : 32;
      memcpy(&dst[2], src, len);
      calc_check_sum(dst);
      size -= 32;
      addr += 32;
    }

  return 0;
}

static const char *
loader_HS_get_type(void)
{
  return "HeaderSave";
}

static const char *
loader_HS_get_name(void)
{
  return "loader for HeaderSave images";
}

static file_loader_t loader = {
  loader_HS_check,
  loader_HS_load,
  loader_HS_get_type,
  loader_HS_get_name
};

void loader_HS_init(void)
{
  fileio_register_loader(&loader);
}
