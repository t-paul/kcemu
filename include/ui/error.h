/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: error.h,v 1.3 2001/04/14 15:15:06 tp Exp $
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

#ifndef __ui_error_h
#define __ui_error_h

#include <list>

#include "ui/errorl.h"

class Error;

class Error
{
  typedef std::list<ErrorListener *> error_list_t;
  
 private:
  static Error *_instance;
  error_list_t _l;

 protected:
  Error(void);
  ~Error(void) {}
  
 public:
  static Error * instance(void);
  void info(const char *msg);
  void addErrorListener(ErrorListener *listener);
};

#endif /* __ui_error_h */
