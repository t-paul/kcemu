/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-1998 Torsten Paul
 *
 *  $Id: log.h,v 1.1 2000/05/21 17:39:11 tp Exp $
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

#ifndef __ui_log_h
#define __ui_log_h

#include "ui/errorl.h"
#include "ui/statusl.h"

class LOG : public StatusListener, public ErrorListener
{
 public:
  LOG(void);
  virtual ~LOG(void);
    
  /*
   *  StatusListener
   */
  void setStatus(const char *msg);

  /*
   *  ErrorListener
   */
  void errorInfo(const char *msg);
};

#endif /* __ui_log_h */
