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

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/gdc.h"
#include "kc/memory.h"

#include "ui/generic/ui_0.h"

UI_0::UI_0(void)
{
  _font = NULL;
  _dirty = NULL;
  _bitmap = NULL;
  _pix_cache = NULL;
  _col_cache = NULL;

  generic_set_mode(UI_GENERIC_MODE_Z1013_32x32);
  
  init();
}

UI_0::~UI_0(void)
{
  dispose();
}

void
UI_0::init(void)
{
  _dirty_size = (get_real_width() * get_real_height()) / 64;
  _dirty = new byte_t[_dirty_size];
  _bitmap = new byte_t[get_real_width() * get_real_height()];
  _pix_cache = new byte_t[_dirty_size];
  _col_cache = new byte_t[_dirty_size];

  memset(_dirty, 1, _dirty_size);
  memset(_bitmap, 0, get_real_width() * get_real_height());
}

void
UI_0::dispose(void)
{
  if (_dirty)
    delete[] _dirty;
  if (_bitmap)
    delete[] _bitmap;
  if (_pix_cache)
    delete[] _pix_cache;
  if (_col_cache)
    delete[] _col_cache;
}

void
UI_0::generic_put_pixels(byte_t *ptr, byte_t val, word_t color)
{
  for (int a = 0;a < 8;a++)
    ptr[a] = (val & (128 >> a)) ? (color >> 8) : color;
}

void
UI_0::generic_update_gdc(byte_t *font, bool clear_cache)
{
  if (gdc == NULL)
    return;

  int a, x, y, z, p, c, fg, bg, col;
  long offset = gdc->get_pram_SAD(0);

  int width = 80;
  int height = 25;
  int lines = 10;
  int linecount = 0;
  int subscreen = 0;

  byte_t *ptr = _bitmap;

  z = -1;
  for (y = 0;y < height;y++)
    {
      if (linecount >= gdc->get_pram_LEN(subscreen))
	{
	  subscreen++;
	  linecount = 0;
	  offset = gdc->get_pram_SAD(subscreen) - z - 1; // compensate value of loop variable!
	}

      linecount += lines;

      for (x = 0;x < width;x++)
	{
	  z++;
	  p = gdc->get_mem(z + offset);
	  c = gdc->get_col(z + offset);

	  _dirty[z] = clear_cache;

	  if (p != _pix_cache[z])
	    {
	      _dirty[z]++;
	      _pix_cache[z] = p;
	    }

	  if (c != _col_cache[z])
	    {
	      _dirty[z]++;
	      _col_cache[z] = c;
	    }

	  if (gdc->get_cursor(z + offset))
	    {
	      _pix_cache[z] = 255; // invalidate cache on cursor position to force
	      _col_cache[z] = 255; // update there when cursor position changes
	    }

	  if (!_dirty[z])
	    continue;

	  fg = c & 15;
	  bg = (c >> 4) & 7;

	  col = 0x0100;
	  if (p & 0x80)
	    {
	      p &= 0x7f;
	      col = 0x0001;
	    }

	  ptr = _bitmap + y * 8 * 8 * width + 8 * x;
          for (a = 0;a < 8;a++)
	    {
	      int xor_val = 0;
	      if (gdc->get_cursor(z + offset, a))
		xor_val = 0x0101;
	      generic_put_pixels(ptr + a * 8 * width, font[8 * p + a], col ^ xor_val);
	    }
	}
    }
}

void
UI_0::generic_update_32x32(byte_t *font, bool clear_cache)
{
  byte_t pix;
  int a, i, x, y, z, width, height;

  byte_t *irm = memory->get_irm();
  byte_t *ptr = _bitmap;

  width = get_real_width();
  height = get_real_height();

  i = -1;
  z = 73;
  ptr += 16 * width + 16;
  for (y = 0;y < 256;y += 8)
    {
      for (x = 0;x < 256;x += 8)
        {
	  i++;
	  z++;

	  _dirty[z] = clear_cache;

	  pix = irm[i];
	  if (_pix_cache[i] != pix)
	    _dirty[z]++;

	  if (!_dirty[z])
	    continue;

	  _pix_cache[i] = pix;
          for (a = 0;a < 8;a++)
            generic_put_pixels(ptr + a * width + x, font[8 * pix + a], 0x0100);
	}
      z += 4;
      ptr += 8 * width;
    }
}

void
UI_0::generic_update_64x16(byte_t *font, bool clear_cache)
{
  byte_t pix;
  int a, i, x, y, z, width, height, width8;

  byte_t *irm = memory->get_irm();
  byte_t *ptr = _bitmap;

  width = get_real_width();
  height = get_real_height();
  width8 = width / 8;

  i = -1;
  ptr += 6 * width + 8;
  for (y = 0;y < 16;y++)
    {
      z = (((12 * y + 6) / 8) * width8);
      for (x = 0;x < 512;x += 8)
        {
	  i++;
	  z++;

	  _dirty[z] = clear_cache;

	  pix = irm[i];
	  if (_pix_cache[i] != pix)
	    _dirty[z]++;

	  if (!_dirty[z])
	    continue;

	  _dirty[z + width8]++;

	  _pix_cache[i] = pix;
          for (a = 0;a < 8;a++)
            generic_put_pixels(ptr + a * width + x, font[8 * pix + a], 0x0100);
	}
      ptr += 12 * width;
    }
}

void
UI_0::generic_update(Scanline *scanline, MemAccess *memaccess, bool clear_cache)
{
  byte_t *font = memory->get_char_rom();

  if (_font != font)
    {
      _font = font;
      clear_cache = true;
    }

  switch (_mode)
    {
    case UI_GENERIC_MODE_Z1013_32x32:
      generic_update_32x32(font, clear_cache);
      break;
    case UI_GENERIC_MODE_Z1013_64x16:
      generic_update_64x16(font, clear_cache);
      break;
    case UI_GENERIC_MODE_GDC:
      generic_update_gdc(font, clear_cache);
      break;
    }
}

void
UI_0::generic_signal_v_retrace(bool value)
{
  if (_mode == UI_GENERIC_MODE_GDC)
    if (gdc != NULL)
      gdc->v_retrace(value);
}

int
UI_0::generic_get_mode(void)
{
  return _mode;
}

void
UI_0::generic_set_mode(int mode)
{
  _mode = mode;

  switch (_mode)
    {
    case UI_GENERIC_MODE_Z1013_32x32:
      set_real_screen_size(288, 288);
      break;
    case UI_GENERIC_MODE_Z1013_64x16:
      set_real_screen_size(528, 200);
      break;
    case UI_GENERIC_MODE_GDC:
      set_real_screen_size(640, 200);
      break;
    }

  dispose();
  init();
}
