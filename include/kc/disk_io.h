/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: disk_io.h,v 1.3 2002/06/09 14:24:32 torsten_paul Exp $
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

#ifndef __kc_disk_io_h
#define __kc_disk_io_h

#include "kc/system.h"

#include "kc/ports.h"

class FloppyIO : public PortInterface
{
 private:
  byte_t _val;
  bool _bit[4];
  
 public:
  FloppyIO(void);
  virtual ~FloppyIO(void);

  virtual byte_t in(word_t addr);
  virtual void out(word_t addr, byte_t val);

  virtual void activate(int bit_nr);
};

#endif /* __kc_disk_io_h */

