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

#include "ui/generic/ui_kramermc.h"

UI_KramerMC::UI_KramerMC(void)
{
  int a;

  set_real_screen_size(64 * 8 + 32, 16 * 8 + 32);

  _dirty_size = (get_real_width() * get_real_height()) / 64;
  _dirty = new byte_t[_dirty_size];
 
  _bitmap = new byte_t[get_real_width() * get_real_height()];
  _pix_cache = new byte_t[_dirty_size];
  
  for (a = 0;a < _dirty_size;a++)
    {
      _dirty[a] = 1;
      _pix_cache[a] = 0xff;
    }
}

UI_KramerMC::~UI_KramerMC(void)
{
}

void
UI_KramerMC::generic_put_pixels(byte_t *ptr, byte_t val)
{
  for (int a = 0;a < 8;a++)
    ptr[a] = (val & (128 >> a)) ? 1 : 0;
}

void
UI_KramerMC::generic_update(Scanline *scanline, MemAccess *memaccess, bool clear_cache)
{
  byte_t *irm = memory->get_irm();
  byte_t *chr = memory->get_char_rom();
  byte_t *ptr = _bitmap;

  int width = get_real_width();

  ptr += 16 * width + 16;

  int z = -1;
  int d = 137;
  byte_t pix;
  for (int y = 0;y < 128;y += 8)
    {
      for (int x = 0;x < 512;x += 8)
	{
	  z++;
	  d++;

	  pix = irm[z];
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
      ptr += 8 * width;
      d += 4;
    }
}
