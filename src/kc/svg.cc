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

#include <ctype.h>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/svg.h"
#include "kc/tape.h"
#include "kc/keyb9.h"
#include "kc/memory9.h"

#include "libdbg/dbg.h"

SVG::SVG(void)
{
  _val_a8 = 0;
  _val_aa = 0;
  _val_ab = 0;
  _page_info[0] = 0;
  _page_info[1] = 0;
  _page_info[2] = 0;
  _page_info[3] = 0;
}

SVG::~SVG(void)
{
}

byte_t
SVG::in(word_t addr)
{
  byte_t val = 0xff;

  switch (addr & 0xff)
    {
    case 0xa8:
      val = _val_a8;
      DBG(2, form("KCemu/SVG/in_A8",
		  "SVG::in():  %04xh val = %02x\n",
		  addr, val));
      break;
    case 0xa9:
      /*
       *  keyboard input
       */
      val = ((Keyboard9 *)keyboard)->get_val();
      DBG(2, form("KCemu/SVG/in_A9",
		  "SVG::in():  %04xh val = %02x\n",
		  addr, val));
      break;
    case 0xaa:
      val = _val_aa;
      DBG(2, form("KCemu/SVG/in_AA",
		  "SVG::in():  %04xh val = %02x\n",
		  addr, val));
      break;
    case 0xab:
      val = _val_ab;
      DBG(2, form("KCemu/SVG/in_AB",
		  "SVG::in():  %04xh val = %02x\n",
		  addr, val));
      break;
    }

  DBG(2, form("KCemu/SVG/in",
              "SVG::in():  %04xh val = %02x\n",
              addr, val));

  return val;
}

void
SVG::out(word_t addr, byte_t val)
{
  int bit;

  DBG(2, form("KCemu/SVG/out",
              "SVG::out(): %04xh val = %02x [%c]\n",
              addr, val, isprint(val) ? val : '.'));

  switch (addr & 0xff)
    {
      /*
       *  change memory configuration:
       *
       *  +---------+---------+---------+---------+
       *  |         |         |         |         |
       *  | Page  3 | Page  2 | Page  1 | Page  0 |
       *  |         |         |         |         |
       *  +----+----+----+----+----+----+----+----+
       *       |         |         |         |
       *       |         |         |         |
       *       +---------+---------+---------+-------  00 - Slot 0
       *                                               01 - Slot 1
       *                                               02 - Slot 2
       *                                               03 - Slot 3
       *
       */
    case 0xa8:
      DBG(2, form("KCemu/SVG/out_A8",
		  "SVG::out(): %04xh val = %02x [%c]\n",
		  addr, val, isprint(val) ? val : '.'));

      if ((_val_a8 & 0x03) != (val & 0x03))
	set_page(0, val & 0x03);
      if ((_val_a8 & 0x0c) != (val & 0x0c))
	set_page(1, (val & 0x0c) >> 2);
      if ((_val_a8 & 0x30) != (val & 0x30))
	set_page(2, (val & 0x30) >> 4);
      if ((_val_a8 & 0xc0) != (val & 0xc0))
	set_page(3, (val & 0xc0) >> 6);
      _val_a8 = val;
      break;
    case 0xaa:
      DBG(2, form("KCemu/SVG/out_AA",
		  "SVG::out(): %04xh val = %02x [%c]\n",
		  addr, val, isprint(val) ? val : '.'));

      /*
       *  bit 0: output keyboard
       *  bit 1: output keyboard
       *  bit 2: output keyboard
       *  bit 3: output keyboard
       *  bit 4: tape motor (0 = on / 1 = off)
       *  bit 5: tape data
       *  bit 6: green led
       *  bit 7: key click (0 = off / 1 = on)
       *
       */
      ((Keyboard9 *)keyboard)->set_val(val & 15);
      _val_aa = val;
      break;
    case 0xab:
      DBG(2, form("KCemu/SVG/out_AB",
		  "SVG::out(): %04xh val = %02x %d%d%d%d%d%d%d%d\n",
		  addr, val,
		  (val & 128) >> 7,
		  (val &  64) >> 6,
		  (val &  32) >> 5,
		  (val &  16) >> 4,
		  (val &   8) >> 3,
		  (val &   4) >> 2,
		  (val &   2) >> 1,
		  (val &   1)));

      bit = val & 1;
      switch (val & 0x0e)
	{
	case 0x08:
	  DBG(2, form("KCemu/SVG/TAPEMOTOR",
		      "SVG::out(): TAPE MOTOR: %s\n",
		      bit ? "off" : "on"));
	  tape->power(!bit);
	  break;
	case 0x0a:
	  DBG(2, form("KCemu/SVG/TAPEDATA",
		      "SVG::out(): TAPE DATA: %d\n",
		      bit));
	  tape->tape_signal();
	  break;
	case 0x0c:
	  DBG(2, form("KCemu/SVG/CAPSLOCKLED",
		      "SVG::out(): CAPS LOCK LED: %s\n",
		      bit ? "off" : "on"));
	  break;
	case 0x0e:
	  DBG(2, form("KCemu/SVG/KEYCLICK",
		      "SVG::out(): KEYCLICK: %s\n",
		      bit ? "on" : "off"));
	  break;
	}
      _val_ab = val;
      break;
    }
}

void
SVG::set_page(int page, int slot)
{
  _page_info[page] = slot;
#if 0
  cout << "setting page " << page << " to slot " << slot
       << " => [" << _page_info[0]
       << "/" << _page_info[1]
       << "/" << _page_info[2]
       << "/" << _page_info[3]
       << "]" << endl;
#endif
  ((Memory9 *)memory)->set_page(page, slot);
}
