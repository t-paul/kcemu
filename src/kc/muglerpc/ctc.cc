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

#include <iostream>
#include <iomanip>

#include "kc/system.h"

#include "kc/z80.h"

#include "kc/muglerpc/ctc.h"

#include "libdbg/dbg.h"

CTCMuglerPCSystem::CTCMuglerPCSystem(void)
{
}

CTCMuglerPCSystem::~CTCMuglerPCSystem(void)
{
}

byte_t
CTCMuglerPCSystem::in(word_t addr)
{
  DBG(2, form("KCemu/CTC/muglerpc_system/in",
              "CTCMuglerPCSystem::in(): addr = %04x\n",
              addr));

  switch (addr & 3)
    {
    case 0:
      return c_in(0);
    case 1:
      return c_in(1);
    case 2:
      return c_in(2);
    case 3:
      return c_in(3);
    }

  return 0; // shouldn't be reached
}

void
CTCMuglerPCSystem::out(word_t addr, byte_t val)
{
  DBG(2, form("KCemu/CTC/muglerpc_system/out",
              "CTCMuglerPCSystem::out(): addr = %04x, val = %02x\n",
              addr, val));

  switch (addr & 3)
    {
    case 0:
      c_out(0, val);
      break;
    case 1:
      c_out(1, val);
      break;
    case 2:
      c_out(2, val);
      break;
    case 3:
      c_out(3, val);
      break;
    }
}

bool
CTCMuglerPCSystem::irq_0(void)
{
  return false;
}

bool
CTCMuglerPCSystem::irq_1(void)
{
  return false;
}

bool
CTCMuglerPCSystem::irq_2(void)
{
  return false;
}

bool
CTCMuglerPCSystem::irq_3(void)
{
  return false;
}

long
CTCMuglerPCSystem::counter_value_0(void)
{
  return 0;
}

long
CTCMuglerPCSystem::counter_value_1(void)
{
  return 0;
}

long
CTCMuglerPCSystem::counter_value_2(void)
{
  return 0;
}

long
CTCMuglerPCSystem::counter_value_3(void)
{
  return 0;
}

CTCMuglerPCUser::CTCMuglerPCUser(void)
{
}

CTCMuglerPCUser::~CTCMuglerPCUser(void)
{
}

byte_t
CTCMuglerPCUser::in(word_t addr)
{
  DBG(2, form("KCemu/CTC/muglerpc_user/in",
              "CTCMuglerPCUser::in(): addr = %04x\n",
              addr));

  switch (addr & 3)
    {
    case 0:
      return c_in(0);
    case 1:
      return c_in(1);
    case 2:
      return c_in(2);
    case 3:
      return c_in(3);
    }

  return 0; // shouldn't be reached
}

void
CTCMuglerPCUser::out(word_t addr, byte_t val)
{
  DBG(2, form("KCemu/CTC/muglerpc_user/out",
              "CTCMuglerPCUser::out(): addr = %04x, val = %02x\n",
              addr, val));

  switch (addr & 3)
    {
    case 0:
      c_out(0, val);
      break;
    case 1:
      c_out(1, val);
      break;
    case 2:
      c_out(2, val);
      break;
    case 3:
      c_out(3, val);
      break;
    }
}

bool
CTCMuglerPCUser::irq_0(void)
{
  return false;
}

bool
CTCMuglerPCUser::irq_1(void)
{
  return false;
}

bool
CTCMuglerPCUser::irq_2(void)
{
  return false;
}

bool
CTCMuglerPCUser::irq_3(void)
{
  return false;
}

long
CTCMuglerPCUser::counter_value_0(void)
{
  return 0;
}

long
CTCMuglerPCUser::counter_value_1(void)
{
  return 0;
}

long
CTCMuglerPCUser::counter_value_2(void)
{
  return 0;
}

long
CTCMuglerPCUser::counter_value_3(void)
{
  return 0;
}
