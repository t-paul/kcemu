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

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <linux/joystick.h>

#include "kc/system.h"

#include "kc/z80.h"
#include "kc/mod_js.h"

#include "ui/status.h"

#include "libdbg/dbg.h"

using namespace std;

PIOJoystick::PIOJoystick(void) : Callback("PIOJoystick")
{
  _is_open = false;

  js_open();
  z80->addCallback(35000, this, NULL);
  register_callback_A_in(this);
}

PIOJoystick::~PIOJoystick(void)
{
  z80->remove_callback_listener(this);
  js_close();
}

bool
PIOJoystick::js_open(void)
{
  bool ok;
  int version;
  char name[250];
  char buf[1024];

  _number_of_axes = 0;
  _number_of_buttons = 0;

  _idle_counter = IDLE_COUNTER_INIT;
  _up = _down = _left = _right = _button0 = _button1 = 0;

  ok = true;
  _fd = open("/dev/js0", O_RDONLY | O_NONBLOCK);
  if (_fd < 0)
    ok = false;

  if (ok)
    ok = (ioctl(_fd, JSIOCGVERSION, &version) >= 0);

  if (ok)
    ok = (ioctl(_fd, JSIOCGNAME(sizeof(name)), name) >= 0);

  if (ok)
    ok = (ioctl(_fd, JSIOCGAXES, &_number_of_axes) >= 0);

  if (ok)
    ok = (ioctl(_fd, JSIOCGBUTTONS, &_number_of_buttons) >= 0);

  if (_number_of_axes < 2)
    ok = false;

  if (_number_of_buttons < 1)
    ok = false;

  if (!ok)
    {
      js_close();
      if (_is_open)
	{
	  Status::instance()->setMessage(_("Joystick removed!"));
	  _is_open = false;
	}
      return false;
    }

  if (!_is_open)
    {
      snprintf(buf, 1024,
	       _("Joystick (%d.%d.%d): %s"),
	       0xff & (version >> 16),
	       0xff & (version >>  8),
	       0xff &  version,
	       name);
      Status::instance()->setMessage(buf);
      _is_open = true;
    }

  return true;
}

void
PIOJoystick::js_close(void)
{
  if (_fd >= 0)
    close(_fd);

  _fd = -1;
}

void
PIOJoystick::change_A(byte_t changed, byte_t val)
{
}

void
PIOJoystick::change_B(byte_t changed, byte_t val)
{
}

void
PIOJoystick::callback(void *data)
{
  struct js_event event;

  strobe_A();
  strobe_B();
  z80->addCallback(35000, this, NULL);

  /*
   *  reopening the joystick driver after some idle time will
   *  ensure that the hotplug system will get the chance to unload
   *  the driver for a disconnected joystick otherwise we would
   *  simply block the device
   */
  if (--_idle_counter < 0)
    {
      js_close();
      js_open();
    }

  /*
   *  If initialization of the joystick driver failes we simply
   *  ignore the fact and keep trying so if a joystick is plugged
   *  in and the driver is loaded (e.g. autoloaded by the hotplug
   *  USB system) we pick it up and use it.
   */
  while (_fd >= 0)
    {
      errno = 0;
      if (read(_fd, &event, sizeof(struct js_event)) < 0)
	{
	  if (errno != EAGAIN)
	    js_close();
	  break;
	}

      _idle_counter = IDLE_COUNTER_INIT;

      switch (event.type & ~JS_EVENT_INIT)
	{
	case JS_EVENT_BUTTON:
	  if (event.number == 0)
	    _button0 = event.value;
	  else
	    _button1 = event.value;
	  break;
	case JS_EVENT_AXIS:
	  if (event.number == 0)
	    {
	      if (event.value < -10000)
		{
		  _left = 1;
		  _right = 0;
		}
	      else if (event.value > 10000)
		{
		  _left = 0;
		  _right = 1;
		}
	      else
		{
		  _left = 0;
		  _right = 0;
		}
	    }
	  else if (event.number == 1)
	    {
	      if (event.value < -10000)
		{
		  _up = 1;
		  _down = 0;
		}
	      else if (event.value > 10000)
		{
		  _up = 0;
		  _down = 1;
		}
	      else
		{
		  _up = 0;
		  _down = 0;
		}
	    }
	  break;
	}
    }

  _val = 0xff;
  if (_up)
    _val ^= 1;
  if (_down)
    _val ^= 2;
  if (_left)
    _val ^= 4;
  if (_right)
    _val ^= 8;
  if (_button0)
    _val ^= 16;
  if (_button1)
    _val ^= 32;

  set_A_EXT(0xff, _val);
}

byte_t
PIOJoystick::in(word_t addr)
{
  byte_t val;

  switch (addr & 3)
    {
    case 0:
      val = in_A_DATA();
      break;
    case 1:
      val = in_B_DATA();
      break;
    case 2:
      val = in_A_CTRL();
      break;
    case 3:
      val = in_B_CTRL();
      break;
    }
  
  DBG(2, form("KCemu/PIO/joystick/in",
              "PIOJoystick::in():  %04xh: addr = %04x, val = %02x\n",
              z80->getPC(), addr, val));

  return val;
}

void
PIOJoystick::out(word_t addr, byte_t val)
{
  DBG(2, form("KCemu/PIO/joystick/out",
              "PIOJoystick::out(): %04xh: addr = %04x, val = %02x\n",
              z80->getPC(), addr, val));

  switch (addr & 3)
    {
    case 0:
      out_A_DATA(val);
      break;
    case 1:
      out_B_DATA(val);
      break;
    case 2:
      out_A_CTRL(val);
      break;
    case 3:
      out_B_CTRL(val);
      break;
    }
}

int
PIOJoystick::callback_A_in(void)
{
  return _val;
}

int
PIOJoystick::callback_B_in(void)
{
  return -1;
}

void
PIOJoystick::callback_A_out(byte_t val)
{
}

void
PIOJoystick::callback_B_out(byte_t val)
{
}

ModuleJoystick::ModuleJoystick(ModuleJoystick &tmpl) :
  ModuleInterface(tmpl.get_name(), tmpl.get_id(), tmpl.get_type())
{
  _pio = new PIOJoystick();
  _pio->next(pio->get_first());
  _pio->iei(1);
  _portg = ports->register_ports("JOYSTICK", 0x90, 4, _pio, 0);

  set_valid(true);
}

ModuleJoystick::ModuleJoystick(const char *name, byte_t id) :
  ModuleInterface(name, id, KC_MODULE_KC_85_3)
{
  _pio = NULL;
  _portg = NULL;
}

ModuleJoystick::~ModuleJoystick(void)
{
  if (_portg != NULL)
    ports->unregister_ports(_portg);

  if (_pio != NULL)
    delete _pio;
}

byte_t
ModuleJoystick::in(word_t addr)
{
  byte_t val = 0xff;
  return val;
}

void
ModuleJoystick::out(word_t addr, byte_t val)
{
}

ModuleInterface *
ModuleJoystick::clone(void)
{
  return new ModuleJoystick(*this);
}

void
ModuleJoystick::reset(bool power_on)
{
}

void
ModuleJoystick::m_out(word_t addr, byte_t val)
{
}
