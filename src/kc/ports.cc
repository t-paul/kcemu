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

#include <string.h>

#include <iostream>
#include <iomanip>

#include "kc/system.h"

#include "kc/z80.h"
#include "kc/ports.h"

#include "libdbg/dbg.h"

using namespace std;

NullPort::NullPort(const char *name)
{
  _name = strdup(name);
}

NullPort::~NullPort(void)
{
  free(_name);
}

byte_t
NullPort::in(word_t addr)
{
  byte_t val = 0xff;

  DBG(2, form("KCemu/Ports/NullPort/in",
	      "%s: in() addr = %04x (returning %02x)\n",
	      _name, addr, val));

  return val;
}

void
NullPort::out(word_t addr, byte_t val)
{
  DBG(2, form("KCemu/Ports/NullPort/out",
	      "%s: out() addr = %04x, val = %02x\n",
	      _name, addr, val));
}

PortGroup::PortGroup(const char *name, PortInterface *p,
		     byte_t start, word_t len, int prio)
{
  _p = p;
  _start = start;
  _len = len;
  _prio = prio;
  _name = strdup(name);
  _active = true;
}

PortGroup::~PortGroup(void)
{
  free(_name);
}

Ports::Ports(void)
{
}

Ports::~Ports(void)
{
}

void
Ports::insert(port_list_t *l, PortGroup *group)
{
  int prio;
  port_list_t::iterator it;

  prio = group->get_prio();
  for (it = l->begin();it != l->end();it++)
    if ((*it)->get_prio() > prio) break;
  l->insert(it, group);
}

PortGroup *
Ports::register_ports(const char *name,
		      byte_t start, word_t len,
		      PortInterface *p, int prio)
{
  int a;
  PortGroup *portg = new PortGroup(name, p, start, len, prio);

  for (a = start;a < (len + start);a++)
    insert(&_port_list[a], portg);
  reload_prt_ptr();

  return portg;
}

void
Ports::unregister_ports(PortGroup *p)
{
  int a;

  for (a = p->get_start();a < (p->get_len() + p->get_start());a++)
    _port_list[a].remove(p);
  reload_prt_ptr();
}

void
Ports::reload_prt_ptr(void)
{
  port_list_t::iterator it;

  for (int a = 0;a < NR_PORTS;a++)
    {
      for (it = _port_list[a].begin();it != _port_list[a].end();it++)
	if ((*it)->is_active())
	  {
	    _port_ptr[a] = (*it)->get_port_if();
	    break;
	  }
    }
}

byte_t
Ports::in(word_t addr)
{
  return _port_ptr[addr & 0xff]->in(addr);
}

void
Ports::out(word_t addr, byte_t val)
{
  _port_ptr[addr & 0xff]->out(addr, val);
}

void
Ports::info(void)
{
  int a;
  port_list_t::iterator it;

  cerr << "  Ports:" << endl;
  cerr << "  ------" << endl << endl;
  for (a = 0;a < NR_PORTS;a++)
    {
      if (_port_list[a].size() > 1)
	{
	  cerr << "  " << hex << setw(2) << setfill('0') << a << "h:";
	  /*
           *  display registered ports but don't list the fallback
           *  entry that comes as the last entry
           */
	  for (it = _port_list[a].begin();it != --(_port_list[a].end());it++)
	    cerr << " [\"" << (*it)->get_name() << "\" "
		 << ((*it)->is_active() ? 'A' : 'a')
		 << " ]";

	  cout << endl;
	}
    }
}
