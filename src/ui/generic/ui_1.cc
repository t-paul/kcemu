/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: ui_1.cc,v 1.2 2002/10/31 01:02:47 torsten_paul Exp $
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

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/pio1.h"
#include "kc/timer1.h"
#include "kc/memory.h"

#include "ui/font1.h"

#include "ui/generic/ui_1.h"

UI_1::UI_1(void)
{
  int a;

  _dirty_size = (get_real_width() * get_real_height()) / 64;
  _dirty = new byte_t[_dirty_size];

  _bitmap = new byte_t[get_real_width() * get_real_height()];
  _pix_cache = new byte_t[_dirty_size];
  _col_cache = new byte_t[_dirty_size];

  for (a = 0;a < _dirty_size;a++)
    {
      _dirty[a] = 1;
      _pix_cache[a] = 0xff;
      _col_cache[a] = 0xff;
    }
}

UI_1::~UI_1(void)
{
  delete[] _dirty;
  delete[] _bitmap;
  delete[] _pix_cache;
  delete[] _col_cache;
}

int
UI_1::get_real_width(void)
{
  return 320 + 64;
}

int
UI_1::get_real_height(void)
{
  return 192 + 64;
}

byte_t *
UI_1::get_dirty_buffer(void)
{
  return _dirty;
}

int
UI_1::get_dirty_buffer_size(void)
{
  return _dirty_size;
}

void
UI_1::generic_put_pixels(byte_t *ptr, byte_t val, word_t color)
{
  for (int a = 0;a < 8;a++)
    ptr[a] = (val & (1 << a)) ? (color >> 8) : color;
}

void
UI_1::generic_set_border_24(int width, int height, byte_t border)
{
  int a, x, y, z;

  byte_t *ptr = _bitmap;

  z = -1;
  for (y = 0;y < height;y += 8)
    for (x = 0;x < width;x += 8)
      {
	z++;
	if ((y >= 32) && (y < 224) && (x >= 32) && (x < 352))
	  continue;

	_dirty[z]++;
	for (a = 0;a < 8;a++)
	  generic_put_pixels(ptr + (y + a) * width + x, 0, border);
      }
}

void
UI_1::generic_set_border_20(int width, int height, byte_t border)
{
  int x, y;

  byte_t *ptr = _bitmap;

  for (y = 40;y < 232;y += 10)
    for (x = 32;x < 352;x += 8)
      {
	generic_put_pixels(ptr + y * width + x, 0, border);
	generic_put_pixels(ptr + (y + 1) * width + x, 0, border);
      }
}

void
UI_1::generic_update_24(int width, int height, int fchg, byte_t flash, bool clear_cache)
{
  word_t color;
  byte_t pix, col;
  int a, x, y, z, d;

  byte_t *irm = memory->get_irm();
  byte_t *ptr = _bitmap;

  z = -1;
  d = 195;
  for (y = 0;y < 192;y += 8)
    {
      for (x = 0;x < 320;x += 8)
        {
	  z++;
	  d++;
	  col = irm[z];
	  if (_col_cache[z] != col)
	    _dirty[d]++;
	  if (fchg && (col & 0x80))
	    _dirty[d]++;

          pix = irm[0x400 + z];
	  if (_pix_cache[z] != pix)
	    _dirty[d]++;

	  if (clear_cache)
	    _dirty[d]++;

	  if (!_dirty[d])
	    continue;

	  _col_cache[z] = col;
	  _pix_cache[z] = pix;

	  color = (0x0011 * col) & 0x0707;
	  if (flash && (col & 0x80))
	    color = ((color << 8) & 0x0700) | ((color >> 8) & (0x0007));

          for (a = 0;a < 8;a++)
            generic_put_pixels(ptr + (a + 32) * width + x + 32, __font[8 * pix + a], color);

        }
      ptr += 8 * width;
      d += 8;
    }
}

void
UI_1::generic_update_20(int width, int height, int fchg, byte_t flash, bool clear_cache)
{
  word_t color;
  byte_t pix, col;
  int a, x, y, z, d;

  byte_t *irm = memory->get_irm();
  byte_t *ptr = _bitmap;

  z = -1;
  d = 195;
  for (y = 0;y < 200;y += 10)
    {
      for (x = 0;x < 320;x += 8)
        {
	  z++;
	  d = (y / 8 + 4) * 48 + (x / 8) + 4;
	  col = irm[z];
	  if (_col_cache[z] != col)
	    _dirty[d]++;

          pix = irm[0x400 + z];
	  if (_pix_cache[z] != pix)
	    _dirty[d]++;

	  if (clear_cache)
	    _dirty[d]++;

	  if (!_dirty[d])
	    continue;

	  _dirty[d + 48]++;

	  _col_cache[z] = col;
	  _pix_cache[z] = pix;

	  color = (0x0011 * col) & 0x0707;

          for (a = 0;a < 8;a++)
            generic_put_pixels(ptr + (a + 32) * width + x + 32, __font[8 * pix + a], color);

        }
      ptr += 10 * width;
      d += 8;
    }
}

void
UI_1::generic_update(bool clear_cache)
{
  static byte_t old_flash = 0xff;
  static byte_t old_lmode = 0xff;
  static byte_t old_border = 0xff;

  int fchg = 0;
  int width = get_real_width();
  int height = get_real_height();

  int flash = ((Timer1 *)timer)->get_flash();
  if (old_flash != flash)
    {
      fchg = 1;
      old_flash = flash;
    }

  byte_t lmode = ((PIO1_1 *)pio)->get_line_mode();
  if (old_lmode != lmode)
    {
      old_lmode = lmode;
      old_border = 0xff; // force drawing of screen border
      clear_cache = true;
    }

  if (get_kc_type() == KC_TYPE_87)
    {
      byte_t border = ((PIO1_1 *)pio)->get_border_color();
      if (old_border != border)
	{
	  old_border = border;
	  
	  generic_set_border_24(width, height, border);
	  if (lmode)
	    generic_set_border_20(width, height, border);
	}
    }

  if (lmode)
    generic_update_20(width, height, fchg, flash, clear_cache);
  else
    generic_update_24(width, height, fchg, flash, clear_cache);
}

int
UI_1::generic_get_mode(void)
{
  return 0;
}

void
UI_1::generic_set_mode(int mode)
{
}
