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

#ifndef __ui_generic_ui_ac1_h
#define __ui_generic_ui_ac1_h

#include "kc/system.h"

#include "ui/generic/ui_base.h"

class UI_AC1 : public UI_Base
{
 protected:
  byte_t *_pix_cache;

  int _lines;
  int _line_shift;
  
 protected:
  inline void generic_put_pixels(byte_t *ptr, byte_t val);

 public:
  UI_AC1(void);
  virtual ~UI_AC1(void);

  virtual void generic_update(Scanline *scanline, MemAccess *memaccess, bool clear_cache);
};

#endif /* __ui_generic_ui_ac1_h */