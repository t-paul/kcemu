/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-1998 Torsten Paul
 *
 *  $Id: debug.h,v 1.1 1998/10/28 23:52:24 tp Exp $
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

#ifndef __ui_debug_h
#define __ui_debug_h

class DebugInterface
{
 public:
  /**
   *  called before each instruction when
   *  tracing is enabled
   */
  virtual void debugTrace(void) = 0;
  /**
   *  called before each instruction when
   *  singlestepping is enabled
   */
  virtual void debugSingleStep(void) = 0;
};

#endif /* __ui_debug_h */
