/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-1998 Torsten Paul
 *
 *  $Id: log.cc,v 1.1 2000/05/21 17:01:08 tp Exp $
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

#include "ui/log.h"
#include "ui/status.h"

#include "libdbg/dbg.h"

LOG::LOG(void)
{
  Status::instance()->addStatusListener(this);
}

LOG::~LOG(void)
{
}

void
LOG::setStatus(const char *msg)
{
  DBG(0, form("KCemu/LOG",
              "LOG::STATUS | %s\n", msg));
}

void
LOG::errorInfo(const char *msg)
{
  DBG(0, form("KCemu/LOG",
              "LOG::ERROR  | %s\n", msg));
}

