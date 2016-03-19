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

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <linux/joystick.h>

#include "kc/system.h"

#include "kc/z80.h"
#include "kc/daisy.h"
#include "kc/mod_js.h"
#include "kc/prefs/prefs.h"

#include "ui/status.h"

#include "libdbg/dbg.h"

using namespace std;

PIOJoystick::PIOJoystick(void) : Callback("PIOJoystick")
{
  _device = NULL;
  _is_open = false;

  js_open();

  z80->addCallback(CALLBACK_OFFSET, this, NULL);
  register_callback_A_in(this);
}

PIOJoystick::~PIOJoystick(void)
{
  z80->remove_callback_listener(this);
  js_close();
}

const char *
PIOJoystick::check_joystick_device(void)
{
  const char *dev = Preferences::instance()->get_string_value("joystick_device", NULL);
  if (dev != NULL)
    return dev;

  const char *devices[] = {
    "/dev/input/js0",
    "/dev/input/js1",
    "/dev/input/js2",
    "/dev/input/js3",
    "/dev/js0",
    "/dev/js1",
    "/dev/js2",
    "/dev/js3",
    NULL,
  };

  for (int a = 0;devices[a] != NULL;a++)
    {
      if (access(devices[a], R_OK) == 0)
	return devices[a];
    }

  return NULL;
}

const char *
PIOJoystick::get_joystick_device(void)
{
  return _device;
}

void
PIOJoystick::set_joystick_device(const char *device)
{
  if (_device != NULL)
    free(_device);

  _device = (device == NULL) ? NULL : strdup(device);
}

bool
PIOJoystick::js_open(void)
{
  int version;
  char name[1024];

  _number_of_axes = 0;
  _number_of_buttons = 0;

  _idle_counter = IDLE_COUNTER_INIT;
  _up = _down = _left = _right = _button0 = _button1 = 0;

  if (get_joystick_device() == NULL)
    {
      const char *dev = check_joystick_device();
      if (dev == NULL)
	return false;

      set_joystick_device(dev);
    }

  bool ok = true;
  _fd = open(get_joystick_device(), O_RDONLY | O_NONBLOCK);
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
	  set_joystick_device(NULL);
	  _is_open = false;
	}
      return false;
    }

  if (!_is_open)
    {
      /*
       * Try to grab the joystick device. This would ensure that
       * the events go exclusively to KCemu. If the grab fails,
       * we still use the device but this might mean that the
       * system gives the events to other programs too (like the
       * X-Server).
       */
      int grab = 1;
      ioctl(_fd, EVIOCGRAB, &grab);

      char buf[1024];
      snprintf(buf, sizeof(buf),
	       _("Joystick (%d.%d.%d): %s on %s"),
	       0xff & (version >> 16),
	       0xff & (version >>  8),
	       0xff &  version,
	       name,
	       get_joystick_device());
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

bool
PIOJoystick::is_open(void)
{
  return _is_open;
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
  z80->addCallback(CALLBACK_OFFSET, this, NULL);

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
   *  USB system) we pick it up and use it againx.
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
	      if (event.value < -AXIS_THRESHOLD)
		{
		  _left = 1;
		  _right = 0;
		}
	      else if (event.value > AXIS_THRESHOLD)
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
	      if (event.value < -AXIS_THRESHOLD)
		{
		  _up = 1;
		  _down = 0;
		}
	      else if (event.value > AXIS_THRESHOLD)
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

void
PIOJoystick::reset(bool power_on)
{
  PIO::reset(power_on);
  z80->addCallback(CALLBACK_OFFSET, this, NULL);
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
    default:
      assert(0);
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

  _portg = NULL;
  if (_pio->is_open())
    {
      daisy->add_last(_pio);
      _portg = ports->register_ports("JOYSTICK", 0x90, 4, _pio, 0);
      set_valid(true);
    }
  else
    {
      char buf[1024];

      if (_pio->get_joystick_device() == NULL)
	{
	  snprintf(buf, sizeof(buf),
		   _("Couldn't find any readable joystick device!\n\n"
		     "If your joystick is connected properly and the correct\n"
		     "driver is loaded give the device name of your joystick\n"
		     "in the 'Joystick Device' section of the configuration\n"
		     "file."));
	}
      else
	{
	  snprintf(buf, sizeof(buf),
		   _("Couldn't open joystick device (%s)!\n\n"
		     "Make sure you have the joystick properly connected\n"
		     "and loaded the correct driver for it.\n\n"
		     "Also check the 'Joystick Device' section in the\n"
		     "configuration file if the device name shown above\n"
		     "looks bogus.\n"),
		   _pio->get_joystick_device());
	}

      set_error_text(buf);
      set_valid(false);
    }
}

ModuleJoystick::ModuleJoystick(const char *name, byte_t id) :
  ModuleInterface(name, id, KC_MODULE_KC_85_3)
{
  _pio = NULL;
  _portg = NULL;
  set_valid(true);
}

ModuleJoystick::~ModuleJoystick(void)
{
  if (_portg != NULL)
    {
      daisy->remove(_pio);
      ports->unregister_ports(_portg);
    }

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
