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

#ifndef __ui_gtk_theora_h
#define __ui_gtk_theora_h

#include "kc/config.h"
#ifdef HAVE_LIBTHEORA

#include <stdio.h>

#include <theora/codec.h>
#include <theora/theoraenc.h>

#include "kc/system.h"

#include "ui/gtk/videoenc.h"

class TheoraVideoEncoder : public VideoEncoder
{
private:
  typedef struct { int y, u, v; } color_t;
  
private:
  FILE             *_f;
  
  th_info           _info;
  th_pixel_fmt      _format;
  th_enc_ctx       *_context;
  
  ogg_stream_state  _stream;

  byte_t           *_buf_y;
  byte_t           *_buf_u;
  byte_t           *_buf_v;

  color_t           _col[256];
  
protected:
  virtual bool flush(void);
  virtual bool write_page(ogg_page *page);
  virtual void encode_444(byte_t *image, byte_t *dirty);
  virtual void encode_420(byte_t *image, byte_t *dirty);
  
public:
  TheoraVideoEncoder(void);
  virtual ~TheoraVideoEncoder(void);
  
  virtual bool init(const char *filename, int width, int height, int fps_den, double quality);
  virtual void allocate_color_rgb(int idx, int r, int g, int b);
  virtual bool encode(byte_t *image, byte_t *dirty);
  virtual void close(void);
};

#endif /* HAVE_LIBTHEORA */

#endif /* __ui_gtk_theora_h */
