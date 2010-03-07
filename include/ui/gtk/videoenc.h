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

#ifndef __ui_gtk_videoenc_h
#define __ui_gtk_videoenc_h

#include "kc/system.h"

class VideoEncoder
{
public:
  enum
  {
    VIDEO_ENCODER_STATE_STOP = 0,
    VIDEO_ENCODER_STATE_RECORD = 1,
    VIDEO_ENCODER_STATE_PAUSE = 2,
  };

public:
  virtual bool init(const char *filename, int width, int height, int fps_den, double quality) = 0;
  virtual void allocate_color_rgb(int idx, int r, int g, int b) = 0;
  virtual bool encode(byte_t *image, byte_t *dirty) = 0;
  virtual void close(void) = 0;
};

class DummyVideoEncoder : public VideoEncoder
{
  virtual bool init(const char *filename, int width, int height, int fps_den, double quality) { return false; }
  virtual void allocate_color_rgb(int idx, int r, int g, int b) {}
  virtual void allocate_color_hsv(int idx, double h, double s, double v) {}
  virtual bool encode(byte_t *image, byte_t *dirty) { return true; }
  virtual void close(void) {}
};

#endif /* __ui_gtk_videoenc_h */
