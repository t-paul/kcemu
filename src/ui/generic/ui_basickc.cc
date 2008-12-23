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

#include "kc/basickc/ports.h"

#include "ui/generic/ui_basickc.h"

UI_BasicKC::UI_BasicKC(void)
{
  set_real_screen_size(256 + 32, 256 + 32);
  create_buffer(get_real_width() * get_real_height());

  _dirty_size = (get_real_width() * get_real_height()) / 64;
  _dirty = new byte_t[_dirty_size];
  for (int a = 0;a < _dirty_size;a++)
    _dirty[a] = 1;

  int mem = 8192;
  _pix_cache = new byte_t[mem];
  for (int a = 0;a < mem;a++)
    _pix_cache[a] = 0xff;
}

UI_BasicKC::~UI_BasicKC(void)
{
}

void
UI_BasicKC::generic_put_pixels(byte_t *ptr, byte_t val)
{
  for (int a = 0;a < 8;a++)
    ptr[a] = (val & (1 << a)) ? 1 : 0;
}

void
UI_BasicKC::generic_update(Scanline *scanline, MemAccess *memaccess, bool clear_cache)
{
  word_t video_base = ((PortsBasicKC *)porti)->get_video_base();
  
  byte_t *irm = memory->get_irm() + (video_base << 8);
  byte_t *ptr = _bitmap;

  int width = get_real_width();

  ptr += width * 16 + 16; // border offset

  if (clear_cache)
    memset(_dirty, 1, _dirty_size);

  for (int y = 0;y < 256;y++)
    {
      for (int x = 0;x < 256;x += 8)
        {
          int d = (y / 8) * 36 + (x / 8) + 74;
          int z = y * 32 + (x / 8);
          byte_t pix = irm[z];
          if (_pix_cache[z] != pix)
            {
              _dirty[d]++;
              _pix_cache[z] = pix;
              generic_put_pixels(ptr + width * y + x, pix);
            }
        }
    }
}
