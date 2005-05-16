/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2003 Torsten Paul
 *
 *  $Id$
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

#ifndef __kc_mod_js_h
#define __kc_mod_js_h

#include "kc/cb.h"
#include "kc/pio.h"
#include "kc/ports.h"
#include "kc/module.h"

class PIOJoystick : public PIO, public PIOCallbackInterface, public Callback
{
 private:
  enum {
    IDLE_COUNTER_INIT = 500
  };

 private:
  int _fd;
  int _up;
  int _down;
  int _left;
  int _right;
  int _button0;
  int _button1;

  bool _is_open;
  int  _idle_counter;

  char _number_of_axes;
  char _number_of_buttons;

  byte_t _val;

  char *_device;

 protected:
  virtual bool js_open(void);
  virtual void js_close(void);
  virtual void change_A(byte_t changed, byte_t val);
  virtual void change_B(byte_t changed, byte_t val);

 public:
  PIOJoystick(void);
  virtual ~PIOJoystick(void);

  virtual bool is_open(void);
  virtual const char * check_joystick_device(void);
  virtual const char * get_joystick_device(void);
  virtual void set_joystick_device(const char *device);

  virtual byte_t in(word_t addr);
  virtual void out(word_t addr, byte_t val);

  /*
   *  Callback
   */
  virtual void callback(void *data);
  
  /*
   *  PIOCallbackInterface
   */
  virtual int callback_A_in(void);
  virtual int callback_B_in(void);
  virtual void callback_A_out(byte_t val);
  virtual void callback_B_out(byte_t val);
};

class ModuleJoystick : public ModuleInterface, public PortInterface
{
 private:
  PIOJoystick *_pio;
  PortGroup *_portg;

 public:
  ModuleJoystick(ModuleJoystick &tmpl);
  ModuleJoystick(const char *name, byte_t id);
  virtual ~ModuleJoystick(void);

  /*
   *  ModuleInterface
   */
  virtual void m_out(word_t addr, byte_t val);
  virtual ModuleInterface * clone(void);
  virtual void reset(bool power_on = false);

  /*
   *  PortInterface
   */
  virtual byte_t in(word_t addr);
  virtual void out(word_t addr, byte_t val);
};

#endif /* __kc_mod_js_h */
