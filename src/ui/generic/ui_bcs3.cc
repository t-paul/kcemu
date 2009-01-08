/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2009 Torsten Paul
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

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/memory.h"

#include "ui/generic/ui_bcs3.h"

UI_BCS3::UI_BCS3(void)
{
  int a;

  set_real_screen_size(40 * 8 + 32, 30 * 8 + 32);

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

UI_BCS3::~UI_BCS3(void)
{
}

void
UI_BCS3::generic_put_pixels(byte_t *ptr, byte_t val)
{
  for (int a = 0;a < 8;a++)
    ptr[a] = (val & (128 >> a)) ? 1 : 0;
}

void
UI_BCS3::generic_update(Scanline *scanline, MemAccess *memaccess, bool clear_cache)
{
  byte_t *chr = memory->get_char_rom();
  byte_t *ptr = _bitmap;

  int width = get_real_width();

  memset(_dirty, 1, _dirty_size);

  bool se31 = memory->memRead8(0x01b0) == '3';
  int rows = se31 ? 29 : 12;
  int cols = se31 ? 29 : 27;
  int base = se31 ? 0x3c80 : 0x3c50;
  for (int y = 0;y < rows;y++)
    {
      for (int x = 0;x < cols;x++)
        {
          int c = memory->memRead8(base + (cols + 1) * y + x);
	  byte_t *chr_ptr = chr + 8 * (c & 0x7f);
	  for (int a = 0;a < 8;a++)
	      generic_put_pixels(ptr + width * a + (8 * x), chr_ptr[a]);
        }
      ptr += 8 * width;
    }
}
