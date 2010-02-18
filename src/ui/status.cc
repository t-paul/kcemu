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

#include "ui/status.h"

Status *Status::_instance = 0;

Status::Status(void)
{
}

Status *
Status::instance(void)
{
  if (_instance == 0)
    {
      _instance = new Status();
    }
  return _instance;
}

void
Status::addStatusListener(StatusListener *listener)
{
  _l.push_back(listener);
}

void
Status::setMessage(const char *msg)
{
  for (status_list_t::iterator it = _l.begin();it != _l.end();it++)
    (*it)->setStatus(msg);
}
