/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
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

#include <ctype.h>
#include <stdio.h>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/memory.h"

#include "ui/font0.h"

#include "ui/generic/ui_0.h"

UI_0::UI_0(void)
{
  int a;

  _dirty_size = (get_real_width() * get_real_height()) / 64;
  _dirty = new byte_t[_dirty_size];

  _bitmap = new byte_t[get_real_width() * get_real_height()];
  _pix_cache = new byte_t[_dirty_size];
  _col_cache = new byte_t[_dirty_size];

  for (a = 0;a < _dirty_size;a++)
    _dirty[a] = 1;
}

UI_0::~UI_0(void)
{
  delete[] _dirty;
  delete[] _bitmap;
  delete[] _pix_cache;
  delete[] _col_cache;
}

int
UI_0::get_real_width(void)
{
  return 256;
}

int
UI_0::get_real_height(void)
{
  return 256;
}

byte_t *
UI_0::get_dirty_buffer(void)
{
  return _dirty;
}

int
UI_0::get_dirty_buffer_size(void)
{
  return _dirty_size;
}

void
UI_0::generic_put_pixels(byte_t *ptr, byte_t val, word_t color)
{
  for (int a = 0;a < 8;a++)
    ptr[a] = (val & (128 >> a)) ? (color >> 8) : color;
}

void
UI_0::generic_update(bool clear_cache)
{
  byte_t pix;
  int a, x, y, z, width, height;

  byte_t *irm = memory->getIRM();
  byte_t *ptr = _bitmap;

  width = get_real_width();
  height = get_real_height();

  z = -1;
  for (y = 0;y < 256;y += 8)
    {
      for (x = 0;x < 256;x += 8)
        {
	  z++;
	  pix = irm[z];
	  if (_pix_cache[z] != pix)
	    _dirty[z]++;

	  if (clear_cache)
	    _dirty[z]++;

	  if (!_dirty[z])
	    continue;

	  _pix_cache[z] = pix;
          for (a = 0;a < 8;a++)
            generic_put_pixels(ptr + a * width + x, __font[8 * pix + a], 0x0100);
	}
      ptr += 8 * width;
    }
}
