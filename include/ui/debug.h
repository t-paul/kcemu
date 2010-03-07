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

#ifndef __ui_debug_h
#define __ui_debug_h

class DebugInterface
{
 public:
  DebugInterface(void) {}
  virtual ~DebugInterface(void) {}

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
