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

#ifndef __kc_bcs3_graphic_h
#define __kc_bcs3_graphic_h

#include "kc/system.h"

#include "kc/kc.h"

class GraphicBCS3
{
private:
  enum {
    MAX_ROWS = 320,
    MAX_COLS = 40,
    MEM_BYTES = MAX_ROWS * MAX_COLS,
  };
  int _row;
  int _col;
  int _line;
  int _cnt;
  byte_t _mem[MEM_BYTES];
  int _diff[MAX_ROWS];
  unsigned long long _z80_tick;

protected:
  virtual void next_row(void);

public:
  GraphicBCS3(void);
  virtual ~GraphicBCS3(void);

  virtual void memory_read(word_t addr, byte_t ram_val);
  virtual void retrace(void);
  virtual void reset_line_counter(void);
  virtual void increment_line_counter(void);
};

#endif /* __kc_bcs3_graphic_h */
