/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: timer.h,v 1.5 2001/04/14 15:14:57 tp Exp $
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

#ifndef __kc_timer_h
#define __kc_timer_h

#include "kc/cb.h"
#include "kc/kc.h"

class Timer : public Callback
{
private:
  enum {
    NR_OF_TIMERS  = 1,
    TIMER_VALUE_0 = 35000, /* 50 Hz */
  };
  
public:
  Timer(void);
  void start(void);
  void callback(void *data);
};

#endif /* __kc_timer_h */
