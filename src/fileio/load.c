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
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include "fileio/loadP.h"

static fileio_kctype_t kctype;
static file_loader_t *loaders[MAX_FILE_LOADERS];

void
fileio_init(void)
{
  fileio_set_kctype(FILEIO_KC85_3);
  memset(loaders, 0, sizeof(loaders));

  /*
   *  order is importend
   *  first check for types with unique signatures
   *  unsafe guesses come last
   */
  loader_AF_init();
  loader_GPF_init();
  loader_BASIC_init();
  loader_BASICODE_init();
  loader_SSS_init();
  loader_HS_init();
  loader_BIN_init();
  loader_TAPE_init();
  loader_UNKNOWN_init(); /* fallback */
}

fileio_kctype_t
fileio_get_kctype(void)
{
  return kctype;
}

void
fileio_set_kctype(fileio_kctype_t type)
{
  kctype = type;
}

int
fileio_register_loader(file_loader_t *loader)
{
  int a;

  for (a = 0;a < MAX_FILE_LOADERS;a++)
    if (loaders[a] == NULL)
      {
        loaders[a] = loader;
        return 1;
      }
  return 0;
}

int
fileio_load_file(const char *filename, fileio_prop_t **prop)
{
  int a, ret;
  FILE *f;
  unsigned char *data;
  struct stat stat_buf;

  if (filename == NULL)
    return -1;
  
  if (prop == NULL)
    return -1;

  *prop = NULL;

  if (stat(filename, &stat_buf) != 0)
    return -1;

  if (!S_ISREG(stat_buf.st_mode))
    return -1;

  data = (unsigned char *)malloc(stat_buf.st_size);
  if (data == 0)
    return -1;

  f = fopen(filename, "rb");
  if (f == NULL)
    {
      free(data);
      return -1;
    }

  if (fread(data, 1, stat_buf.st_size, f) != stat_buf.st_size)
    {
      free(data);
      return -1;
    }

  ret = -1;
  for (a = 0;a < MAX_FILE_LOADERS;a++)
    {
      if (loaders[a])
        if (loaders[a]->check)
          {
	    /* printf("trying %s...\n", loaders[a]->get_name()); */
            if (loaders[a]->check(filename, data, stat_buf.st_size))
              {
	        /* printf("using %s.\n", loaders[a]->get_name()); */
                ret = loaders[a]->load(filename, data, stat_buf.st_size, prop);
		(*prop)->filetype = loaders[a]->get_type();
                break;
              }
          }
    }

  free(data);
  return ret;
}

void
fileio_free_prop(fileio_prop_t **prop)
{
  fileio_prop_t *ptr;
  
  for (ptr = *prop;ptr != NULL;ptr = ptr->next)
    {
      free(ptr->data);
    }
}

long
fileio_get_image_z1013(fileio_prop_t *prop, unsigned char *buf)
{
  int a, b, len;

  len = prop->size - 32;

  a = 36;
  b = 0;
  while (len-- > 0)
    {
      switch (a % 36)
	{
	case 0:
	case 1:
	case 34:
	case 35:
	  break;
	default:
	  buf[b] = prop->data[a];
	  b++;
	  break;
	}
      a++;
    }

  return b;
}

long
fileio_get_image(fileio_prop_t *prop, unsigned char *buf)
{
  int a, b, len;

  if (fileio_get_kctype() == FILEIO_Z1013)
    return fileio_get_image_z1013(prop, buf);

  b = 0;
  switch (prop->type)
    {
    case FILEIO_TYPE_COM:
    case FILEIO_TYPE_UNKNOWN:
      a = 130;
      len = prop->size - 128;
      break;
    case FILEIO_TYPE_BAS:
    case FILEIO_TYPE_PROT_BAS:
    case FILEIO_TYPE_DATA:
    case FILEIO_TYPE_LIST:
      a = 14;
      len = prop->size - 13;
      break;
    case FILEIO_TYPE_BASICODE:
      a = 0;
      len = prop->size;
      break;
    }

  while (len-- > 0)
    {
      buf[b] = prop->data[a];
      b++;
      a++;
      if ((a % 129) == 0)
	a++;
    }

  return b;
}

void
fill_header_COM(unsigned char *data,
                fileio_prop_t *prop)
{
  int a, c, end;

  memset(data, 0, 128);
  memcpy(data, "        COM", 11);
  for (a = 0;a < 8;a++)
    {
      c = toupper(prop->name[a]);
      if (c == 0)
        break;
      data[a] = c;
    }

  if ((prop->valid & FILEIO_V_LOAD_ADDR) == FILEIO_V_LOAD_ADDR)
    {
      end = prop->load_addr + prop->size + 1;
      data[16] = 2;
      data[17] = prop->load_addr & 0xff;
      data[18] = (prop->load_addr >> 8) & 0xff;
      data[19] = end & 0xff;
      data[20] = (end >> 8) & 0xff;
    }
  if ((prop->valid & FILEIO_V_START_ADDR) == FILEIO_V_START_ADDR)
    {
      data[16] = 3;
      data[21] = prop->start_addr & 0xff;
      data[22] = (prop->start_addr >> 8) & 0xff;
    }
}

void
fileio_copy_blocks(unsigned char *dptr, const unsigned char *sptr, long size, int block)
{
  long len;

  while (size > 0)
    {
      if (size > 128)
	*dptr = block++;
      else
	*dptr = 0xff;

      len = (size > 128) ? 128 : size;
      memcpy(dptr + 1, sptr, len);
      dptr += 129;
      sptr += 128;
      size -= 128;
    }
}

void
fileio_debug_dump(fileio_prop_t *prop, int write_file)
{
  FILE *f;
  char buf[100];
  const char *type;
  fileio_prop_t *ptr;
  
  printf("fileio: dumping file properties...\n");
  for (ptr = prop;ptr != NULL;ptr = ptr->next)
    {
      switch (ptr->type)
        {
        case FILEIO_TYPE_UNKNOWN:  type = "(UNKNOWN)"; break;
        case FILEIO_TYPE_COM:      type = "(COM)"; break;
        case FILEIO_TYPE_BAS:      type = "(BASIC)"; break;
        case FILEIO_TYPE_PROT_BAS: type = "(BASIC*)"; break;
        default:                   type = "(?)"; break;
        }
      
      printf("fileio: %-14s %-10s - ", ptr->name, type);
      printf("%04x-%04lx/",  ptr->load_addr, ptr->load_addr + ptr->size + 1);
      if (ptr->autostart)
        printf("%04x", ptr->start_addr);
      else
        printf(" -  ");
      printf(" : %5ld bytes [%s]", ptr->size, ptr->filetype);

      if (write_file)
        {
          sprintf(buf, ".out-%s", ptr->name);
          f = fopen(buf, "wb");
          if (f)
            {
              while (ptr->size > 0)
                {
                  ptr->data++;
                  ptr->size--;
                  if (ptr->size == 0)
                    break;
                  fwrite(ptr->data, 1, 128, f);
                  ptr->data += 128;
                  ptr->size -= 128;
                }
              fclose(f);
              printf(" -> '%s'", buf);
            }
        }
      printf("\n");
    }
  printf("fileio: done.\n");
}

void
fileio_show_config(void)
{
  int a;

  printf("available fileio plugins:\n");
  for (a = 0;a < MAX_FILE_LOADERS;a++)
    {
      if (loaders[a])
	printf("  %s\n", loaders[a]->get_name());
    }
}
