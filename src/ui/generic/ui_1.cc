/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: ui_1.cc,v 1.1 2002/06/09 14:24:34 torsten_paul Exp $
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

#include "ui/font1.h"

#include "ui/generic/ui_1.h"

UI_1::UI_1(void)
{
  int a, b;

  b = (get_real_width() * get_real_height()) / 64;
  _bitmap = new byte_t[get_real_width() * get_real_height()];
  _dirty = new byte_t[b];
  _pix_cache = new byte_t[b];
  _col_cache = new byte_t[b];
  for (a = 0;a < b;a++)
    _dirty[a] = 1;
}

UI_1::~UI_1(void)
{
  delete _dirty;
  delete _bitmap;
  delete _pix_cache;
  delete _col_cache;
}

int
UI_1::get_real_width(void)
{
  return 320;
}

int
UI_1::get_real_height(void)
{
  return 192;
}

void
UI_1::generic_put_pixels(byte_t *ptr, byte_t val, word_t color)
{
  for (int a = 0;a < 8;a++)
    ptr[a] = (val & (1 << a)) ? (color >> 8) : color;
}

void
UI_1::generic_update(void)
{
  word_t color;
  byte_t pix, col, fg, bg, c;
  int a, x, y, z, width, height, changed;

  byte_t *irm = memory->getIRM();
  byte_t *ptr = _bitmap;

  width = get_real_width();
  height = get_real_height();

  z = -1;
  for (y = 0;y < 192;y += 8)
    {
      for (x = 0;x < 320;x += 8)
        {
	  z++;
	  col = irm[z];
	  if (_col_cache[z] != col)
	    _dirty[z]++;

          pix = irm[0x400 + z];
	  if (_pix_cache[z] != pix)
	    _dirty[z]++;

	  if (!_dirty[z])
	    continue;

	  _col_cache[z] = col;
	  _pix_cache[z] = pix;
	  color = (0x0011 * col) & 0x0707;
          for (a = 0;a < 8;a++)
            generic_put_pixels(ptr + a * width + x, __font[8 * pix + a], color);

        }
      ptr += 8 * width;
    }
}

UI_ModuleInterface *
UI_1::getModuleInterface(void)
{
  static DummyModuleHandler *i = new DummyModuleHandler();
  return i;
}

TapeInterface  *
UI_1::getTapeInterface(void)
{
  static DummyTapeHandler *i = new DummyTapeHandler();
  return i;
}

DebugInterface *
UI_1::getDebugInterface(void)
{
  return 0;
}
