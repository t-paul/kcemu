/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: ui_3.cc,v 1.2 2002/10/31 01:02:47 torsten_paul Exp $
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
#include "kc/memory.h"

#include "ui/generic/ui_3.h"

UI_3::UI_3(void)
{
  int a, b, n1, n2;

  b = (get_real_width() * get_real_height()) / 8;
  _bitmap = new byte_t[get_real_width() * get_real_height()];
  _dirty_size = b / 8;
  _dirty = new byte_t[_dirty_size];
  _pix_cache = new byte_t[b];
  _col_cache = new byte_t[b];

  for (a = 0;a < 256;a++)
    {
      n1 = (a & 0x03) << 2;
      n2 = (a & 0x0c) >> 2;
      _bitswap1[a] = ((a & 0xf0) | n1 | n2);
      n1 = (a & 0x03) << 4;
      n2 = (a & 0x30) >> 4;
      _bitswap2[a] = ((a & 0xcc) | n1 | n2);
    }

  for (a = 0;a < 256;a++)
    _bitswap3[a] = 0x3000 + (_bitswap1[_bitswap2[a] >> 2] << 3);

  for (a = 0;a < _dirty_size;a++)
    _dirty[a] = 1;
}

UI_3::~UI_3(void)
{
  delete _bitmap;
}

int
UI_3::get_real_width(void)
{
  return 320;
}

int
UI_3::get_real_height(void)
{
  return 256;
}

byte_t *
UI_3::get_dirty_buffer(void)
{
  return _dirty;
}

int
UI_3::get_dirty_buffer_size(void)
{
  return _dirty_size;
}

void
UI_3::generic_put_pixels(int x, int y, byte_t val, byte_t fg, byte_t bg)
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
UI_3::generic_update(Scanline *scanline, bool clear_cache)
{
  int x, y;
  int changed;
  int p, pc, ys, yc, s;
  byte_t val, col, fg, bg;

  byte_t *irm = memory->get_irm();

  p = 0;
  for (y = 0;y < 256;y++)
    {
      pc = (p & 0x7f) | ((p & 0xfe00) >> 2) + 0x2800;
      ys = _bitswap1[y];

      s = 0;
      if (scanline)
	s = scanline->get_value(ys);

      for (x = 0;x < 32;x++)
        {
	  changed = clear_cache;

          val = irm[p];
          col = irm[pc + x];

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
	      changed++;
	      if (s)
		fg = bg;
	    }

          p++;

	  if (!changed)
	    continue;

          generic_put_pixels(8 * x, ys, val, fg, bg);
        }
    }

  p = 0x2000;
  for (y = 0;y < 256;y++)
    {
      ys = _bitswap2[y];
      yc = _bitswap3[y];

      s = 0;
      if (scanline)
	s = scanline->get_value(ys);

      for (x = 0;x < 8;x++)
        {
	  changed = clear_cache;

          val = irm[p];
          col = irm[yc + x];

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
	      changed++;
	      if (s)
		fg = bg;
	    }

          p++;

	  if (!changed)
	    continue;

          generic_put_pixels(8 * x + 256, ys, val, fg, bg);
        }
    }
}

int
UI_3::generic_get_mode(void)
{
  return 0;
}

void
UI_3::generic_set_mode(int mode)
{
}
