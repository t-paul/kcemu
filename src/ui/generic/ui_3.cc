/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: ui_3.cc,v 1.1 2002/06/09 14:24:34 torsten_paul Exp $
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
  int a, n1, n2;

  _bitmap = new byte_t[get_real_width() * get_real_height()];

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

void
UI_3::generic_put_pixels(int x, int y, byte_t val, byte_t fg, byte_t bg)
{
  int idx = y * get_real_width() + x;

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
UI_3::generic_update(void)
{
  int x, y;
  int p, pc, ys, yc;
  byte_t val, col, fg, bg;

  byte_t *irm = memory->getIRM();

  p = 0;
  for (y = 0;y < 256;y++)
    {
      pc = (p & 0x7f) | ((p & 0xfe00) >> 2) + 0x2800;
      ys = _bitswap1[y];
      for (x = 0;x < 32;x++)
        {
          val = irm[p];
          col = irm[pc + x];
          p++;

          bg = (col & 7) | 0x10;
          fg = (col >> 3) & 15;

          generic_put_pixels(8 * x, ys, val, fg, bg);
        }
    }

  p = 0x2000;
  for (y = 0;y < 256;y++)
    {
      ys = _bitswap2[y];
      yc = _bitswap3[y];
      for (x = 0;x < 8;x++)
        {
          val = irm[p];
          col = irm[yc + x];
          p++;

          bg = (col & 7) | 0x10;
          fg = (col >> 3) & 15;

          generic_put_pixels(8 * x + 256, ys, val, fg, bg);
        }
    }
}

UI_ModuleInterface *
UI_3::getModuleInterface(void)
{
  static DummyModuleHandler *i = new DummyModuleHandler();
  return i;
}

TapeInterface  *
UI_3::getTapeInterface(void)
{
  static DummyTapeHandler *i = new DummyTapeHandler();
  return i;
}

DebugInterface *
UI_3::getDebugInterface(void)
{
  return 0;
}

