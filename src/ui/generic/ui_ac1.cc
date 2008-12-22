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

#include "kc/prefs/prefs.h"

#include "ui/generic/ui_ac1.h"

UI_AC1::UI_AC1(void)
{
  set_real_screen_size(64 * 6 + 32, 32 * 8 + 32);

  switch (Preferences::instance()->get_kc_variant())
    {
    case KC_VARIANT_AC1_16:
      _lines = 16;
      _line_shift = 1;
      break;
    default:
      _lines = 32;
      _line_shift = 0;
      break;
    }

  int size = get_real_width() * get_real_height();
  _dirty_size = size / 64;
  _dirty = new byte_t[_dirty_size];
 
  _bitmap = new byte_t[get_real_width() * get_real_height()];

  _pix_cache = new byte_t[32 * 64];

  for (int a = 0;a < size;a++)
    _bitmap[a] = 0;
  
  for (int a = 0;a < _dirty_size;a++)
    {
      _dirty[a] = 1;
      _pix_cache[a] = 0xff;
    }
}

UI_AC1::~UI_AC1(void)
{
}

void
UI_AC1::generic_put_pixels(byte_t *ptr, byte_t val)
{
  for (int a = 0;a < 6;a++)
    ptr[a] = (val & (1 << a)) ? 1 : 0;
}

void
UI_AC1::generic_update(Scanline *scanline, MemAccess *memaccess, bool clear_cache)
{
  byte_t *irm = memory->get_irm();
  byte_t *chr = memory->get_char_rom();

  int width = get_real_width();

  byte_t *ptr = _bitmap + 2 * 8 * width + 16;
  int ptr_add = (8 << _line_shift) * width;

  if (clear_cache)
    memset(_dirty, 1, _dirty_size);

  int z = _lines * 64;
  for (int y = 0;y < _lines;y++)
    {
      int yy = y << _line_shift;
      byte_t *dirty_ptr = _dirty + 106 + yy * 52;

      int d = 0;
      for (int x = 0;x < 64;x++)
        {
          z--;
          byte_t pix = irm[z];

          if (_pix_cache[z] != pix)
            {
              dirty_ptr[d / 8]++;
              dirty_ptr[(d + 6) / 8]++;
              _pix_cache[z] = pix;
            }
          d += 6;

          byte_t *chr_ptr = chr + 8 * pix;
	  for (int a = 0;a < 8;a++)
	    generic_put_pixels(ptr + width * a + x * 6, chr_ptr[a]);
        }

      ptr += ptr_add;
    }
}
