/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-1998 Torsten Paul
 *
 *  $Id: ic.h,v 1.5 2000/05/21 17:33:51 tp Exp $
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

#ifndef __kc_ic_h
#define __kc_ic_h

#include <iostream.h>

#include "kc/system.h"

class InterfaceCircuit
{
private:
  InterfaceCircuit *_next;

public:
  InterfaceCircuit(void) { _next = 0; }

  virtual void iei(byte_t val) = 0;

  virtual void ieo(byte_t val) {
    if (_next)
      {
	// cerr << "InterfaceCircuit::ieo()..." << endl;
	_next->iei(val);
      }
    else
      {
	// cerr << "InterfaceCircuit::ieo(): end" << endl;
      }
  }

  virtual void next(InterfaceCircuit *ic) { _next = ic; }

  virtual void reset(bool power_on = false) = 0;
  virtual void reti(void) = 0;
};

#endif __kc_ic_h
