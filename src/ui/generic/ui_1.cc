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
#include "kc/prefs/types.h"
#include "kc/prefs/prefs.h"

#include "kc/pio1.h"
#include "kc/timer1.h"
#include "kc/memory.h"
#include "kc/module.h"

#include "ui/generic/ui_1.h"

UI_1::UI_1(void)
{
  int a;

  set_real_screen_size(320 + 64, 192 + 64);

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

void
UI_1::generic_put_pixels(byte_t *ptr, byte_t val, word_t color)
{
  for (int a = 0;a < 8;a++)
    ptr[a] = (val & (128 >> a)) ? (color >> 8) : color;
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
  byte_t *font = memory->get_char_rom();
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
            generic_put_pixels(ptr + (a + 32) * width + x + 32, font[8 * pix + a], color);

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
  byte_t *font = memory->get_char_rom();
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
            generic_put_pixels(ptr + (a + 32) * width + x + 32, font[8 * pix + a], color);

        }
      ptr += 10 * width;
      d += 8;
    }
}

void
UI_1::generic_update(Scanline *scanline, MemAccess *memaccess, bool clear_cache)
{
  static byte_t old_flash = 0xff;
  static byte_t old_lmode = 0xff;
  static byte_t old_border = 0xff;
  static byte_t old_color_expansion = 0xff;

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
      clear_cache = true;
    }

  bool color_expansion_installed = !module->is_empty(61);
  if (old_color_expansion != color_expansion_installed)
    {
      old_color_expansion = color_expansion_installed;
      clear_cache = true;
    }

  if (clear_cache)
    old_border = 0xff; // force drawing of screen border

  if (color_expansion_installed || clear_cache)
    {
      byte_t border = color_expansion_installed ? ((PIO1_1 *)pio)->get_border_color() : 0;
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
