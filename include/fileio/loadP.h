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

#ifndef __fileio_loadp_h
#define __fileio_loadp_h

#include "fileio/load.h"

typedef struct file_loader
{
  int (*check)(const char *filename,
               unsigned char *data,
               long size);
  int (*load)(const char *filename,
              unsigned char *data,
              long size,
              fileio_prop_t **prop);
  const char * (*get_type)(void);
  const char * (*get_name)(void);
} file_loader_t;

#define MAX_FILE_LOADERS (10)

void fill_header_COM(unsigned char *data,
                     fileio_prop_t *prop);
void fill_header_HS(unsigned char *data,
                    fileio_prop_t *prop);
void fileio_copy_blocks(unsigned char *dptr,
			const unsigned char *sptr,
			long size,
			int block);
int  fileio_register_loader(file_loader_t *loader);

void loader_AF_init(void);
void loader_GPF_init(void);
void loader_SSS_init(void);
void loader_BASIC_init(void);
void loader_BASICODE_init(void);
void loader_HS_init(void);
void loader_BIN_init(void);
void loader_TAPE_init(void);
void loader_UNKNOWN_init(void);

#endif /* __fileio_loadp_h */
