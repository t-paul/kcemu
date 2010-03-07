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

#include "kc/config.h"
#ifdef HAVE_LIBSCHROEDINGER

#include <string.h>

#include "kc/system.h"

#include "ui/gtk/schro.h"

void
SchroedingerVideoEncoder::image_free(SchroFrame *frame, void *priv)
{
  delete[] (byte_t *)priv;
}

SchroedingerVideoEncoder::SchroedingerVideoEncoder(void)
{
  _f = NULL;
  _buf = NULL;
  _image = NULL;
}

SchroedingerVideoEncoder::~SchroedingerVideoEncoder(void)
{
  close();
}

bool
SchroedingerVideoEncoder::init(const char *filename, int width, int height, int fps_den, double quality)
{
  if (filename == NULL)
    return false;

  if (quality < 0)
    quality = 0;
  if (quality > 1)
    quality = 1;

  _f = fopen(filename, "wb+");
  if (_f == NULL)
    return false;

  _width = width;
  _height = height;

  schro_init();

  _encoder = schro_encoder_new();
  SchroVideoFormat *format = schro_encoder_get_video_format(_encoder);
  format->width = width;
  format->height = height;
  format->clean_width = width;
  format->clean_height = height;
  format->top_offset = 0;
  format->left_offset = 0;
  format->aspect_ratio_numerator = 1;
  format->aspect_ratio_denominator = 1;
  format->frame_rate_numerator = 50;
  format->frame_rate_denominator = fps_den;
  format->chroma_format = SCHRO_CHROMA_444;
  format->interlaced = FALSE;
  schro_encoder_set_video_format(_encoder, format);
  free(format);

  schro_encoder_setting_set_double(_encoder, "quality", 10 * quality);
  schro_encoder_start(_encoder);

  return true;
}

void
SchroedingerVideoEncoder::allocate_color_rgb(int idx, int r, int g, int b)
{
  _col[idx].y = (0.257 * r) + (0.504 * g) + (0.098 * b) + 16;
  _col[idx].u = -(0.148 * r) - (0.291 * g) + (0.439 * b) + 128;
  _col[idx].v = (0.439 * r) - (0.368 * g) - (0.071 * b) + 128;
}

bool
SchroedingerVideoEncoder::encode_loop()
{
  while (242)
    {
      switch (schro_encoder_wait(_encoder))
        {
        case SCHRO_STATE_AGAIN:
          break;
        case SCHRO_STATE_NEED_FRAME:
        case SCHRO_STATE_END_OF_STREAM:
          return true;
        case SCHRO_STATE_HAVE_BUFFER:
          {
            SchroBuffer *buffer = schro_encoder_pull(_encoder, NULL);
            if (fwrite(buffer->data, buffer->length, 1, _f) != 1)
              return false;

            schro_buffer_unref(buffer);
            return true;
          }
        default:
          return false;
        }
    }
}

bool
SchroedingerVideoEncoder::encode(byte_t *image, byte_t *dirty)
{
  int count = _width * _height;
  byte_t *buf = new byte_t[4 * count];

  for (int src = 0, dst = 0;src < count;src++, dst += 4)
    {
      color_t *col = &_col[image[src]];
      buf[dst] = 0;
      buf[dst + 1] = col->y;
      buf[dst + 2] = col->u;
      buf[dst + 3] = col->v;
    }

  if (!encode_loop())
    return false;

  SchroFrame *frame = schro_frame_new_from_data_AYUV(buf, _width, _height);
  schro_frame_set_free_callback(frame, image_free, buf);
  schro_encoder_push_frame(_encoder, frame);

  return encode_loop();
}

void
SchroedingerVideoEncoder::close(void)
{
  if (_f != NULL)
    {
      schro_encoder_end_of_stream(_encoder);
      encode_loop();
      schro_encoder_free(_encoder);
      fclose(_f);
    }

  if (_buf != NULL)
    delete _buf;

  if (_image != NULL)
    delete _image;

  _f = NULL;
  _buf = NULL;
  _image = NULL;
}

#endif /* HAVE_LIBSCHROEDINGER */
