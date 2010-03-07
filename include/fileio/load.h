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

#ifndef __fileio_load_h
#define __fileio_load_h

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
  FILEIO_KC85_1,
  FILEIO_KC85_3, /* default */
  FILEIO_Z1013,
} fileio_kctype_t;

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
  FILEIO_TYPE_BASICODE,
  FILEIO_TYPE_DATA,     /* produced with CSAVE*"FILENAME";ARRAYNAME */
  FILEIO_TYPE_LIST,     /* produced with LIST#1"FILENAME" */
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
  const char     *filetype;
  unsigned char  *data;
  struct fileio_prop *next;
} fileio_prop_t;

/*
 *  initialize loader modules
 */
void            fileio_init(void);

/*
 *  set the kc-type for the file io handling
 */
void            fileio_set_kctype(fileio_kctype_t type);

/*
 *  return the kc-type for the file io handling
 */
fileio_kctype_t fileio_get_kctype(void);

/*
 *  load file(s) into memory
 */
int             fileio_load_file(const char *filename, fileio_prop_t **prop);

/*
 *  save file in wav format
 */
int             fileio_save_wav_prop(const char *filename, fileio_prop_t *prop);
int             fileio_save_wav(const char *filename, const unsigned char *data, int size);

/*
 *  save file in tap format
 */
int             fileio_save_tap_prop(const char *filename, fileio_prop_t *prop);
int             fileio_save_tap(const char *filename, const unsigned char *data, int size);

/*
 *  save file in image format
 */
int             fileio_save_img_prop(const char *filename, fileio_prop_t *prop);
int             fileio_save_img(const char *filename, const unsigned char *data, int size);

/*
 *  save file in raw format (memory dump)
 */
int             fileio_save_raw_prop(const char *filename, fileio_prop_t *prop);
int             fileio_save_raw(const char *filename, const unsigned char *data, int size, int load, int start);

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
 *  dump configuration to stdout
 */    
void            fileio_show_config(void);

/*
 *  only for DEBUG
 */
void            fileio_debug_dump(fileio_prop_t *prop, int write_file);


#ifdef __cplusplus
}
#endif

#endif /* __fileio_load_h */
