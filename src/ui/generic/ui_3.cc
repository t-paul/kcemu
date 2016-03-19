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
#include "kc/memory.h"

#include "ui/generic/ui_3.h"

UI_3::UI_3(void)
{
  int a, b, n1, n2;

  set_real_screen_size(320, 256);

  b = (get_real_width() * get_real_height()) / 8;
  _bitmap = new byte_t[get_real_width() * get_real_height()];
  _dirty_size = b / 8;
  _dirty = new byte_t[_dirty_size];
  _pix_cache = new byte_t[b];
  _col_cache = new byte_t[b];
  _scan_cache = new byte_t[b];

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

  for (a = 0;a < b;a++)
    {
      _pix_cache[a] = 0;
      _col_cache[a] = 0;
      _scan_cache[a] = 0;
    }
}

UI_3::~UI_3(void)
{
  delete[] _dirty;
  delete[] _bitmap;
  delete[] _pix_cache;
  delete[] _col_cache;
  delete[] _scan_cache;
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
UI_3::generic_update(Scanline *scanline, MemAccess *memaccess, bool clear_cache)
{
  int x, y;
  int changed;
  int p, pc, ys, yc, s;
  byte_t val, col, fg, bg;

  byte_t *irm = memory->get_irm();

  p = 0;
  for (y = 0;y < 256;y++)
    {
      pc = (p & 0x7f) | (((p & 0xfe00) >> 2) + 0x2800);
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

	  if (memaccess)
	    {
	      byte_t m = memaccess->get_value(x, y);

	      switch (m)
		{
		case 2:
		  /*
		   *  access in the current frame
		   */
		  if ((fg == bg) || (val == 0))
		    {
		      /*
		       *  nothing to change so we don't need to
		       *  update in the next frame
		       */
		      memaccess->set_value(x, y, 0);
		    }
		  else
		    {
		      fg = bg;
		      changed++;
		    }
		  break;
		case 1:
		  /*
		   *  access in the previous frame
		   */
		  changed++;
		  break;
		}
	    }

	  if (col & 128)
	    {
	      if (_scan_cache[40 * y + x] != s)
		{
		  changed++;
		  _scan_cache[40 * y + x] = s;
		  if (s)
		    fg = bg;
		}
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

	  if (memaccess)
	    {
	      byte_t m = memaccess->get_value(32 + x, y);

	      switch (m)
		{
		case 2:
		  /*
		   *  access in the current frame
		   */
		  if ((fg == bg) || (val == 0))
		    {
		      /*
		       *  nothing to change so we don't need to
		       *  update in the next frame
		       */
		      memaccess->set_value(32 + x, y, 0);
		    }
		  else
		    {
		      fg = bg;
		      changed++;
		    }
		  break;
		case 1:
		  /*
		   *  access in the previous frame
		   */
		  changed++;
		  break;
		}
	    }

	  if (col & 128)
	    {
	      if (_scan_cache[40 * y + x + 32] != s)
		{
		  changed++;
		  _scan_cache[40 * y + x + 32] = s;
		  if (s)
		    fg = bg;
		}
	    }

          p++;

	  if (!changed)
	    continue;

          generic_put_pixels(8 * x + 256, ys, val, fg, bg);
        }
    }
}
