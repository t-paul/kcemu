/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
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

#ifndef __kc_svg_h
#define __kc_svg_h

#include "kc/system.h"

#include "kc/ports.h"

class SVG : public PortInterface
{
 private:
  enum {
    A = 0,
    B = 1,
    C = 2,
  };

 private:
  byte_t _val_a0;
  byte_t _val_a1;
  byte_t _val_a8;
  byte_t _val_aa;
  byte_t _val_ab;
  int    _page_info[4];
  int    _sound_reg[14];
  int    _sound_val[3];
  int    _sound_freq[3];

 protected:
  void set_page(int page, int slot);
  
 public:
  SVG(void);
  virtual ~SVG(void);

  virtual byte_t in(word_t addr);
  virtual void out(word_t addr, byte_t val);
};

#endif /* __kc_svg_h */
