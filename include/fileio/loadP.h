/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-1998 Torsten Paul
 *
 *  $Id: loadP.h,v 1.3 2000/07/15 13:16:41 tp Exp $
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
  const char * (*get_name)(void);
} file_loader_t;

#define MAX_FILE_LOADERS (10)

void fill_header_COM(unsigned char *data,
                     fileio_prop_t *prop);

void loader_AF_init(void);
void loader_GPF_init(void);
void loader_SSS_init(void);
void loader_BASIC_init(void);
void loader_TAPE_init(void);
void loader_UNKNOWN_init(void);

#endif /* __fileio_loadp_h */
