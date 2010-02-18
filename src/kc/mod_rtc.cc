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

#include <time.h>
#include <stdio.h>

#include <iostream>
#include <iomanip>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/mod_rtc.h"

#include "sys/sysdep.h"

ModuleRTC::ModuleRTC(ModuleRTC &tmpl) :
  ModuleInterface(tmpl.get_name(), tmpl.get_id(), tmpl.get_type())
{
  _portg = NULL;
  _master = &tmpl;

  if (_master->get_count() == 0)
    {
      _master->set_count(1);
      _base_port = _master->_base_port;
      _portg = ports->register_ports("RTC", _base_port, 16, this, 0);
      set_valid(true);
    }
  else
    {
      char buf[1024];
      snprintf(buf, sizeof(buf),
	       _("It's not possible to have more than one\n"
		 "module of type %s!"),
	       get_name());
      set_error_text(buf);
      set_valid(false);
    }
}

ModuleRTC::ModuleRTC(const char *name, int base_port) :
  ModuleInterface(name, 0, KC_MODULE_Z1013)
{
  _count = 0;
  _portg = NULL;
  _base_port = base_port;

  set_valid(true);
}

ModuleRTC::~ModuleRTC(void)
{
  if (_portg)
    {
      _master->set_count(0);
      ports->unregister_ports(_portg);
    }
}

void
ModuleRTC::m_out(word_t addr, byte_t val)
{
}

ModuleInterface *
ModuleRTC::clone(void)
{
  return new ModuleRTC(*this);
}

void
ModuleRTC::reset(bool power_on)
{
}

int
ModuleRTC::get_count(void)
{
  return _count;
}

void
ModuleRTC::set_count(int count)
{
  _count = count;
}

byte_t
ModuleRTC::in(word_t addr)
{
  int year, month, day, hour, minute, second;

  sys_localtime(&year, &month, &day, &hour, &minute, &second);

  switch (addr & 0x0f)
    {
    case 0x00:
      return second % 10;
    case 0x01:
      return second / 10;
    case 0x02:
      return minute % 10;
    case 0x03:
      return minute / 10;
    case 0x04:
      return hour % 10;
    case 0x05:
      return hour / 10;
    case 0x06:
      return day % 10;
    case 0x07:
      return day / 10;
    case 0x08:
      return month % 10;
    case 0x09:
      return month / 10;
    case 0x0a:
      return year % 10;
    case 0x0b:
      return (year / 10) % 10;
    case 0x0c: // ???
      return 0x00;
    case 0x0d: // control D
      return 0x00;
    case 0x0e: // control E
      return 0x00;
    case 0x0f: // control F
      return 0x00;
    }

  return 0xff;
}

void
ModuleRTC::out(word_t addr, byte_t val)
{
}
