/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: sh_mem.h,v 1.2 2001/04/14 15:14:51 tp Exp $
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

#ifndef __kc_sh_mem_h
#define __kc_sh_mem_h

#include "kc/config.h"
#include "kc/system.h"

#include "kc/ports.h"

class FloppySharedMem : public PortInterface
{
 private:
  byte_t *_mem;
  
 public:
  FloppySharedMem(void);
  virtual ~FloppySharedMem(void);

  virtual void set_memory(byte_t *mem);
  
  virtual byte_t in(word_t addr);
  virtual void out(word_t addr, byte_t val);
};

#endif /* __kc_sh_mem_h */

