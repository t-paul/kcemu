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

#ifndef __kc_timer1_h
#define __kc_timer1_h

#include "kc/kc.h"
#include "kc/timer.h"

class Timer1 : public Timer
{
 private:
  byte_t _flash;

 public:
  Timer1(void);
  virtual ~Timer1(void);

  virtual byte_t get_flash();

  virtual void start(void);
  virtual void callback(void *data);
};

#endif /* __kc_timer1_h */
