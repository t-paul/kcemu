/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2009 Torsten Paul
 *
 *  $Id$
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
#ifndef __ui_gtk_schro_h
#define __ui_gtk_schro_h

#include "kc/config.h"
#ifdef HAVE_LIBSCHROEDINGER

#include <stdio.h>

#include <schroedinger/schro.h>

#include "kc/system.h"

#include "ui/gtk/videoenc.h"

class SchroedingerVideoEncoder : public VideoEncoder
{
private:
  typedef struct { int y, u, v; } color_t;
  
private:
  FILE     *_f;
  color_t   _col[256];
  byte_t   *_buf;
  int       _width;
  int       _height;
  byte_t   *_image;
  int       _image_size;

  SchroEncoder *_encoder;
  
protected:
  bool encode_loop(void);

  static void image_free(SchroFrame *frame, void *priv);

public:
  SchroedingerVideoEncoder(void);
  virtual ~SchroedingerVideoEncoder(void);
  
  virtual bool init(const char *filename, int width, int height, int fps_den, double quality);
  virtual void allocate_color_rgb(int idx, int r, int g, int b);
  virtual bool encode(byte_t *image, byte_t *dirty);
  virtual void close(void);
};

#endif /* HAVE_LIBSCHROEDINGER */

#endif /* __ui_gtk_schro_h */
