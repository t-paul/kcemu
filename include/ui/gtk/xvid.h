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

#ifndef __ui_gtk_xvid_h
#define __ui_gtk_xvid_h

#include "kc/config.h"
#ifdef HAVE_LIBXVIDCORE

#include <stdio.h>

#include <xvid.h>

#include "kc/system.h"

#include "ui/gtk/videoenc.h"

class XvidVideoEncoder : public VideoEncoder
{
private:
  typedef struct { int r, g, b; } color_t;
  
private:
  FILE     *_f;
  color_t   _col[256];
  byte_t   *_buf;
  byte_t   *_image;
  double    _quality;

  xvid_gbl_init_t      _gbl_init;
  xvid_enc_create_t    _enc_create;
  xvid_plugin_single_t _plugin_single;
  xvid_enc_plugin_t    _plugins[1];

  
public:
  XvidVideoEncoder(void);
  virtual ~XvidVideoEncoder(void);
  
  virtual bool init(const char *filename, int width, int height, int fps_den, double quality);
  virtual void allocate_color_rgb(int idx, int r, int g, int b);
  virtual bool encode(byte_t *image, byte_t *dirty);
  virtual void close(void);
};

#endif /* HAVE_LIBXVIDCORE */

#endif /* __ui_gtk_xvid_h */
