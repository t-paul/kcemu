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

#include "kc/config.h"
#ifdef HAVE_LIBTHEORA

#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "kc/system.h"

#include "ui/gtk/theora.h"

using namespace std;

TheoraVideoEncoder::TheoraVideoEncoder(void)
{
  _f = NULL;
  _context = NULL;
  _buf_y = _buf_u = _buf_v = NULL;
}

TheoraVideoEncoder::~TheoraVideoEncoder(void)
{
  close();
}

bool
TheoraVideoEncoder::init(const char *filename, int width, int height, double quality)
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

  th_info_init(&_info);
  _info.pic_x = 0;
  _info.pic_y = 0;
  _info.pic_width = width;
  _info.pic_height = height;
  _info.frame_width = (width + 15) & ~0x0f;
  _info.frame_height = (height + 15) & ~0x0f;
  _info.target_bitrate = 0;
  _info.quality = 63 * quality;
  _info.fps_numerator = 50;
  _info.fps_denominator = 1;
  _info.aspect_numerator = 1;
  _info.aspect_denominator = 1;
  _info.colorspace = TH_CS_ITU_REC_470BG;
  _info.pixel_fmt = TH_PF_444;

  _buf_y = new byte_t[_info.frame_width * _info.frame_height];
  _buf_u = new byte_t[_info.frame_width * _info.frame_height];
  _buf_v = new byte_t[_info.frame_width * _info.frame_height];

  _context = th_encode_alloc(&_info);
  if (_context == NULL)
    return false;

  srand(time(NULL));
  if (ogg_stream_init(&_stream, rand()) != 0)
    return false;

  th_comment comment;
  th_comment_init(&comment);
  th_comment_add_tag(&comment, "ENCODER", "KCemu");
  th_comment_add_tag(&comment, "ENCODER_VERSION", KCEMU_VERSION);

  ogg_packet packet;
  while (th_encode_flushheader(_context, &comment, &packet) > 0)
    {
      th_comment_clear(&comment);

      ogg_stream_packetin(&_stream, &packet);

      ogg_page page;
      if (ogg_stream_pageout(&_stream, &page) != 0)
        if (!write_page(&page))
          return false;
    }

  return flush();
}

void
TheoraVideoEncoder::allocate_color_rgb(int idx, int r, int g, int b)
{
  _col[idx].y =  (0.257 * r) + (0.504 * g) + (0.098 * b) + 16;
  _col[idx].u = -(0.148 * r) - (0.291 * g) + (0.439 * b) + 128;
  _col[idx].v =  (0.439 * r) - (0.368 * g) - (0.071 * b) + 128;

  printf("rgb: %d, %d, %d -> %d, %d, %d\n", r, g, b, _col[idx].y, _col[idx].u, _col[idx].v);
}

void
TheoraVideoEncoder::allocate_color_hsv(int idx, double h, double s, double v)
{
  int r, g, b;

  hsv2rgb(h, s, v, &r, &g, &b);
  allocate_color_rgb(idx, r, g, b);
}

bool
TheoraVideoEncoder::write_page(ogg_page *page)
{
  if (fwrite(page->header, 1, page->header_len, _f) != (size_t) page->header_len)
    return false;

  if (fwrite(page->body, 1, page->body_len, _f) != (size_t) page->body_len)
    return false;

  return true;
}

bool
TheoraVideoEncoder::flush(void)
{
  ogg_page page;
  while (ogg_stream_flush(&_stream, &page) != 0)
    if (!write_page(&page))
      return false;

  fflush(_f);

  return true;
}

bool
TheoraVideoEncoder::encode(byte_t *image, byte_t *dirty)
{
  th_ycbcr_buffer buffer;
  for (int a = 0;a < 3;a++)
    {
      buffer[a].width = _info.frame_width;
      buffer[a].height = _info.frame_height;
      buffer[a].stride = buffer[a].width;
    }
  buffer[0].data = _buf_y;
  buffer[1].data = _buf_u;
  buffer[2].data = _buf_v;

  for (int y = 0;y < _info.frame_height;y++)
  {
    int z = y * _info.frame_width;
    for (int x = 0;x < _info.frame_width;x++)
    {
      _buf_y[z + x] = _col[image[z + x]].y;
      _buf_u[z + x] = _col[image[z + x]].u;
      _buf_v[z + x] = _col[image[z + x]].v;
    }
  }

  if (th_encode_ycbcr_in(_context, buffer) != 0)
    return false;

  ogg_packet packet;
  while (th_encode_packetout(_context, 0, &packet))
    {
      ogg_stream_packetin(&_stream, &packet);

      ogg_page page;
      if (ogg_stream_pageout(&_stream, &page) != 0)
        if (!write_page(&page))
          return false;
    }

  flush();
}

void
TheoraVideoEncoder::close(void)
{
  if (_context != NULL)
    {
      th_encode_free(_context);
      _context = NULL;
    }

  if (_f != NULL)
    {
      fclose(_f);
      _f = NULL;
    }

  if (_buf_y != NULL)
    {
      delete[] _buf_y;
      delete[] _buf_u;
      delete[] _buf_v;
      _buf_y = _buf_u = _buf_v = NULL;
    }
}

#endif /* HAVE_LIBTHEORA */
