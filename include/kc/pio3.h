/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-1998 Torsten Paul
 *
 *  $Id: pio3.h,v 1.5 1999/05/01 21:56:46 tp Exp $
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

#ifndef __kc_pio3_h
#define __kc_pio3_h

#include "kc/pio.h"

class PIO3 : public PIO
{
protected:
  virtual void change_A(byte_t changed, byte_t val);
  virtual void change_B(byte_t changed, byte_t val);

public:
  PIO3(void);
  virtual ~PIO3(void);
  virtual byte_t in(word_t addr);
  virtual void out(word_t addr, byte_t val);
};

#endif /* __kc_pio3_h */
