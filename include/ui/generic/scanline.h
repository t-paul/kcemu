/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2002 Torsten Paul
 *
 *  $Id: scanline.h,v 1.1 2002/10/31 01:02:43 torsten_paul Exp $
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

#ifndef __ui_generic_scanline_h
#define __ui_generic_scanline_h

#include "kc/system.h"

class Scanline
{
 private:
  bool       _enabled;
  int        _val;
  int        _idx;
  int        _count;
  word_t     _vec[48];
  word_t    *_v1;
  word_t    *_v2;
  word_t    *_v3;
  long long  _time;
  long long  _frame_time;

 public:
  Scanline(void);
  virtual ~Scanline(void);

  virtual void update(void);
  virtual void trigger(bool enable);
  virtual void reset(bool enable);
  virtual int get_value(int y);
};

#endif /* __ui_generic_scanline_h */
