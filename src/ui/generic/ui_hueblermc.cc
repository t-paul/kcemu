/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2008 Torsten Paul
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

#include "ui/generic/ui_hueblermc.h"

UI_HueblerMC::UI_HueblerMC(void)
{
  int a;

  set_real_screen_size(64 * 6 + 32, 24 * 10 + 32);

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

UI_HueblerMC::~UI_HueblerMC(void)
{
}

void
UI_HueblerMC::generic_put_pixels(byte_t *ptr, byte_t val)
{
  for (int a = 0;a < 6;a++)
    ptr[a] = (val & (128 >> a)) ? 1 : 0;
}

void
UI_HueblerMC::generic_update(Scanline *scanline, MemAccess *memaccess, bool clear_cache)
{
  byte_t *irm = memory->get_irm();
  byte_t *chr = memory->get_char_rom();
  byte_t *ptr = _bitmap;

  int width = get_real_width();

  ptr += width * 16 + 16; // border offset

  memset(_dirty, 1, _dirty_size);
  for (int y = 0;y < 240;y += 10)
    {
      for (int x = 0;x < 384;x += 6)
	{
          int z = (y / 10) * 64 + (x / 6);
          byte_t pix = irm[z];

	  byte_t *chr_ptr = chr + 8 * (pix & 0x7f);
	  for (int a = 0;a < 8;a++)
            {
              byte_t val = pix & 0x80 ? chr_ptr[a] ^ 0xff : chr_ptr[a];
	      generic_put_pixels(ptr + width * (y + a) + x, val);
            }
	}
    }
}
