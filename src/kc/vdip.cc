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

#include <time.h>
#include <dirent.h>
#include <unistd.h>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/pio.h"
#include "kc/z80.h"
#include "kc/vdip.h"
#include "kc/prefs/prefs.h"

#include "cmd/cmd.h"

#include "libdbg/dbg.h"

class CMD_vdip_attach : public CMD
{
private:
  VDIP *_vdip;
  static const char * _path;

protected:
  long get_slot_no(CMD_Args *args)
  {
    long n = 0;

    if (args && args->has_arg("slot"))
      n = args->get_long_arg("slot");

    return n;
  }

public:
  CMD_vdip_attach(VDIP *vdip) : CMD("vdip-attach")
  {
    _vdip = vdip;
    register_cmd("vdip-attach", 0);
    register_cmd("vdip-detach", 2);
  }

  void execute(CMD_Args *args, CMD_Context context)
    {
      const char *filename;

      filename = NULL;
      switch (context)
        {
          /*
           *  vdip-attach
           */
        case 0:
          if (!args)
            args = new CMD_Args();
          filename = args->get_string_arg("filename");
          if (!filename)
            {
              args->set_long_arg("ui-file-select-dir-only", 1);
              args->set_string_arg("ui-file-select-title",
                                   _("Select root directory..."));
	      if (_path)
		args->set_string_arg("ui-file-select-path", _path);
              args->add_callback("ui-file-select-CB-ok", this, 1);
              CMD_EXEC_ARGS("ui-file-select", args);
              return;
            }
          break;
          /*
           *  ui-file-select-CB-ok
           */
        case 1:
          if (args)
            filename = args->get_string_arg("filename");
          break;
          /*
           *  vdip-detach
           */
        case 2:
          if (get_slot_no(args) == 0)
            _vdip->set_root("");
          return;
        }

      if (filename)
        _vdip->set_root(filename);
    }
};

const char * CMD_vdip_attach::_path = NULL;

VDIP::VDIP(void) : Callback("Vinculum USB")
{
  const char *vdip_root_1 = kcemu_vdip_root;
  if (vdip_root_1 == NULL)
    vdip_root_1 = Preferences::instance()->get_string_value("vdip_root_1", NULL);
  if (vdip_root_1 == NULL)
    vdip_root_1 = "";

  _pio = NULL;
  _file = NULL;

  _root = vdip_root_1;

  _cmd = NULL;
  _cwd = new StringList();
  _attach_cmd = new CMD_vdip_attach(this);
  reset();
}

VDIP::~VDIP(void)
{
}

string
VDIP::get_firmware_version(void) const
{
  return "03.66VDAPF";
}

bool
VDIP::is_short_command_set(void) const
{
  return _short_command_set;
}

void
VDIP::set_short_command_set(bool val)
{
  _short_command_set = val;
}

bool
VDIP::is_binary_mode(void) const
{
  return _binary_mode;
}

void
VDIP::set_binary_mode(bool val)
{
  _binary_mode = val;
}

FILE *
VDIP::get_file(void) const
{
  return _file;
}

void
VDIP::set_file(FILE *file)
{
  _file = file;
}

void
VDIP::register_pio(PIO *pio)
{
  _pio = pio;
  reset();
}

string
VDIP::get_root(void) const
{
  return _root;
}

void
VDIP::set_root(string root)
{
  bool had_disk = has_disk();

  _root = root;

  if (_pio)
    {
      if (had_disk != has_disk())
        {
          if (has_disk())
            _output_buffer = is_short_command_set() ? "DD2\r" : "Device Detected P2\r";
          else
            _output_buffer = is_short_command_set() ? "DR2\r" : "Device Removed P2\r";

          set_pio_ext_b(0x02);
          z80->addCallback(20000, this, NULL);
        }
    }

  CMD_Args *args = new CMD_Args();
  args->set_long_arg("slot", 0);
  args->set_string_arg("filename", _root.c_str());
  CMD_EXEC_ARGS("ui-vdip-update-MSG", args);
  delete args;
}

bool
VDIP::has_disk(void)
{
  return !_root.empty();
}

bool
VDIP::is_root(void)
{
  return _cwd->size() == 0;
}

string
VDIP::get_cwd(void) const
{
  string cwd = get_root() + "/" + _cwd->join('/');
  return cwd;
}

string
VDIP::get_path(string dir) const
{
  string path = get_cwd() + "/" + dir;
  return path;
}

void
VDIP::chdir_up(void)
{
  if (_cwd->size() > 0)
    _cwd->pop_back();
}

void
VDIP::chdir_root(void)
{
  _cwd->clear();
}

void
VDIP::chdir(string dir)
{
  _cwd->push_back(dir);
}

void
VDIP::set_pio_ext_b(byte_t val)
{
  _pio_ext = val;
  _pio->set_B_EXT(0x03, _pio_ext);
}

void
VDIP::callback(void *data)
{
  _pio->strobe_B();
}

