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
#ifdef HAVE_LIBVNCSERVER

#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "kc/system.h"

#include "ui/gtk/vnc.h"

using namespace std;

VncVideoEncoder::VncVideoEncoder(void)
{
  _width = -1;
  _height = -1;
  _rfbScreen = NULL;
  rfbLogEnable(FALSE);
}

VncVideoEncoder::~VncVideoEncoder(void)
{
  close();
}

bool
VncVideoEncoder::init(const char *filename, int width, int height, int fps_den, double quality)
{
  if ((_width == width) && (_height == height) && (_rfbScreen != NULL))
    {
      rfbMarkRectAsModified(_rfbScreen, 0, 0, _width, _height);
      return true;
    }

  _width = width;
  _height = height;

  char *framebuffer = (char *)new byte_t[width * height * 4];

  if (_rfbScreen == NULL)
    {
      int argc[] = { 1 };
      char *argv[] = { "KCemu", NULL };
      _rfbScreen = rfbGetScreen(argc, argv, width, height, 8, 3, 4);
      _rfbScreen->desktopName = "KCemu " KCEMU_VERSION;
      _rfbScreen->alwaysShared = TRUE;
      _rfbScreen->serverFormat.trueColour = TRUE;
      _rfbScreen->frameBuffer = framebuffer;
      rfbInitServer(_rfbScreen);
    }
  else
    {
      char *oldfb = _rfbScreen->frameBuffer;
      rfbNewFramebuffer(_rfbScreen, framebuffer, width, height, 8, 3, 4);
      delete[] oldfb;
      rfbInitServer(_rfbScreen);
    }

  return true;
}

void
VncVideoEncoder::allocate_color_rgb(int idx, int r, int g, int b)
{
  _col[idx].r = r;
  _col[idx].g = g;
  _col[idx].b = b;
}

bool
VncVideoEncoder::encode(byte_t *image, byte_t *dirty)
{
  int x_min = _width;
  int x_max = -1;
  int y_min = _height;
  int y_max = -1;

  int d = -1;
  for (int y = 0;y < _height;y += 8)
    {
      for (int x = 0;x < _width;x += 8)
        {
          d++;
          if (dirty && !dirty[d])
            continue;

          if (x < x_min)
            x_min = x;
          if (x > x_max)
            x_max = x;
          if (y < y_min)
            y_min = y;
          if (y > y_max)
            y_max = y;

          int z = y * _width + x;

          for (int yy = 0;yy < 8;yy++)
            {
              for (int xx = 0;xx < 8;xx++)
                {
                  int s = 4 * (z + xx);
                  _rfbScreen->frameBuffer[s] = _col[image[z + xx]].r;
                  _rfbScreen->frameBuffer[s + 1] = _col[image[z + xx]].g;
                  _rfbScreen->frameBuffer[s + 2] = _col[image[z + xx]].b;
                }
              z += _width;
            }
        }
    }

  if (x_max >= 0)
    {
      rfbMarkRectAsModified(_rfbScreen, x_min, y_min, x_max + 8, y_max + 8);
    }
  
  while (rfbProcessEvents(_rfbScreen, 0));
  
  return true;
}

void
VncVideoEncoder::close(void)
{
  if (_rfbScreen != NULL)
    {
      rfbShutdownServer(_rfbScreen, TRUE);
      _rfbScreen = NULL;
    }
}

#endif /* HAVE_LIBVNCSERVER */
