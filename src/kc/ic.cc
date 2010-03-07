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

#include "kc/ic.h"

#include "libdbg/dbg.h"

InterfaceCircuit::InterfaceCircuit(const char *name)
{
  _next      = 0;
  _prev      = 0;
  _name      = strdup(name);
  reset();
}

InterfaceCircuit::~InterfaceCircuit(void)
{
  free(_name);
}

const char * const
InterfaceCircuit::get_ic_name(void)
{
  return _name;
}

void
InterfaceCircuit::debug(void)
{
  DBG(2, form("KCemu/IC/irq",
              "IC::debug():   %-8s: iei = %d - ieo = %d - irqreq = %d - irqactive = %d\n",
              _name, _iei, ieo(), _irqreq, _irqactive));

  if (_next)
    _next->debug();
}

void
InterfaceCircuit::irq(void)
{
  DBG(2, form("KCemu/IC/irq",
              "IC::irq():     %-8s: iei = %d - ieo = %d - irqreq = %d - irqactive = %d\n",
              _name, _iei, ieo(), _irqreq, _irqactive));
  if (_irqactive || _irqreq)
    return;

  _irqreq = 1;
  if (_next)
    _next->iei(ieo());
  
  if ((_iei == 1) && (ieo() == 0))
    {
      DBG(2, form("KCemu/IC/irq",
		  "IC::irq():     %-8s: will call irqreq()\n",
		  get_ic_name()));
      irqreq();
    }
}

word_t
InterfaceCircuit::ack(void)
{
  DBG(2, form("KCemu/IC/ack",
              "IC::ack():     %-8s: iei = %d - ieo = %d - irqreq = %d - irqactive = %d\n",
              _name, _iei, ieo(), _irqreq, _irqactive));
  if ((_iei == 1) && (ieo() == 0))
    {
      DBG(2, form("KCemu/IC/ack",
		  "IC::ack():     %-8s: will call irqack()\n",
		  get_ic_name()));
      _irqreq = 0;
      _irqactive = 1;
      return irqack();
    }
  if (_prev)
    return _prev->ack();
  return IRQ_NOT_ACK;
}

void
InterfaceCircuit::iei(byte_t val)
{
  _iei = val;
  if (_next)
    _next->iei(ieo());
}

byte_t
InterfaceCircuit::ieo(void)
{
  /*
   *  temporary state between reti_ED() and reti_4D()
   */
  if (_ieo_reti)
    return 1;
  
  return _iei & (!(_irqreq || _irqactive));
}

void
InterfaceCircuit::prev(InterfaceCircuit *ic)
{
  _prev = ic;
}
  
void
InterfaceCircuit::next(InterfaceCircuit *ic)
{
  _next = ic;
  if (ic != NULL)
    {
      ic->prev(this);
      ic->iei(ieo());
    }
}

InterfaceCircuit *
InterfaceCircuit::get_prev()
{
  return _prev;
}

InterfaceCircuit *
InterfaceCircuit::get_next()
{
  return _next;
}

InterfaceCircuit *
InterfaceCircuit::get_first()
{
  if (_prev)
    return _prev->get_first();
  return this;
}

InterfaceCircuit *
InterfaceCircuit::get_last()
{
  if (_next)
    return _next->get_last();
  return this;
}

void
InterfaceCircuit::reti_ED(void)
{
  DBG(2, form("KCemu/IC/reti",
              "IC::reti_ED(): %-8s: iei = %d - ieo = %d - irqreq = %d - irqactive = %d\n",
              _name, _iei, ieo(), _irqreq, _irqactive));
  if ((_iei == 1) && (ieo() == 0))
    if (!_irqactive)
      {
	/*
	 *  enter temporary ieo state
	 */
	_ieo_reti = 1;
	if (_next)
	  _next->iei(1);
      }
  if (_prev)
    _prev->reti_ED();
}

void
InterfaceCircuit::reti_4D(void)
{
  DBG(2, form("KCemu/IC/reti",
              "IC::reti_4D(): %-8s: iei = %d - ieo = %d - irqreq = %d - irqactive = %d\n",
              _name, _iei, ieo(), _irqreq, _irqactive));
  if ((_iei == 1) && (ieo() == 0) && _irqactive)
    {
      DBG(2, form("KCemu/IC/reti",
		  "IC::reti_4D(): %-8s:\n",
		  get_ic_name()));
      _irqreq = 0;
      _irqactive = 0;
      reti();
      if (_next)
	_next->iei(ieo());
    }
  if (_prev)
    _prev->reti_4D();
  
  /*
   *  exit temporary ieo state
   */
  if (_ieo_reti)
    {
      _ieo_reti = 0;
      if (_next)
	_next->iei(ieo());
    }

  if (_irqreq != 0)
    {
      DBG(2, form("KCemu/IC/reti",
		  "IC::reti_4D(): %-8s: irqreq is set!\n",
		  get_ic_name()));

      if (_next)
	_next->iei(ieo());
      
      if ((_iei == 1) && (ieo() == 0))
	{
	  DBG(2, form("KCemu/IC/reti",
		      "IC::reti_4D(): %-8s: will call irqreq()\n",
		      get_ic_name()));
	  irqreq();
	}
    }
}

void
InterfaceCircuit::reset(bool power_on)
{
  DBG(2, form("KCemu/IC/reset",
              "IC::reset():   %-8s\n",
              _name));

  _iei       = 0;
  _ieo_reti  = 0;
  _irqreq    = 0;
  _irqactive = 0;
}
