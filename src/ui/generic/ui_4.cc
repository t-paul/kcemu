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
#include "kc/ports.h"
#include "kc/memory.h"

#include "ui/generic/ui_4.h"

UI_4::UI_4(void)
{
  int a, b, y;

  set_real_screen_size(320, 256);

  y = get_real_height();
  b = (get_real_width() * y) / 8;
  _bitmap = new byte_t[get_real_width() * y];
  _dirty_size = b / 8;
  _dirty = new byte_t[_dirty_size];
  _pix_cache = new byte_t[b];
  _col_cache = new byte_t[b];
  _scn_cache = new byte_t[y];

  for (a = 0;a < _dirty_size;a++)
    _dirty[a] = 1;

  for (a = 0;a < b;a++)
    {
      _pix_cache[a] = 0;
      _col_cache[a] = 0;
    }

  for (a = 0;a < y;a++)
    _scn_cache[a] = 0;
  }

UI_4::~UI_4(void)
{
  delete[] _bitmap;
  delete[] _dirty;
  delete[] _pix_cache;
  delete[] _col_cache;
  delete[] _scn_cache;
}

void
UI_4::generic_put_pixels(int x, int y, byte_t val, byte_t fg, byte_t bg)
{
  int idx = y * get_real_width() + x;

  _dirty[(y / 8) * (get_real_width() / 8) + (x / 8)] = 1;

  for (int a = 0;a < 8;a++)
    {
      if (val & 0x80)
	_bitmap[idx++] = fg;
      else
	_bitmap[idx++] = bg;
      val <<= 1;
    }
}

void
UI_4::generic_update_hires(Scanline *scanline, byte_t *irm, bool clear_cache)
{
  int x, y, yy, z, p;
  int yyadd, idx, val, col, changed;

  p = 0;
  yy = 0;
  yyadd = get_real_width();
  for (y = 0;y < get_real_height();y++)
    {
      for (x = 0;x < 320;x += 8)
	{
	  changed = clear_cache;

	  idx = 32 * x + y;
	  val = irm[idx];
	  col = irm[idx + 0x4000];

	  if (val != _pix_cache[p])
	    {
	      changed++;
	      _pix_cache[p] = val;
	    }

	  if (col != _col_cache[p])
	    {
	      changed++;
	      _col_cache[p] = col;
	    }

	  p++;

	  if (!changed)
	    continue;

	  _dirty[(y / 8) * (yyadd / 8) + (x / 8)] = 1;

	  for (z = 0;z < 8;z++)
	    {
	      /*
	       *  some bit juggling to map one bit from val and
	       *  col to the color index for the current pixel
	       *
	       *  mapping is:
	       *
	       *  val | col | color index | color
	       *  ----+-----+-------------+------
	       *   0  |  0  |  0  (000b)  | black
	       *   0  |  1  |  5  (101b)  | cyan
	       *   1  |  0  |  2  (010b)  | red
	       *   1  |  1  |  7  (111b)  | white
	       */
	      val <<= 1;
	      _bitmap[yy + x + z] = ((val & 256) | (5 * (col & 128))) >> 7;
	      col <<= 1;
	    }
	}
      yy += yyadd;
    }
}

void
UI_4::generic_update_lores(Scanline *scanline, byte_t *irm, bool clear_cache)
{
  int x, y, p, s;
  int idx, changed;
  bool scanline_changed;
  byte_t val, col, fg, bg;

  p = 0;
  for (y = 0;y < get_real_height();y++)
    {
      s = 0;
      if (scanline)
	s = scanline->get_value(y);

      scanline_changed = false;
      if (s != _scn_cache[y])
	{
	  _scn_cache[y] = s;
	  scanline_changed = true;
	}

      for (x = 0;x < 40;x++)
	{
	  changed = clear_cache;

	  idx = 256 * x + y;
	  val = irm[idx];
	  col = irm[idx + 0x4000];

	  if (val != _pix_cache[p])
	    {
	      changed++;
	      _pix_cache[p] = val;
	    }

	  if (col != _col_cache[p])
	    {
	      changed++;
	      _col_cache[p] = col;
	    }

	  bg = (col & 7) | 0x10;
	  fg = (col >> 3) & 15;

	  if (col & 128)
	    {
	      if (scanline_changed)
		changed++;
	      if (s)
		fg = bg;
	    }

	  p++;

	  if (!changed)
	    continue;

	  generic_put_pixels(8 * x, y, val, fg, bg);
	}
    }
}

void
UI_4::generic_update(Scanline *scanline, MemAccess *memaccess, bool clear_cache)
{
  int a;
  int hires;
  byte_t *irm;
  static byte_t *irm_old = NULL;
  static int hires_old = -1;

  irm = memory->get_irm();
  if (irm != irm_old)
    {
      /*
       *  on screen switch we need to ignore the
       *  display cache
       */
      irm_old = irm;
      clear_cache = true;
    }

  hires = ((ports->in(0x84) & 8) == 0);
  if (hires != hires_old)
    {
      /*
       *  when switching lores/hires mode ignore the
       *  display cache too
       */
      hires_old = hires;
      clear_cache = true;
    }

  if (clear_cache)
    for (a = 0;a < _dirty_size;a++)
      _dirty[a] = 1;

  if (hires)
    generic_update_hires(scanline, irm, clear_cache);
  else
    generic_update_lores(scanline, irm, clear_cache);
}
