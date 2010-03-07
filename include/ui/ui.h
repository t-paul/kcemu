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
    virtual int  get_mode(void)                   = 0;
    virtual void set_mode(int mode)               = 0;
    virtual void flash(bool enable)               = 0;
    virtual void memory_read(word_t addr)         = 0;
    virtual void memory_write(word_t addr)        = 0;
    virtual void update(bool full_update = false, bool clear_cache = false) = 0;

    virtual void init(int *argc, char ***argv)    = 0;
    virtual void init2(void)                      = 0;
    virtual char * select_profile(void)           = 0;
    virtual void show(void)                       = 0;
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
