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
#ifdef HAVE_LIBXVIDCORE

#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "kc/system.h"

#include "ui/gtk/xvid.h"

using namespace std;

XvidVideoEncoder::XvidVideoEncoder(void)
{
  _f = NULL;
  _buf = NULL;
  _image = NULL;
}

XvidVideoEncoder::~XvidVideoEncoder(void)
{
  close();
}

bool
XvidVideoEncoder::init(const char *filename, int width, int height, int fps_den, double quality)
{
  if (filename == NULL)
    return false;
  
  if (quality < 0)
    quality = 0;
  if (quality > 1)
    quality = 1;

  _quality = quality;

  _f = fopen(filename, "wb+");
  if (_f == NULL)
    return false;

  memset(&_gbl_init, 0, sizeof (xvid_gbl_init_t));
  _gbl_init.version = XVID_VERSION;
  xvid_global(NULL, XVID_GBL_INIT, &_gbl_init, NULL);

  memset(&_enc_create, 0, sizeof (xvid_enc_create_t));
  _enc_create.version = XVID_VERSION;
  _enc_create.width = width;
  _enc_create.height = height;
  _enc_create.zones = NULL;
  _enc_create.fincr = fps_den;
  _enc_create.fbase = 50;
  _enc_create.max_key_interval = 500 / fps_den;
  _enc_create.bquant_ratio = 150;
  _enc_create.bquant_offset = 100;

  memset(&_plugin_single, 0, sizeof(xvid_plugin_single_t));
  _plugin_single.version = XVID_VERSION;

  _plugins[0].func  = xvid_plugin_single;
  _plugins[0].param = &_plugin_single;

   _enc_create.plugins = _plugins;
   _enc_create.num_plugins = 1;

   xvid_encore(NULL, XVID_ENC_CREATE, &_enc_create, NULL);

   _buf = new byte_t[3 * width * height];
   _image = new byte_t[3 * width * height];
   return true;
}

void
XvidVideoEncoder::allocate_color_rgb(int idx, int r, int g, int b)
{
  _col[idx].r = r;
  _col[idx].g = g;
  _col[idx].b = b;
}

bool
XvidVideoEncoder::encode(byte_t *image, byte_t *dirty)
{
  int d = -1;
  for (int y = 0;y < _enc_create.height;y += 8)
    {
      for (int x = 0;x < _enc_create.width;x += 8)
        {
          d++;
          if (dirty && !dirty[d])
            continue;

          int z = y * _enc_create.width + x;

          for (int yy = 0;yy < 8;yy++)
            {
              for (int xx = 0;xx < 8;xx++)
                {
                  int s = 3 * (z + xx);
                  _image[s] = _col[image[z + xx]].b;
                  _image[s + 1] = _col[image[z + xx]].g;
                  _image[s + 2] = _col[image[z + xx]].r;
                }
              z += _enc_create.width;
            }
        }
    }

  xvid_enc_frame_t frame;
  memset(&frame, 0, sizeof(xvid_enc_frame_t));

  frame.version = XVID_VERSION;
  frame.bitstream = _buf;
  frame.length = -1;
  frame.input.plane[0] = _image;
  frame.input.csp = XVID_CSP_BGR;
  frame.input.stride[0] = 3 * _enc_create.width;
  frame.vol_flags = 0;
  frame.vop_flags = XVID_VOP_HALFPEL | XVID_VOP_TRELLISQUANT | XVID_VOP_HQACPRED;
  frame.type = XVID_TYPE_AUTO;
  frame.quant = _quality;
  frame.motion = XVID_ME_ADVANCEDDIAMOND16 | XVID_ME_HALFPELREFINE16 | XVID_ME_EXTSEARCH16 | XVID_ME_ADVANCEDDIAMOND8 | XVID_ME_HALFPELREFINE8 | XVID_ME_EXTSEARCH8;

  int size = xvid_encore(_enc_create.handle, XVID_ENC_ENCODE, &frame, NULL);
  return fwrite(_buf, 1, size, _f) == (size_t)size;
}

void
XvidVideoEncoder::close(void)
{
  if (_f != NULL)
    {
      xvid_encore(_enc_create.handle, XVID_ENC_DESTROY, NULL, NULL);
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

#endif /* HAVE_LIBXVIDCORE */
