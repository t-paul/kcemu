/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: keyboard.h,v 1.6 2001/04/14 15:14:21 tp Exp $
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

#ifndef __kc_keyboard_h
#define __kc_keyboard_h

#include "kc/kc.h"
#include "kc/cb.h"
#include "kc/ic.h"

class Keyboard : public InterfaceCircuit, public Callback
{
  public:
    Keyboard(void) : Callback("Keyboard") {}
    virtual ~Keyboard(void) {}
    
    /**
     *  keysym:  translated key (e.g. shift already included)
     *  keycode: raw key number (this should be exactly one number for
     *           every key on the keyboard so we can track key presses
     *           and releases even with changing modifiers between
     *           keypress and keyrelease)
     */
    virtual void keyPressed(int keysym, int keycode) = 0;
    virtual void keyReleased(int keysym, int keycode) = 0;
    virtual void replayString(const char *text) = 0;

    virtual void callback(void *data) = 0;
};

#endif /* __kc_keyboard_h */
