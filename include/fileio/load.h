/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: load.h,v 1.4 2001/04/22 22:24:03 tp Exp $
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

#ifndef __fileio_load_h
#define __fileio_load_h

#ifdef __cplusplus
extern "C" {
#endif


typedef enum
{
  FILEIO_V_NONE       = 0x00,
  FILEIO_V_LOAD_ADDR  = 0x01,
  FILEIO_V_START_ADDR = 0x02,
  FILEIO_V_AUTOSTART  = 0x04,
} fileio_valid_t;

typedef enum
{
  FILEIO_TYPE_UNKNOWN,
  FILEIO_TYPE_COM,
  FILEIO_TYPE_BAS,
  FILEIO_TYPE_PROT_BAS,
  FILEIO_TYPE_MINTEX,
} fileio_type_t;

typedef struct fileio_prop
{
  fileio_type_t  type;
  fileio_valid_t valid;
  unsigned char  name[12];
  unsigned short load_addr;
  unsigned short start_addr;
  int            autostart;
  long           size;
  unsigned char  *data;
  struct fileio_prop *next;
} fileio_prop_t;

/*
 *  initialize loader modules
 */
void            fileio_init(void);

/*
 *  load file(s) into memory
 */
int             fileio_load_file(const char *filename, fileio_prop_t **prop);

/*
 *  free loaded files and the associated property data
 */
void            fileio_free_prop(fileio_prop_t **prop);

/*
 *  convert internal file format into a raw memory image, returns the
 *  length of the stored image
 *
 *  the allocated buffer should have prop->size bytes which
 *  is guarantied to be slightly more than needed
 */
long            fileio_get_image(fileio_prop_t *prop, unsigned char *buf);

/*
 *  only for DEBUG
 */
void            fileio_debug_dump(fileio_prop_t *prop, int write_file);


#ifdef __cplusplus
}
#endif

#endif /* __fileio_load_h */
