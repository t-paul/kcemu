/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: cb.h,v 1.7 2002/10/31 01:46:33 torsten_paul Exp $
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

#ifndef __kc_cb_h
#define __kc_cb_h

#include <string.h>
#include <stdlib.h>
#include <iostream.h>

#include "kc/kc.h"

class Callback
{
private:
  char *_name;
  
public:
  Callback(const char *name = 0)
    {
      if (name != 0)
	_name = strdup(name);
      else
	_name = strdup("unknown");
    }

  virtual ~Callback(void)
    {
      free(_name);
    }
  
  virtual void callback(void *data) = 0;

  virtual const char * get_name(void)
    {
      return _name;
    }
};

#endif /* __kc_cb_h */
