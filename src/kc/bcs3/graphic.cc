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
#include "kc/ctc.h"
#include "kc/z80.h"
#include "kc/bcs3/graphic.h"

GraphicBCS3::GraphicBCS3(void)
{
  _cnt = 0;
  _row = 0;
  _line = 0;
  memset(_mem, ' ', MEM_BYTES);
}

GraphicBCS3::~GraphicBCS3(void)
{
}

void
GraphicBCS3::memory_read(word_t addr, byte_t ram_val)
{
  if (_diff[_cnt] == 0)
    {
      _diff[_cnt] = z80->getCounter() - _z80_tick;
    }
  //printf("GraphicBCS3::memory_read(): addr = %04x, ram = %02x, line = %d, [%3dx%3d]\n", addr, ram_val, _line, _row, _col);
  _mem[MAX_COLS * _cnt + _col] = ram_val;
  _col++;
}

void
GraphicBCS3::retrace(void)
{
  static long long old_counter = 0;

  _row = 0;


  printf("\033[H%lld        \n", z80->getCounter() - old_counter);
  printf("%d        \n\n", _cnt);
  _cnt = 0;
  old_counter = z80->getCounter();
  for (int y = 0;y < MAX_ROWS;y++)
    {
      bool show = false;
      for (int x = 0;x < MAX_COLS;x++)
        {
          int c = _mem[MAX_COLS * y + x];
          if (c & 0x80)
            continue;
          if ((c != ' ') && (c != '.'))
            show = true;
        }

      if (!show)
        continue;

      printf("%3d %4d |", y, _diff[y]);
      for (int x = 0;x < MAX_COLS;x++)
        {
          int c = _mem[MAX_COLS * y + x];
          printf("%c", isprint(c) ? c : '.');
        }
      printf("|\n");
    }
  memset(_mem, ' ', MEM_BYTES);
  memset(_diff, 0, sizeof(_diff));
}

void
GraphicBCS3::reset_line_counter(void)
{
  z80->set_wait(false);
  //printf("GraphicBCS3::reset_line_counter()\n");
  next_row();
  _line = 7;
}

void
GraphicBCS3::increment_line_counter(void)
{
  _cnt++;
  _z80_tick = z80->getCounter();
  _col = 0;
  //printf("GraphicBCS3::increment_line_counter(): %d -> %d\n", _line, (_line + 1) & 7);
  _line = (_line + 1) & 7;
}

void
GraphicBCS3::next_row(void)
{
  if (++_row >= MAX_ROWS)
    _row = MAX_ROWS - 1;

  _col = 0;
}