void
VDIP::reset(void)
{
  _reset = false;
  _binary_mode = true;
  _short_command_set = false;

  _input_data = -1;
  _input_buffer = "";

  _input = false;
  _output = -1;
  _output_buffer = "\rVer " + get_firmware_version() + " On-Line:\r";
  if (has_disk())
    _output_buffer += "Device Detected P2\rNo Upgrade\r";
  _output_buffer += "D:\\>\r";

  _pio_ext = 0;

  _cwd->clear();

  if (_file)
    {
      fclose(_file);
      _file = NULL;
    }

  if (_pio)
    {
      set_pio_ext_b(0x02);
      z80->addCallback(20000, this, NULL);
    }
}

byte_t
VDIP::read_byte(void)
{
  if (_reset)
    return 0xff;

  int a = _output;
  _output = -1;
  DBG(2, form("KCemu/VDIP/read_byte",
              "VDIP: %04xh [%10Ld]: read_byte  -> %02x ('%c')\n",
              z80->getPC(), z80->getCounter(),
              a & 0xff, isprint(a) ? a : '.'));

  return a;
}

void
VDIP::latch_byte(void)
{
  if (_reset)
    return;

  if (!_output_buffer.empty())
    {
      _output = _output_buffer.at(0);
      _output_buffer = _output_buffer.substr(1);
    }

  DBG(2, form("KCemu/VDIP/latch_byte",
              "VDIP: %04xh [%10Ld]: latch_byte -> %02x ('%c')\n",
              z80->getPC(), z80->getCounter(),
              _output & 0xff, isprint(_output) ? _output : '.'));
}

void
VDIP::read_end(void)
{
  if (_reset)
    return;

  DBG(2, form("KCemu/VDIP/read_end",
              "VDIP: %04xh [%10Ld]: read_end, output buffer size = %d\n",
              z80->getPC(), z80->getCounter(),
              _output_buffer.length()));

  if (_output_buffer.empty())
    set_pio_ext_b(0x01);
  else
    z80->addCallback(50, this, NULL);
}

VDIP_CMD *
VDIP::decode_command(string buf)
{
  StringList *list = NULL;
  VDIP_CMD *vdip_cmd = NULL;

  if (buf.length() == 0)
    {
      vdip_cmd = VDIP_CMD::create_command(this, CMD_EMPTY);
    }
  else
    {
      list = new StringList(buf, ' ');
      string cmd = list->front();
      if (cmd.length() == 1)
        vdip_cmd = VDIP_CMD::create_command(this, (vdip_command_t)(cmd.at(0) & 0xff));
      else
        vdip_cmd = VDIP_CMD::create_command(this, list->front());
    }

  if (list && (list->size() > 1))
    {
      list->pop_front();
      vdip_cmd->set_args(list);
    }

  return vdip_cmd;
}

void
VDIP::write_byte(byte_t val)
{
  if (_reset)
    return;

  _input_data = val;
  DBG(2, form("KCemu/VDIP/write_byte",
              "VDIP: %04xh [%10Ld]: write_byte <- %02xh ('%c'), input buffer size = %d\n",
              z80->getPC(), z80->getCounter(),
              val, isprint(val) ? val : '.', _input_buffer.length()));
}

void
VDIP::write_end(void)
{
  if (_reset)
    return;

  DBG(2, form("KCemu/VDIP/write_end",
              "VDIP: %04xh [%10Ld]: write_end <- %02x ('%c'), input buffer size = %d, CMD = %s\n",
              z80->getPC(), z80->getCounter(),
              _input_data, isprint(_input_data) ? _input_data: '.',
              _input_buffer.length(),
              _cmd == NULL ? "<none>" : "active"));

  if (_cmd != NULL)
    {
      _cmd->handle_input(_input_data);
    }
  else if (_input_data < 0)
    {
      // ignore bogus signals on startup
    }
  else if (_input_data != 0x0d)
    {
      _input_buffer += _input_data;
      _input_data = -1;
    }
  else
    {
      string text;
      for (unsigned int a = 0; a < _input_buffer.length(); a++)
        text += isprint(_input_buffer.at(a)) ? _input_buffer.at(a) : '.';
      text += " [";
      for (unsigned int a = 0; a < _input_buffer.length(); a++)
        {
          char buf[10];
          snprintf(buf, sizeof (buf), "%02x ", _input_buffer.at(a) & 0xff);
          text += buf;
        }
      text += "]";
      DBG(2, form("KCemu/VDIP/execute",
                  "VDIP: %04xh [%10Ld]: execute command: %s\n",
                  z80->getPC(), z80->getCounter(),
                  text.c_str()));

      _cmd = decode_command(_input_buffer);
      _cmd->exec();
    }

  if (_cmd != NULL)
    {
      string response = _cmd->get_response(_input_buffer);
      if (response.length() > 0)
        {
          _input = false;
          _input_buffer = "";
          _output_buffer = response;
          set_pio_ext_b(0x02);
          z80->addCallback(1000, this, NULL);
          delete _cmd;
          _cmd = NULL;
        }
    }
}

int
VDIP::callback_A_in(void)
{
  return _output;
}

int
VDIP::callback_B_in(void)
{
  return -1;
}

void
VDIP::callback_A_out(byte_t val)
{
}

void
VDIP::callback_B_out(byte_t val)
{
}
