/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: load_AF.c,v 1.5 2001/04/22 22:24:05 tp Exp $
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

#include "fileio/loadP.h"

static int
loader_AF_check(const char *filename,
                unsigned char *data,
                long size)
{
  /*
   *  files from the KC-Emulator by Arne Fitzenreiter
   *  (currently only the first file is loaded!)
   */
  if (size >= (16 + 128)) /* file signature + first block */
    if (strncmp((const char *)&data[1], "KC-TAPE by AF", 13) == 0)
      {
        return 1;
      }

  return 0;
}

static void
set_type(unsigned char *data, fileio_prop_t *prop)
{
  int load, start, autostart;

  load = 0;
  start = 0;
  autostart = 0;

  if ((data[1] == 0xd3) && (data[2] == 0xd3) && (data[3] == 0xd3))
    {
      prop->type = FILEIO_TYPE_BAS;
      prop->valid = FILEIO_V_NONE;
      memcpy(prop->name, data + 4, 8);
      prop->name[8] = '\0';
    }
  else if ((data[1] == 0xd4) && (data[2] == 0xd4) && (data[3] == 0xd4))
    {
      prop->type = FILEIO_TYPE_MINTEX;
      prop->valid = FILEIO_V_NONE;
      memcpy(prop->name, data + 4, 8);
      prop->name[8] = '\0';
    }
  else if ((data[1] == 0xd7) && (data[2] == 0xd7) && (data[3] == 0xd7))
    {
      prop->type = FILEIO_TYPE_PROT_BAS;
      prop->valid = FILEIO_V_NONE;
      memcpy(prop->name, data + 4, 8);
      prop->name[8] = '\0';
    }
  else
    {
      prop->type = FILEIO_TYPE_COM;
      prop->valid = (FILEIO_V_LOAD_ADDR |
                     FILEIO_V_START_ADDR |
                     FILEIO_V_AUTOSTART);
      memcpy(prop->name, data + 1, 11);
      prop->name[11] = '\0';
      load      = data[18] | (data[19] << 8);
      start     = data[22] | (data[23] << 8);
      autostart = (data[17] > 2);
    }
  prop->load_addr  = load;
  prop->start_addr = start;
  prop->autostart  = autostart;
}

/*
 *  return number of bytes used from the original file or -1 on error
 */
static long
fill_prop(unsigned char *data, long size, fileio_prop_t *prop)
{
  long len;
  int first;

  len = 0;
  first = 1;
  while (242)
    {
      if (size < 129)
        return -1;

      len += 129;

      if ((*data) == 0xff)
        {
          prop->size = len;
          return len;
        }
            
      if (first)
        {
          first = 0;
	  set_type(data, prop);
        }

      data += 129;
      size -= 129;
    }
}

/*
 *  return number of bytes used from the original file or -1 on error
 */
static int
do_load(unsigned char *data, unsigned long size, fileio_prop_t *prop)
{
  long len;

  len = fill_prop(data, size, prop);
  if (len < 0)
    return -1;
  
  prop->data = (unsigned char *)malloc(prop->size);
  if (prop->data == NULL)
    return -1;

  memcpy(prop->data, data, prop->size);

  return prop->size;
}

static int
loader_AF_load(const char *filename,
               unsigned char *data,
               long size,
               fileio_prop_t **prop)
{
  int olen;
  unsigned char *ptr;
  fileio_prop_t *tmp, *prop_ptr, *prop_lptr;

  ptr = data;
  prop_ptr = 0;
  prop_lptr = 0;
  while (size > 0)
    {
      ptr += 16;  /* skip header... */
      size -= 16;
      if (size < 0)
        break;

      tmp = (fileio_prop_t *)malloc(sizeof(fileio_prop_t));
      if (tmp == NULL)
        return -1;
      memset(tmp, 0, sizeof(fileio_prop_t));
      
      olen = do_load(ptr, size, tmp);
      if (olen < 0)
        {
          free(tmp);
          *prop = prop_ptr;
          return -1;
        }

      if (prop_ptr == 0)
        prop_ptr = prop_lptr = tmp;
      else
        {
          prop_lptr->next = tmp;
          prop_lptr = prop_lptr->next;
        }
      size -= olen;
      ptr += olen;
    }
  
  *prop = prop_ptr;
  return 0;
}

static const char *
loader_AF_get_name(void)
{
  return "loader for KC85/3+4 Emulator by Arne Fitzenreiter";
}

static file_loader_t loader = {
  loader_AF_check,
  loader_AF_load,
  loader_AF_get_name
};

void loader_AF_init(void)
{
  fileio_register_loader(&loader);
}
