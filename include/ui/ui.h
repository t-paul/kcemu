/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: ui.h,v 1.15 2002/06/09 14:24:32 torsten_paul Exp $
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

#ifndef __ui_ui_h
#define __ui_ui_h

#include "kc/ic.h"
#include "kc/cb.h"
#include "kc/profile.h"

#include "ui/tape.h"
#include "ui/debug.h"
#include "ui/module.h"

#include "cmd/cmdargs.h"

#define WINDOW_NAME ("KC 85/4 Emulator")
#define ICON_NAME   ("KC 85/4")

class UI : public InterfaceCircuit, public Callback, public CMD_Caller
{
  public:
    UI(void) : InterfaceCircuit("UI"), Callback("UI") {}
    virtual ~UI(void) {}
    virtual void processEvents(void)              = 0;
    virtual void flash(bool enable)               = 0;
    virtual void memWrite(int addr, char val)     = 0;
    virtual void update(bool full_update = false, bool clear_cache = false) = 0;

    virtual void init(int *argc, char ***argv)    = 0;
    virtual void callback(void *data)             = 0;

    /*
     *  interface handling
     */
    virtual UI_ModuleInterface * getModuleInterface(void) = 0;
    virtual TapeInterface  * getTapeInterface(void)  = 0;
    virtual DebugInterface * getDebugInterface(void) = 0;

    /*
     *  InterfaceCircuit
     */
    virtual void reti(void) {}
    virtual void irqreq(void) {}
    virtual word_t irqack() { return IRQ_NOT_ACK; }
    virtual void reset(bool power_on = false) {}
};

#endif /* __ui_ui_h */
