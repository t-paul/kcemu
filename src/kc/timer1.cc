/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: timer1.cc,v 1.1 2002/10/31 01:46:36 torsten_paul Exp $
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
#include "kc/z80.h"
#include "kc/timer1.h"

Timer1::Timer1(void) : Timer("Timer1")
{
  _flash = 0;
}

Timer1::~Timer1(void)
{
}

void
Timer1::start(void)
{
  z80->addCallback(0, this, 0);
}

void
Timer1::callback(void * /* data */)
{
  _flash = 1 - _flash;
  z80->addCallback(312500, this, 0);
}

byte_t
Timer1::get_flash()
{
  return _flash;
}
