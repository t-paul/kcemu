/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: status.cc,v 1.3 2001/04/14 15:16:46 tp Exp $
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
