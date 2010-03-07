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

#include "ui/generic/ui_muglerpc.h"

UI_MuglerPC::UI_MuglerPC(void)
{
  int a;

  set_real_screen_size(64 * 8 + 32, 32 * 8 + 32);

  _dirty_size = (get_real_width() * get_real_height()) / 64;
  _dirty = new byte_t[_dirty_size];

  create_buffer(get_real_width() * get_real_height());
  _pix_cache = new byte_t[_dirty_size];
  
  for (a = 0;a < _dirty_size;a++)
    {
      _dirty[a] = 1;
      _pix_cache[a] = 0xff;
    }
}

UI_MuglerPC::~UI_MuglerPC(void)
{
}

void
UI_MuglerPC::generic_put_pixels(byte_t *ptr, byte_t val)
{
  for (int a = 0;a < 8;a++)
    ptr[a] = (val & (128 >> a)) ? 1 : 0;
}

void
UI_MuglerPC::generic_update(Scanline *scanline, MemAccess *memaccess, bool clear_cache)
{
  byte_t *irm = memory->get_irm();
  byte_t *chr = memory->get_char_rom();
  byte_t *ptr = _bitmap;

  int width = get_real_width();

  ptr += 16 * width + 16;

  int d = 137;
  int offset = 0;
  for (int y = 0;y < 256;y += 8)
    {
      offset = 0x400 - offset;
      int z = (y / 16) * 64 + offset - 1;

      for (int x = 0;x < 512;x += 8)
	{
	  z++;
	  d++;

	  byte_t pix = irm[z];
	  if (_pix_cache[z] != pix)
	    _dirty[d]++;

	  if (clear_cache)
	    _dirty[d]++;

	  if (!_dirty[d])
	    continue;

	  _pix_cache[z] = pix;

	  byte_t *chr_ptr = chr + 8 * pix;
	  for (int a = 0;a < 8;a++)
	      generic_put_pixels(ptr + width * a + x, chr_ptr[a]);
	}

      d += 4;
      ptr += 8 * width;
    }
}
