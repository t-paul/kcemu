/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2004 Torsten Paul
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

#include "kc/mod_1m.h"

Module1M::Module1M(Module1M &tmpl) :
  ModuleSegmentedRAM(tmpl)
{
}

Module1M::Module1M(const char *name, byte_t id) :
  ModuleSegmentedRAM(name, id, 64, 0x4000)
{
}

Module1M::~Module1M(void)
{
}

int
Module1M::get_segment_index(word_t addr, byte_t val)
{
  return (val >> 2) & 0x3f;
}

word_t
Module1M::get_base_address(word_t addr, byte_t val)
{
  return 0x8000;
}

ModuleInterface *
Module1M::clone(void)
{
  return new Module1M(*this);
}
