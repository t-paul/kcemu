/*
 *  KCemu -- The emulator for the KC85 homecomputer series and much more.
 *  Copyright (C) 1997-2010 Torsten Paul
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
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <ctype.h>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/z80.h"
#include "kc/fdc7.h"

#include "libdbg/dbg.h"

FDC7::FDC7(void)
{
}

FDC7::~FDC7(void)
{
}

long long
FDC7::get_counter()
{
  return z80->getCounter();
}

void
FDC7::add_callback(unsigned long long offset, Callback *cb, void *data)
{
  z80->addCallback(offset, cb, data);
}

/*
 *  Documentation from Volker...
 *
 *  ; FDC-Ports:
 *  ; ----------
 *  
 *  	'ROBOTRON'
 *  
 *  FDCD         	equ	98h  	; FDC Datenregister
 *  FDCC         	equ	99h  	; FDC Steueregister	
 *  FDCZ        	equ	0A0h 	; FDC Zusatzregister
 *  				; 5 4 3 2 1 0
 *  				; x x 0 0 x x 
 *  				; | |     | |
 *  				; | |     | Motor Laufwerk 0 ein/aus
 *  				; | |     Motor Laufwerk 1 ein/aus
 *  				; | Terminal Count aktivieren/deakt.
 *  				; FDC Reset
 *  
 *  	ELSE Rossendorf
 *  
 *  FDCD         	equ	10h  	; FDC Datenregister
 *  FDCC         	equ	11h  	; FDC Steueregister	
 *  FDCZ        	equ	12h 	; FDC Zusatzregister
 *  				; 5 4 3 2 1 0
 *  				; x x 0 0 x x 
 *  				; | |     | |
 *  				; | |     | Motor Laufwerk 0 ein/aus
 *  				; | |     Motor Laufwerk 1 ein/aus
 *  				; | Terminal Count aktivieren/deakt.
 *  				; FDC Reset
 * 
 */
byte_t
FDC7::in(word_t addr)
{
  byte_t val = 0;

  switch (addr & 0xff)
    {
    case 0x98: // CPM-Z9 module (Data Register, ROBOTRON module)
    case 0x10: // CPM-Z9 module (Data Register, Rossendorf module)
      val = get_msr();
      break;
    case 0x99: // CPM-Z9 module (Status Register, ROBOTRON module)
    case 0x11: // CPM-Z9 module (Status Register, Rossendorf module)
      val = in_data(addr);
      break;
    case 0xa0: // CPM-Z9 module (Extra Register, ROBOTRON module)
    case 0x12: // CPM-Z9 module (Extra Register, Rossendorf module)
      break;
    default:
      DBG(2, form("KCemu/FDC/in_unhandled",
                  "FDC::in(): addr = %04x\n",
                  addr));
      break;
    }

  return val;
}

void
FDC7::out(word_t addr, byte_t val)
{
  switch (addr & 0xff)
    {
    case 0x98: // CPM-Z9 module (Data Register, ROBOTRON module)
    case 0x10: // CPM-Z9 module (Data Register, Rossendorf module)
      write_byte(val);
      break;
    case 0x99: // CPM-Z9 module (Status Register, ROBOTRON module)
    case 0x11: // CPM-Z9 module (Status Register, Rossendorf module)
      out_data(addr, val);
      break;
    case 0xa0: // CPM-Z9 module (Extra Register, ROBOTRON module)
    case 0x12: // CPM-Z9 module (Extra Register, Rossendorf module)
      set_terminal_count((val & 0x10) == 0x10);

      if ((val & 0x20) == 0x20)
	{} // RESET

      break;
    default:
      DBG(2, form("KCemu/FDC/out_unhandled",
                  "FDC::out(): addr = %04x, val = %02x [%c]\n",
                  addr, val, isprint(val) ? val : '.'));
      break;
    }
}
