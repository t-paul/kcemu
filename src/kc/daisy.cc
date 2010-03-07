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

#include "kc/system.h"

#include "kc/daisy.h"

#include "libdbg/dbg.h"

DaisyChain::DaisyChain(void)
{
}

DaisyChain::~DaisyChain(void)
{
}

void
DaisyChain::add_last(InterfaceCircuit *ic)
{
  if (_daisy_chain_list.empty())
    {
      ic->iei(1);
    }
  else
    {
      InterfaceCircuit *last = _daisy_chain_list.back();
      last->next(ic);
    }

  ic->next(0);
  _daisy_chain_list.push_back(ic);

  //printf("------- DaisyChain::add_last(): '%s'\n", ic->get_ic_name());
  //_daisy_chain_list.front()->debug();
}

void
DaisyChain::remove(InterfaceCircuit *ic)
{
  _daisy_chain_list.remove(ic);

  InterfaceCircuit *prev = ic->get_prev();
  InterfaceCircuit *next = ic->get_next();

  if (prev != NULL)
    prev->next(ic->get_next());
  else if (next != NULL)
    next->prev(0);

  //printf("------- DaisyChain::remove(): '%s'\n", ic->get_ic_name());
  //_daisy_chain_list.front()->debug();
}

word_t
DaisyChain::irq_ack(void)
{
  if (_daisy_chain_list.empty())
    return IRQ_NOT_ACK;

  //_daisy_chain_first->debug();
  word_t val = _daisy_chain_list.back()->ack();
  //_daisy_chain_first->debug();
  //sleep(1);
  return val;
}

void
DaisyChain::reti(void)
{
  if (_daisy_chain_list.empty())
    return;

  _daisy_chain_list.back()->reti_ED();
  _daisy_chain_list.back()->reti_4D();
}

