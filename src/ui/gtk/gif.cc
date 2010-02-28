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

#include <string>

#include <string.h>

#include "kc/config.h"
#ifdef HAVE_LIBGIF

#include "kc/system.h"

#include "ui/gtk/gif.h"

using namespace std;

GifVideoEncoder::GifVideoEncoder(void)
{
  _width = -1;
  _height = -1;
  _cmap = NULL;
  _gif = NULL;
  _buf = NULL;
}

GifVideoEncoder::~GifVideoEncoder(void)
{
  close();
}

bool
GifVideoEncoder::init(const char *filename, int width, int height, int fps_den, double quality)
{
  /*
   * Netscape Application Extension
   * (see: http://www.let.rug.nl/~kleiweg/gif/netscape.html)
   *
   * byte   1       : 33 (hex 0x21) GIF Extension code
   * byte   2       : 255 (hex 0xFF) Application Extension Label
   * byte   3       : 11 (hex 0x0B) Length of Application Block
   *                  (eleven bytes of data to follow)
   * bytes  4 to 11 : "NETSCAPE"
   * bytes 12 to 14 : "2.0"
   * byte  15       : 3 (hex 0x03) Length of Data Sub-Block
   *                  (three bytes of data to follow)
   * byte  16       : 1 (hex 0x01)
   * bytes 17 to 18 : 0 to 65535, an unsigned integer in
   *                  lo-hi byte format. This indicate the
   *                  number of iterations the loop should
   *                  be executed.
   * byte  19       : 0 (hex 0x00) a Data Sub-Block Terminator.
   */
  static unsigned char EXT_NETSCAPE[] = {'N','E','T','S', 'C','A','P','E', '2','.','0' };
  static unsigned char EXT_NETSCAPE_LOOP[] = { 1, 0, 0 }; // loop counter = 0
  
  if ((_width == width) && (_height == height) && (_gif != NULL))
    {
      return false;
    }

  _width = width;
  _height = height;
  _fps_den = fps_den;
  _frame_delay = 0;

  EGifSetGifVersion(&GIF89_STAMP[GIF_VERSION_POS]);
  _gif = EGifOpenFileName(filename, 0);
  if (_gif == NULL)
    return false;

  _cmap = MakeMapObject(256, NULL);
  for (int a = 0;a < 256;a++)
    {
      _cmap->Colors[a].Red = 0;
      _cmap->Colors[a].Green = 0;
      _cmap->Colors[a].Blue = 0;
    }

  if (EGifPutScreenDesc(_gif, _width, _height, 256, 0, _cmap) != GIF_OK)
    return false;

  string comment = "Created by KCemu " KCEMU_VERSION;
  if (EGifPutComment(_gif, comment.c_str()) != GIF_OK)
    return false;

  if (EGifPutExtensionFirst(_gif, 0xFF, 11, EXT_NETSCAPE) != GIF_OK)
    return false;

  if (EGifPutExtensionLast(_gif, 0, 3, EXT_NETSCAPE_LOOP) != GIF_OK)
    return false;

  return true;
}

void
GifVideoEncoder::allocate_color_rgb(int idx, int r, int g, int b)
{
  if (_cmap == NULL)
    return;

  _cmap->Colors[idx].Red = r;
  _cmap->Colors[idx].Green = g;
  _cmap->Colors[idx].Blue = b;
}

bool
GifVideoEncoder::encode(byte_t *image, byte_t *dirty)
{
  _frame_delay += 2 * _fps_den;
  if (_frame_delay > MAX_FRAME_DELAY)
    _frame_delay = MAX_FRAME_DELAY;

  if (dirty)
    {
      if (_buf == NULL)
        {
          _buf = new byte_t[_width * _height];
          memcpy(_buf, image, _width * _height);
          return true;
        }

      bool changed = false;
      for (int a = 0;a < (_width * _height) / 64;a++)
        {
          if (dirty[a])
            {
              changed = true;
              break;
            }
        }

      if (!changed)
        return true;
    }

  bool ret;
  if (_buf)
    {
      ret = flush_buffer(_buf, _frame_delay);
      memcpy(_buf, image, _width * _height);
    }
  else
    {
      ret = flush_buffer(image, _frame_delay);
    }

  _frame_delay = 0;

  return ret;
}

bool
GifVideoEncoder::flush_buffer(byte_t *buf, int delay)
{
  /*
   * Graphic Control Extension
   * (see: http://local.wasp.uwa.edu.au/~pbourke/dataformats/gif/)
   *
   * byte 1: | 3 bit Reserved | 3 bit Disposal Method | User Input | Transparent |
   *         Disposal Method:
   *         0 -   No disposal specified. The decoder is
   *               not required to take any action.
   *         1 -   Do not dispose. The graphic is to be left
   *               in place.
   *         2 -   Restore to background color. The area used by the
   *               graphic must be restored to the background color.
   *         3 -   Restore to previous. The decoder is required to
   *               restore the area overwritten by the graphic with
   *               what was there prior to rendering the graphic.
   *         4-7 -    To be defined.
   *         User Input:
   *         0 -   User input is not expected.
   *         1 -   User input is expected.
   *         Transparent:
   *         0 -   Transparent Index is not given.
   *         1 -   Transparent Index is given.
   * byte 2 & 3:
   *         Delay Time - If not 0, this field specifies the number of
   *         hundredths (1/100) of a second to wait before continuing with the
   *         processing of the Data Stream. The clock starts ticking immediately
   *         after the graphic is rendered. This field may be used in
   *         conjunction with the User Input Flag field.
   * byte 4:
   *         Transparency Index - The Transparency Index is such that when
   *         encountered, the corresponding pixel of the display device is not
   *         modified and processing goes on to the next pixel. The index is
   *         present if and only if the Transparency Flag is set to 1.
   */
  unsigned char EXT_GCE[] = {0, delay, delay >> 8, 0};

  if (EGifPutExtension(_gif, 0xF9, 4, EXT_GCE) != GIF_OK)
    return false;

  if (EGifPutImageDesc(_gif, 0, 0, _width, _height, 0, _cmap) != GIF_OK)
    return false;

  if (EGifPutLine(_gif, buf, _width * _height) != GIF_OK)
    return false;
}

void
GifVideoEncoder::close(void)
{
  if (_gif != NULL)
    {
      if (_buf)
        flush_buffer(_buf, _frame_delay);
      EGifCloseFile(_gif);
      _gif = NULL;
    }
  if (_cmap != NULL)
    {
      FreeMapObject(_cmap);
      _cmap = NULL;
    }
  if (_buf != NULL)
    {
      delete[] _buf;
      _buf = NULL;
    }
}

#endif /* HAVE_LIBGIF */
