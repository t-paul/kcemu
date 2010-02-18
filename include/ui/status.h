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

#ifndef __ui_status_h
#define __ui_status_h

#include <list>

#include "ui/statusl.h"

class Status;

class Status
{
  typedef std::list<StatusListener *> status_list_t;
  
 private:
  static Status *_instance;
  status_list_t _l;

 protected:
  Status(void);
  ~Status(void) {}
  
 public:
  static Status * instance(void);
  void setMessage(const char *msg);
  void addStatusListener(StatusListener *listener);
};

#endif /* __ui_status_h */
