/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2004 Torsten Paul
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

#include "ui/generic/ui_kramermc.h"

UI_KramerMC::UI_KramerMC(void)
{
  int a;

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

int
UI_KramerMC::get_real_width(void)
{
  return 64 * 8 + 32;
}
 
int
UI_KramerMC::get_real_height(void)
{
  return 16 * 8 + 32;
}

byte_t *
UI_KramerMC::get_dirty_buffer(void)
{
  return _dirty;
}
 
int
UI_KramerMC::get_dirty_buffer_size(void)
{
  return _dirty_size;
}

void
UI_KramerMC::generic_put_pixels(byte_t *ptr, byte_t val)
{
  for (int a = 0;a < 8;a++)
    ptr[a] = (val & (128 >> a)) ? 1 : 0;
}

void
UI_KramerMC::generic_update(bool clear_cache)
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

int
UI_KramerMC::generic_get_mode(void)
{
  return 0;
}

void
UI_KramerMC::generic_set_mode(int mode)
{
}
