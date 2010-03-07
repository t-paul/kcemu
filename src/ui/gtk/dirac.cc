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
#ifdef HAVE_LIBDIRAC

#include "kc/system.h"

#include "ui/gtk/dirac.h"

DiracVideoEncoder::DiracVideoEncoder(void)
{
  _f = NULL;
  _buf = NULL;
  _image = NULL;
}

DiracVideoEncoder::~DiracVideoEncoder(void)
{
  close();
}

bool
DiracVideoEncoder::init(const char *filename, int width, int height, int fps_den, double quality)
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

  dirac_encoder_context_init(&_context, VIDEO_FORMAT_CUSTOM);

  dirac_frame_rate_t frame_rate;
  frame_rate.numerator = 50;
  frame_rate.denominator = fps_den;

  dirac_pix_asr_t pix_asr;
  pix_asr.numerator = 1;
  pix_asr.denominator = 1;

  dirac_signal_range_t signal_range;
  signal_range.chroma_offset = 128;
  signal_range.chroma_excursion = 255;
  signal_range.luma_offset = 0;
  signal_range.luma_excursion = 255;

  _context.src_params.width = width;
  _context.src_params.height = height;
  _context.src_params.chroma_width = width;
  _context.src_params.chroma_height = height;
  _context.src_params.chroma = format444;
  _context.src_params.pix_asr = pix_asr;
  _context.src_params.signal_range = signal_range;
  _context.src_params.source_sampling = 0;
  _context.src_params.topfieldfirst = 0;
  _context.src_params.frame_rate = frame_rate;

  if (quality == 1.0)
    _context.enc_params.lossless = 1;

  _context.enc_params.qf = 10.0 * quality;
  _context.enc_params.full_search = 0;
  _context.enc_params.combined_me = 0;
  _context.enc_params.num_L1 = fps_den < 5 ? 0 : 2 * fps_den;
  _context.instr_flag = 0;
  _context.decode_flag = 0;

  _encoder = dirac_encoder_init(&_context, 0);
  if (_encoder == NULL)
    {
      close();
      return false;
    }

  _image_size = 3 * width * height;

  _buf = new byte_t[_image_size];
  _image = new byte_t[_image_size];

  return true;
}

void
DiracVideoEncoder::allocate_color_rgb(int idx, int r, int g, int b)
{
  _col[idx].y = (0.257 * r) + (0.504 * g) + (0.098 * b) + 16;
  _col[idx].u = -(0.148 * r) - (0.291 * g) + (0.439 * b) + 128;
  _col[idx].v = (0.439 * r) - (0.368 * g) - (0.071 * b) + 128;
}

bool
DiracVideoEncoder::encode_loop()
{
  dirac_encoder_state_t state;
  
  do
    {
      _encoder->enc_buf.buffer = _buf;
      _encoder->enc_buf.size = _image_size;

      state = dirac_encoder_output(_encoder);

      switch (state)
        {
        case ENC_STATE_BUFFER:
          break;
        case ENC_STATE_EOS:
        case ENC_STATE_AVAIL:
          fwrite(_encoder->enc_buf.buffer, _encoder->enc_buf.size, 1, _f);
          break;
        default:
        case ENC_STATE_INVALID:
          close();
          return false;
        }
    }
  while (state == ENC_STATE_AVAIL);
  
  return true;
}

bool
DiracVideoEncoder::encode(byte_t *image, byte_t *dirty)
{
  const int count = _context.src_params.width * _context.src_params.height;

  for (int idx = 0;idx < count;idx++)
    {
      color_t *col = &_col[image[idx]];
      _image[idx] = col->y;
      _image[idx + count] = col->u;
      _image[idx + 2 * count] = col->v;
    }

  if (dirac_encoder_load(_encoder, _image, _image_size) < 0)
    return false;

  return encode_loop();
}

void
DiracVideoEncoder::close(void)
{
  if (_f != NULL)
    {
      dirac_encoder_end_sequence(_encoder);
      encode_loop();
      dirac_encoder_close(_encoder);
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

#endif /* HAVE_LIBDIRAC */
