/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: ui_4.cc,v 1.1 2002/06/09 14:24:34 torsten_paul Exp $
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

#include "ui/generic/ui_4.h"

UI_4::UI_4(void)
{
  _bitmap = new byte_t[get_real_width() * get_real_height()];
}

UI_4::~UI_4(void)
{
}

int
UI_4::get_real_width(void)
{
  return 320;
}

int
UI_4::get_real_height(void)
{
  return 256;
}

void
UI_4::generic_put_pixels(int x, int y, byte_t val, byte_t fg, byte_t bg)
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
UI_4::generic_update(void)
{
  byte_t fg, bg;
  int x, y, col;

  byte_t *irm = memory->getIRM();

  for (y = 0;y < get_real_height();y++)
    for (x = 0;x < get_real_width();x += 8)
      {
	col = irm[32 * x + y + 0x4000];
	fg = (col >> 3) & 15;
	bg = (col & 7) | 0x10;
	generic_put_pixels(x, y, irm[32 * x + y], fg, bg);
      }
}

UI_ModuleInterface *
UI_4::getModuleInterface(void)
{
  static DummyModuleHandler *i = new DummyModuleHandler();
  return i;
}

TapeInterface  *
UI_4::getTapeInterface(void)
{
  static DummyTapeHandler *i = new DummyTapeHandler();
  return i;
}

DebugInterface *
UI_4::getDebugInterface(void)
{
  return 0;
}
