/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-1998 Torsten Paul
 *
 *  $Id: ports4.cc,v 1.7 2000/05/21 16:54:36 tp Exp $
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

#include <stdio.h>
#include <iostream.h>
#include <iomanip.h>

#include "kc/config.h"
#include "kc/system.h"

#include "kc/kc.h"
#include "kc/ctc.h"
#include "kc/pio.h"
#include "kc/module.h"
#include "kc/ports4.h"
#include "kc/memory4.h"

#include "libdbg/dbg.h"

#define memory ((Memory4 *)memory)

Ports4::Ports4(void)
{
    int a;
    
    for (a = 0;a < NR_PORTS;a++) {
	inout[a] = 0;
    }
    inout[0x88] = 0x0f;
}

Ports4::~Ports4(void)
{
}

byte_t
Ports4::in(word_t addr)
{
    int val;
    byte_t a = addr & 0xff;

    switch (a)
      {
      case 0x80: val = module->in(addr); break;
      case 0x88: val = pio->in_A_DATA(); break;
      case 0x89: val = pio->in_B_DATA(); break;
      case 0x8a: val = pio->in_A_CTRL(); break;
      case 0x8b: val = pio->in_B_CTRL(); break;
      case 0x8c: val = ctc->in(0);       break;
      case 0x8d: val = ctc->in(1);       break;
      case 0x8e: val = ctc->in(2);       break;
      case 0x8f: val = ctc->in(3);       break;
      default:   val = inout[a];         break;
      }

#if 0
    if ((a >= 0x08) && (a <= 0x0f))
      cout.form("Ports::in():  %04x -> %02x\n", addr, inout[a]);
#endif
    
    return val;
}

void
Ports4::out(word_t addr, byte_t val)
{
  byte_t a = addr & 0xff;

#if 0
  if ((a >= 0x08) && (a <= 0x0f))
    cout.form("Ports::out(): %04x <- %02x\n", addr, val);
#endif
  
  switch (a)
    {
    case 0x80: module->out(addr, val);              return;
    case 0x84: change_0x84(inout[0x84] ^ val, val); break;
    case 0x86: change_0x86(inout[0x86] ^ val, val); break;
    case 0x88: pio->out_A_DATA(val);                return;
    case 0x89: pio->out_B_DATA(val);                return;
    case 0x8a: pio->out_A_CTRL(val);                return;
    case 0x8b: pio->out_B_CTRL(val);                return;
    case 0x8c: ctc->out(0, val);                    return;
    case 0x8d: ctc->out(1, val);                    return;
    case 0x8e: ctc->out(2, val);                    return;
    case 0x8f: ctc->out(3, val);                    return;
    }
    inout[addr & 0xff] = val;
}

void
Ports4::change_0x84(byte_t changed, byte_t val)
{
  if (changed & 0x01)
    {
      DBG(2, form("KCemu/Ports/4/change/84",
                  "Ports 0x84: display screen %d\n",
                  val & 1));
    }
  if (changed & 0x02)
    {
      DBG(2, form("KCemu/Ports/4/change/84",
                  "Ports 0x84: pixel/ color %d\n",
                  (val >> 1) & 1));
      memory->enableCOLOR(val & 0x02);
    }
  if (changed & 0x04)
    {
      DBG(2, form("KCemu/Ports/4/change/84",
                  "Ports 0x84: access screen %d\n",
                  (val >> 2) & 1));
      memory->enableSCREEN_1(val & 0x04);
    }
  if (changed & 0x08)
    {
      DBG(2, form("KCemu/Ports/4/change/84",
                  "Ports 0x84: high resolution %d\n",
                  (val >> 3) & 1));
    }
  if (changed & 0x10)
    {
      DBG(2, form("KCemu/Ports/4/change/84",
                  "Ports 0x84: access RAM8 block %d\n",
                  (val >> 4) & 1));
      memory->enableRAM_8_1(val & 0x10);
    }
  if (changed & 0x20)
    {
      DBG(2, form("KCemu/Ports/4/change/84",
                  "Ports 0x84: RAM8 on/off %d\n",
                  (val >> 5) & 1));
      memory->enableRAM_8(val & 0x20);
    }
  if (changed & 0x40)
    {
      DBG(2, form("KCemu/Ports/4/change/84",
                  "Ports 0x84: reserved 0x40 %d\n",
                  (val >> 6) & 1));
    }
  if (changed & 0x80)
    {
      DBG(2, form("KCemu/Ports/4/change/84",
                  "Ports 0x84: reserved 0x80 %d\n",
                  (val >> 7) & 1));
    }
}

void
Ports4::change_0x86(byte_t changed, byte_t val)
{
  if (changed & 0x01)
    {
      DBG(2, form("KCemu/Ports/4/change/86",
                  "Ports 0x86: RAM 4 %d\n",
                  val & 1));
      memory->enableRAM_4(val & 0x01);
    }
  if (changed & 0x02)
    {
      DBG(2, form("KCemu/Ports/4/change/86",
                  "Ports 0x86: write protect RAM 4 %d\n",
                  (val >> 1) & 1));
      memory->protectRAM_4(val & 0x02);
    }
  if (changed & 0x04)
    {
      DBG(2, form("KCemu/Ports/4/change/86",
                  "Ports 0x86: unused 0x04 %d\n",
                  (val >> 2) & 1));
    }
  if (changed & 0x08)
    {
      DBG(2, form("KCemu/Ports/4/change/86",
                  "Ports 0x86: unused 0x08 %d\n",
                  (val >> 3) & 1));
    }
  if (changed & 0x10)
    {
      DBG(2, form("KCemu/Ports/4/change/86",
                  "Ports 0x86: unused 0x10 %d\n",
                  (val >> 4) & 1));
    }
  if (changed & 0x20)
    {
      DBG(2, form("KCemu/Ports/4/change/86",
                  "Ports 0x86: reserved 0x20 %d\n",
                  (val >> 5) & 1));
    }
  if (changed & 0x40)
    {
      DBG(2, form("KCemu/Ports/4/change/86",
                  "Ports 0x86: reserved 0x40 %d\n",
                  (val >> 6) & 1));
    }
  if (changed & 0x80)
    {
      DBG(2, form("KCemu/Ports/4/change/86",
                  "Ports 0x86: CAOS C %d\n",
                  (val >> 7) & 1));
      memory->enableCAOS_C(val & 0x80);
    }
}
