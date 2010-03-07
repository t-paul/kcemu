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
check_signature(const char *data)
{
  if (strncmp(data, "KC-TAPE by AF", 13) == 0)
    return 1;
  
  return 0;
}

static int
loader_AF_check(const char *filename,
                unsigned char *data,
                long size)
{
  /*
   *  files from the KC-Emulator by Arne Fitzenreiter
   */
  if (size >= (16 + 128)) /* file signature + first block */
    return check_signature((const char *)&data[1]);
  
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
      prop->type = FILEIO_TYPE_DATA;
      prop->valid = FILEIO_V_NONE;
      memcpy(prop->name, data + 4, 8);
      prop->name[8] = '\0';
    }
  else if ((data[1] == 0xd5) && (data[2] == 0xd5) && (data[3] == 0xd5))
    {
      prop->type = FILEIO_TYPE_LIST;
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
 *  check if we have the last block of the current file
 *
 *  for machine code we simply check the block counter but
 *  basic files seem to be sometimes larger than 255 blocks
 *  so in case of basic files we check for the basic end
 *  marker in the current block (three zero bytes)
 */
static int
check_end(unsigned char *data, int type)
{
  int a;
  int is_last_block = 0;

  switch (type)
    {
    case FILEIO_TYPE_BAS:
    case FILEIO_TYPE_PROT_BAS:
	for (a = 1;a < 127;a++)
	  if ((data[a] | data[a + 1] | data[a + 2]) == 0)
	    is_last_block = 1;
        break;
    default:
        if ((*data) == 0xff)
	  is_last_block = 1;
	break;
    }

  return is_last_block;
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

      if (first)
        {
          first = 0;
	  set_type(data, prop);
        }

      if (check_end(data, prop->type))
        {
          prop->size = len;
          return len;
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
      /*
       *  ignore trailing garbage if we don't find
       *  file header (might be caused by basic files
       *  which often store the last block twice)
       *
       *  so we don't simply bail out here but instead
       *  skip one block of 129 bytes and try to go on...
       */
      if (!check_signature((const char *)&ptr[1]))
        {
	  ptr += 129;
	  size -= 129;
	  continue;
	}

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
loader_AF_get_type(void)
{
  return "AF";
}

static const char *
loader_AF_get_name(void)
{
  return "loader for KC85/3+4 Emulator by Arne Fitzenreiter";
}

static file_loader_t loader = {
  loader_AF_check,
  loader_AF_load,
  loader_AF_get_type,
  loader_AF_get_name
};

void loader_AF_init(void)
{
  fileio_register_loader(&loader);
}
