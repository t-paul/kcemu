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

#include <stdio.h> /* FIXME */
#include <string.h>
#include <ctype.h>

#include <iostream>
#include <iomanip>

#include "kc/system.h"
#include "kc/prefs/prefs.h"

#include "kc/memstream.h"

#include "kc/z80.h"
#include "kc/tape.h"
#include "kc/memory.h"
#include "kc/basicrun.h"

#include "sys/sysdep.h"

#include "ui/ui.h"
#include "ui/error.h"
#include "ui/status.h"

#include "libdbg/dbg.h"

#define TAPE_IF() (ui->getTapeInterface())

using namespace std;

class CMD_tape_load : public CMD
{
private:
  Tape *_t;
public:
  CMD_tape_load(Tape *t) : CMD("tape-load")
    {
      _t = t;
      register_cmd("tape-load", 0);
      register_cmd("tape-run", 1);
      register_cmd("tape-delete", 2);
    }

  void execute(CMD_Args *args, CMD_Context context)
    {
      bool ret;
      istream *is;
      word_t addr;
      const char *name;
      char *shortname;
      char buf[1000];
      kct_file_props_t props;

      if (!args)
        return;

      name = args->get_string_arg("tape-filename");
      if (!name)
        return;

      switch (context)
        {
        case 0:
        case 1:
          is = _t->read(name, &props);
          if (!is)
            return;

	  if (Preferences::instance()->get_kc_type() == KC_TYPE_Z1013)
	    ret = memory->loadRAM_Z1013(is, props.load_addr);
	  else
	    ret = memory->loadRAM(is, true);

          if (!ret)
	    break;

	  shortname = sys_basename(name);
	  snprintf(buf, sizeof(buf), _("File `%s' loaded."), shortname);
	  free(shortname);
	  Status::instance()->setMessage(buf);
	  if (context != 1)
	    break;

	  switch (props.type)
	    {
	    case KCT_TYPE_BAS:
	    case KCT_TYPE_BAS_P:
	      switch (Preferences::instance()->get_kc_type())
		{
		case KC_TYPE_85_1:
		case KC_TYPE_87:
		  /*
		   *  write autostart routine into tape buffer
		   */
		  addr = 0x100;
		  for (int a = 0;a < basicrun_len;a++)
		    memory->memWrite8(addr + a, basicrun[a]);
		  z80->jump(addr);
		  break;
		default:
		  break;
		}
	      break;
	    case KCT_TYPE_COM:
	      z80->jump(props.start_addr);
	      break;
	    default:
	      break;
	    }
	  TAPE_IF()->tapeNext();

          break;
        case 2:
          _t->remove(name);
          break;
        }
    }
};

class CMD_tape_attach : public CMD
{
private:
  Tape *_t;
  static const char * _path;

public:
  CMD_tape_attach(Tape *t) : CMD("tape-attach")
    {
      _t = t;
      register_cmd("tape-attach", 0);
      register_cmd("tape-detach", 3);
    }

  void execute(CMD_Args *args, CMD_Context context)
    {
      bool create;
      char buf[1000];
      tape_error_t err;
      char *shortname;
      const char *filename;

      create = false;
      filename = NULL;
      switch (context)
        {
          /*
           *  tape-attach
           */
        case 0:
          if (!args)
            args = new CMD_Args();
          filename = args->get_string_arg("filename");
          if (!filename)
            {
              args->set_string_arg("ui-file-select-title",
                                   _("Select tape-archive..."));
	      if (_path)
		args->set_string_arg("ui-file-select-path", _path);
              args->add_callback("ui-file-select-CB-ok", this, 1);
              args->add_callback("ui-file-select-CB-cancel", this, 1);
              CMD_EXEC_ARGS("ui-file-select", args);
              return;
            }
          break;
          /*
           *  ui-file-select-CB
           */
        case 1:
          if (args)
            filename = args->get_string_arg("filename");
          break;
          /*
           *  ui-dialog-yes-no-CB-yes
           */
        case 2:
          if (args)
            filename = args->get_string_arg("filename");
          create = true;
          break;
          /*
           *  tape-detach
           */
        case 3:
          tape->detach();
	  CMD_EXEC("ui-tape-detached");
          return;
        }
      
      if (filename)
        {
	  _path = filename;
	  err = tape->attach(filename, create);
          switch (err)
            {
            case TAPE_NOENT:
              if (!create)
                {
                  args->set_string_arg("ui-dialog-title", _("create file?"));
                  args->set_string_arg("ui-dialog-text",
                                       _("The file '%s' doesn't exist.\n"
                                         "Do you want to create it?"));
                  args->set_string_arg("ui-dialog-text-arg", "filename");
                  args->add_callback("ui-dialog-yes-no-CB-yes", this, 2);
                  CMD_EXEC_ARGS("ui-dialog-yes-no", args);
                }
              break;
            case TAPE_OK:
	    case TAPE_OK_READONLY:
              shortname = sys_basename(filename);
              snprintf(buf, sizeof(buf), _("tape-archive `%s' attached."), shortname);
	      free(shortname);
              Status::instance()->setMessage(buf);
	      /*  args should have 'filename' still set here! */
	      CMD_EXEC_ARGS("ui-tape-attached", args);
              break;
            default:
              Status::instance()->setMessage(_("Can't attach tape-archive."));
              break;
            }
        }
    }
};

const char * CMD_tape_attach::_path = NULL;

class CMD_tape_export : public CMD
{
private:
  Tape *_t;
  static const char * _path;

public:
  CMD_tape_export(Tape *t) : CMD("tape-export")
    {
      _t = t;
      register_cmd("tape-export");
      register_cmd("tape-export-wav", 2);
    }

  void execute(CMD_Args *args, CMD_Context context)
    {
      char buf[1000];
      const char *filename, *tapename;

      filename = 0;
      tapename = args->get_string_arg("tape-filename");
      if (!tapename)
        return;

      switch (context)
        {
        case 0:
          if (!args)
            args = new CMD_Args();
          filename = args->get_string_arg("filename");
          if (!filename)
            {
              args->set_string_arg("ui-file-select-title", _("Export As..."));
	      if (_path)
		args->set_string_arg("ui-file-select-path", _path);
              args->add_callback("ui-file-select-CB-ok", this, 1);
              args->add_callback("ui-file-select-CB-cancel", this, 1);
              CMD_EXEC_ARGS("ui-file-select", args);
            }
	  /* fall through */
        case 1:
          if (args)
            filename = args->get_string_arg("filename");
	  if (!filename)
	    return;

	  _path = filename;

	  if (tape->export_tap(tapename, filename) == TAPE_OK)
	    {
	      sprintf(buf, _("File `%s' saved."), filename);
	      Status::instance()->setMessage(buf);
	    }
	  else
	    {
	      Status::instance()->setMessage(_("Can't export file."));
	    }

          break;

	case 2:
	  if (!args)
	    args = new CMD_Args();
	  filename = args->get_string_arg("filename");
	  if (!filename)
	    {
              args->set_string_arg("ui-file-select-title", _("Export As..."));
	      if (_path)
		args->set_string_arg("ui-file-select-path", _path);
              args->add_callback("ui-file-select-CB-ok", this, 3);
              args->add_callback("ui-file-select-CB-cancel", this, 3);
              CMD_EXEC_ARGS("ui-file-select", args);
            }
	  /* fall through */
	case 3:
	  if (args)
	    filename = args->get_string_arg("filename");
	  if (!filename)
	    return;

	  _path = filename;
	  
	  if (tape->export_wav(tapename, filename) == TAPE_OK)
	    {
	      sprintf(buf, _("File `%s' saved."), filename);
	      Status::instance()->setMessage(buf);
	    }
	  else
	    {
	      Status::instance()->setMessage(_("Can't export file."));
	    }

	  break;
        }
    }
};

const char * CMD_tape_export::_path = NULL;

class CMD_tape_add_file : public CMD
{
private:
  Tape *_t;
  static const char * _path;

public:
  CMD_tape_add_file(Tape *t) : CMD("tape-add-file")
    {
      _t = t;
      register_cmd("tape-add-file", 0);
      register_cmd("tape-rename-file", 3);
    }

  void execute(CMD_Args *args, CMD_Context context)
    {
      CMD_Args *a;
      fileio_prop_t *prop;
      kct_file_type_t type;
      const char *filename, *tapename;

      filename = 0;
      if (args)
	filename = args->get_string_arg("filename");

      switch (context)
        {
        case 0:
	  if (!filename)
	    {
	      a = new CMD_Args();
	      a->set_string_arg("ui-file-select-title",
				_("Select file..."));
	      if (_path)
		a->set_string_arg("ui-file-select-path", _path);
	      a->add_callback("ui-file-select-CB-ok", this, 1);
	      CMD_EXEC_ARGS("ui-file-select", a);
	      return;
	    }
	  /* fall through */

        case 1:
          if (!filename)
            return;

	  _path = filename;
	  _t->add(filename);
	  break;

	case 2:
	  /*
	   *  this is the callback entry ui-tape-name-edit-CB-ok
	   *  which is used by the function add_file()
	   */
	  prop = (fileio_prop_t *)args->get_pointer_arg("prop");
	  tapename = args->get_string_arg("tape-filename");
	  type = (kct_file_type_t)args->get_long_arg("type");

	  _t->add_file(tapename, prop, type, KCT_MACHINE_ALL);
	  break;

	case 3:
	  if (!filename)
	    return;

	  tapename = args->get_string_arg("tape-filename");
	  if (!tapename)
	    return;

	  _t->rename(filename, tapename);
	  break;
        }
    }
};

const char * CMD_tape_add_file::_path = NULL;

class CMD_tape_play : public CMD
{
private:
  Tape *_t;
public:
  CMD_tape_play(Tape *t) : CMD("tape-play")
    {
      _t = t;
      register_cmd("tape-play", 0);
      register_cmd("tape-record", 1);
      register_cmd("tape-stop", 2);
    }
  void execute(CMD_Args *args, CMD_Context context)
    {
      int delay;
      const char *name;

      switch (context)
        {
	case 0:
          if (!args)
            return;

          name = args->get_string_arg("tape-filename");
          if (!name)
            return;
          
	  delay = 0;
	  if (args->has_arg("tape-play-delay"))
	    delay = args->get_long_arg("tape-play-delay");

          _t->play(name, delay);
          break;

        case 1:
          _t->record();
          break;

	case 2:
	  _t->stop();
	  break;
        }
    }
};

static void
dump_buf(const char *buf, int block)
{
  int a, b;

  cout << "tape block dump: block "
       << hex << setw(2) << setfill('0') << block << "h [saved block number: "
       << hex << setw(2) << setfill('0') << (((int)*buf) & 0xff) << "h]"
       << endl;

  for (a = 0;a < 128;a += 16)
    {
      cout << hex << setw(2) << setfill('0') << a << ":";
      for (b = 0;b < 16;b++)
        {
	  cout << " " << hex << setw(2) << setfill('0') << (buf[a + b + 1] & 0xff);
          if (b == 8)
            cout << " -";
        }
      cout << " | ";
      for (b = 0;b < 16;b++)
        {
          cout << (isprint(buf[a + b + 1]) ? buf[a + b + 1] : '.');
          if (b == 8)
            cout << " - ";
        }
      cout << endl;
    }
}

Tape::Tape(int bit_0, int bit_1, int bit_s, int start_block) : Callback("Tape")
{
    BIT_0 = bit_0;
    BIT_1 = bit_1;
    BIT_S = bit_s;
    _start_block = start_block;

    _record = false;
    _power = 0; /* FIXME: */
    _flip_flop = 0;
    _bits = -1;
    _state = 0;
    _sync = 2;

    _tape_cb = 0;

    _cmd_tape_load     = new CMD_tape_load(this);
    _cmd_tape_play     = new CMD_tape_play(this);
    _cmd_tape_attach   = new CMD_tape_attach(this);
    _cmd_tape_export   = new CMD_tape_export(this);
    _cmd_tape_add_file = new CMD_tape_add_file(this);
}

Tape::~Tape()
{
  delete _cmd_tape_load;
  delete _cmd_tape_play;
  delete _cmd_tape_attach;
  delete _cmd_tape_export;
  delete _cmd_tape_add_file;
}

void
Tape::set_tape_callback(TapeCallback *tape_cb)
{
  _tape_cb = tape_cb;
}

void
Tape::power(bool val)
{
  _power = val;

  DBG(1, form("KCemu/Tape/power",
	      "Tape::power(): Power %s\n",
	      _power ? "on" : "off"));

  TAPE_IF()->tapePower(_power);
}

void
Tape::record(void)
{
  _record = true;
  _sync = 2;
  _sync_count = 200;
  _flip_flop = 0;
  _byte_counter = 0;
  _state = 0;
  
  _play = false;
  _record = true;

  _os = new memstream();
}

void
Tape::play(const char *name, int delay)
{
  long offset;
  kct_file_props_t props;
  
  DBG(1, form("KCemu/Tape/play",
	      "Tape::play(): name = '%s', delay = %d seconds\n",
	      name, delay));

  if (name == NULL)
    return;

  _play = true;
  _record = false;

  _init = 4000;
  _last_block = 0;
  _record = false;
  _state = 0;
  _byte_counter = BLOCK_SIZE;
  _bytes_read = 0;
  _flip_flop = 0;
  _is = _kct_file.read(name, &props);
  _file_size = props.size;
  _file_type = props.type;
  if (_is == NULL)
    {
      DBG(1, form("KCemu/Tape/play",
		  "Tape::play(): _is == NULL"));
      stop();
    }
  else
    {
      offset = get_delay(delay);
      DBG(1, form("KCemu/Tape/play",
		  "Tape::play(): trigger offset is %ld\n",
		  offset));
      z80->addCallback(offset, this, (void *)2);
    }
}

const char *
Tape::get_filename(byte_t *data)
{
  int a;
  static char filename[9];

  memcpy(filename, data, 8);
  filename[8] = '\0';
  for (a = 7;(a > 0) && (filename[a] != ' ');a++)
    filename[a] = '\0';

  return filename;
}

bool
Tape::check_addr(byte_t *data, long size)
{
  unsigned short load, end, start, x;
  
  /*
   *  finally look if specified addresses are plausible
   */
  x     = data[17];
  load  = data[18] | (data[19] << 8);
  end   = data[20] | (data[21] << 8);
  start = data[22] | (data[23] << 8);

  if (load >= end)
    return false;

  if ((end - load - 1) > size)
    return false;

  if (x > 2)
    if ((start < load) || (start >= end))
      return false;

  return true;
}

bool
Tape::check_com(byte_t *data, long size)
{
  int a;

  if ((data[9] == 'C') && (data[10] == 'O') && (data[11] == 'M'))
    if ((data[17] >= 2) && (data[18] <= 0x0a))
      return check_addr(data, size - 129);

  for (a = 1;a < 9;a++)
    {
      if (data[a] == '\0')
	break;
      if ((data[a] >= 'A') && (data[a] <= 'Z'))
	continue;
      if ((data[a] >= 'a') && (data[a] <= 'z'))
	continue;
      if (strchr(" .&+", data[a]) != NULL)
	continue;
      break;
    }

  if (a > 1)
    return check_addr(data, size - 129);

  return false;
}

void
Tape::stop(void)
{
  byte_t *ptr;
  ofstream os;
  int load, start;
  kct_file_type_t type;
  const char *filename;

  _play = false;
  _record = false;

  TAPE_IF()->tapeProgress(0);

  if (!_os)
    return;
  if (((memstream *)_os)->size() == 0)
    return;

  os.open("/tmp/kcemu.output", ios::out | ios::binary);
  if (!os)
    cout << "Tape::stop(): can't open output file" << endl;
  else
    {
      ptr = (byte_t *)((memstream *)_os)->str();
      DBG(1, form("KCemu/Tape/write",
		  "Tape::stop(): writing output file (%d bytes)\n",
		  ((memstream *)_os)->size()));
      os.write((char *)ptr, ((memstream *)_os)->size());

      load = 0;
      start = 0;
      filename = "FILE";
      if ((ptr[1] == 0xd3) && (ptr[2] == 0xd3) && (ptr[3] == 0xd3))
	{
	  type = KCT_TYPE_BAS;
	  filename = get_filename(ptr + 4);
	}
      else if ((ptr[1] == 0xd4) && (ptr[2] == 0xd4) && (ptr[3] == 0xd4))
	{
	  type = KCT_TYPE_DATA;
	  filename = get_filename(ptr + 4);
	}
      else if ((ptr[1] == 0xd5) && (ptr[2] == 0xd5) && (ptr[3] == 0xd5))
	{
	  type = KCT_TYPE_LIST;
	  filename = get_filename(ptr + 4);
	}
      else if ((ptr[1] == 0xd7) && (ptr[2] == 0xd7) && (ptr[3] == 0xd7))
	{
	  type = KCT_TYPE_BAS_P;
	  filename = get_filename(ptr + 4);
	}
      else
	{
	  if (check_com(ptr, ((memstream *)_os)->size()))
	    {
	      type = KCT_TYPE_COM;
	      load = ptr[18] | (ptr[19] << 8);
	      start = ptr[22] | (ptr[23] << 8);
	      filename = get_filename(ptr + 1);
	    }
	  else
	    {
	      type = KCT_TYPE_BIN;
	    }
	}

      DBG(1, form("KCemu/Tape/write",
		  "Tape::stop(): type = %s, load = %04xh, start = %04xh\n",
		  _kct_file.type_name(type), load, start));
      _kct_file.write(filename,
                      ptr, ((memstream *)_os)->size(), load, start,
                      type, KCT_MACHINE_ALL);

      update_tape_list();
      os.close();
    }
  delete _os;
  _os = NULL;
}

void
Tape::seek(int percent)
{
  unsigned long offset;

  DBG(1, form("KCemu/Tape/seek",
	      "Tape::seek(): seek to %d percent\n",
	      percent));
  if (!_is)
    return;

  offset = (percent * _file_size) / 100;
  offset -= (offset % 129);

  DBG(1, form("KCemu/Tape/seek",
	      "Tape::seek(): new offset is %ld (file size is %ld)\n",
	      offset, _file_size));

  _is->seekg(offset);
}

long
Tape::get_delay(int seconds)
{
  return 600 * BIT_S * seconds;
}

void
Tape::callback(void *data)
{
  long edge = (long)data;
  
  if (_play)
    do_play(edge);
}

void
Tape::do_play_bic(int edge)
{
  static int bit;
  static byte_t byte;
  static int bytes;

  if (_state == 0)
    {
      _state++;
      bytes = 0;
    }

  if (_init > 0)
    {
      _init--;
      z80->addCallback(600, this, (void *)1);
      z80->addCallback(1200, this, (void *)0);
      return;
    }

  if (_init == 0)
    {
      bit = 0;
      _init = -1;
      z80->addCallback(1300, this, (void *)1);
      z80->addCallback(2600, this, (void *)0);
      return;
    }

  if (_is == NULL)
    return;

  if (bit == 0)
    {
      bytes++;
      byte = _is->get();
      int peek = ((memstream *)_is)->peek(); // FIXME: bug in memstream
      if (peek == EOF)
	return;
    }

  if (bit < 8)
    {
      if (byte & (1 << bit))
	{
	  z80->addCallback( 600, this, (void *)1);
	  z80->addCallback(1200, this, (void *)1);
	  z80->addCallback(1800, this, (void *)1);
	  z80->addCallback(2400, this, (void *)0);
	}
      else
	{
	  z80->addCallback(1300, this, (void *)1);
	  z80->addCallback(2600, this, (void *)0);
	}
    }
  else
    {
      z80->addCallback( 600, this, (void *)1);
      z80->addCallback(1200, this, (void *)1);
      z80->addCallback(1800, this, (void *)1);
      z80->addCallback(2400, this, (void *)0);
    }

  bit++;

  if (bit == 10)
    {
      bit = 0;
      _init = 0;

      if (bytes == 16)
	_init = 4000;
    }

  if ((bytes % 128) == 127)
    TAPE_IF()->tapeProgress((100 * bytes) / _file_size);
}

void
Tape::do_play_z1013(int edge)
{
  static int idx;
  static int bidx;
  static int byte;
  static int bytes;
  static int blocks;
  static int hs_flag;

  if (_is == NULL)
    return;

  switch (_state)
    {
    case 0: // SYNC
      z80->addCallback(1500, this, (void *)1);
      z80->addCallback(3000, this, (void *)0);
      
      if (_init > 0)
	{
	  _init--;
	}
      else
	{
	  idx = 20;
	  bytes = 0;
	  blocks = 0;
	  hs_flag = 0;
	  _state++;
	}
      break;

    case 1: // BLOCK SYNC
      z80->addCallback(1550, this, (void *)1);
      z80->addCallback(3100, this, (void *)0);

      if (idx > 0)
	{
	  idx--;
	}
      else
	{
	  idx = 20;
	  _state++;
	}
      break;

    case 2: // BLOCK START
      z80->addCallback(775, this, (void *)1);
      z80->addCallback(1550, this, (void *)0);
      bidx = 0;
      _state++;
      break;

    case 3: // BLOCK DATA
      if (bidx == 0)
	{
	  byte = _is->get();
	  if (byte == EOF)
	    return;

	  if (blocks == 0)
	    {
	      if ((bytes == 15) && (byte == 0xd3))
		hs_flag++;
	      if ((bytes == 16) && (byte == 0xd3))
		hs_flag++;
	      if ((bytes == 17) && (byte == 0xd3))
		hs_flag++;
	    }

	  //cout << hex << setw(2) << setfill('0') << (int)byte << " ";

	  bytes++;
	}

      if (byte & (1 << bidx))
	{
	  z80->addCallback(775, this, (void *)0);
	}
      else
	{
	  z80->addCallback(387, this, (void *)1);
	  z80->addCallback(775, this, (void *)0);
	}

      bidx++;
      if (bidx == 8)
	{
	  bidx = 0;
	  if ((bytes % 36) == 0)
	    {
	      _state = 1;
	      if (hs_flag == 3)
		{
		  // long sync after first header save block
		  idx = 1000;
		  hs_flag = 0;
		}

	      //cout << endl;

	      TAPE_IF()->tapeProgress((100 * bytes) / _file_size);
	      blocks++;
	    }
	}

      break;
    }
}

void
Tape::do_play_basicode(int edge)
{
  static int idx;
  static int bidx;
  static int byte;
  static int bytes;
  static int checksum;
  static int done;

  if (_is == NULL)
    return;

  switch (_state)
    {
    case 0: // SYNC HEADER
      if (_init > 0)
	{
	  _init = 0;
	  idx = 12000;
	}

      z80->addCallback(BIT_0, this, (void *)1);
      z80->addCallback(2 * BIT_0, this, (void *)0);

      if (--idx == 0)
	{
	  idx = 20;
	  byte = 0x82; // START BYTE
	  bytes = 0;
	  bidx = 0;
	  checksum = 0;
	  done = 0;
	  _state++;
	}
      break;
    case 1: // PROGRAM DATA
      if (bidx == 11)
	{
	  if (done)
	    {
	      // END SYNC
	      idx = 400;
	      z80->addCallback(BIT_0, this, (void *)1);
	      z80->addCallback(2 * BIT_0, this, (void *)0);
	      _state = 3;
	      return;
	    }
	  else if (byte == 0x83) // END BYTE
	    {
	      byte = checksum;
	      done = 1;
	    }
	  else
	    {
	      byte = _is->get();
	      if (byte == EOF)
		byte = 0x83; // END BYTE
	      else
		byte ^= 0x80;
	    }

	  bytes++;
	  bidx = 0;
	}

      switch (bidx)
	{
	case 0: // START BIT
	  checksum ^= byte;
	  z80->addCallback(BIT_1, this, (void *)1);
	  z80->addCallback(2 * BIT_1, this, (void *)0);
	  if ((bytes % 256) == 255)
	    TAPE_IF()->tapeProgress((100 * bytes) / _file_size);
	  break;
	case 9: // STOP BITS
	case 10:
	  z80->addCallback(BIT_0, this, (void *)1);
	  z80->addCallback(2 * BIT_0, this, (void *)0);
	  break;
	default: // DATA BITS 0 - 7
	  if (byte & (1 << (bidx - 1)))
	    {
	      z80->addCallback(BIT_0, this, (void *)1);
	      z80->addCallback(2 * BIT_0, this, (void *)0);
	      _state++;
	    }
	  else
	    {
	      z80->addCallback(BIT_1, this, (void *)1);
	      z80->addCallback(2 * BIT_1, this, (void *)0);
	    }
	  break;
	}
      
      bidx++;

      break;
    case 2: // second wave for 1 bits
      z80->addCallback(BIT_0, this, (void *)1);
      z80->addCallback(2 * BIT_0, this, (void *)0);
      _state--;
      break;
    case 3: // SYNC TRAILER
      if (idx > 0)
	{
	  idx--;
	  z80->addCallback(BIT_0, this, (void *)1);
	  z80->addCallback(2 * BIT_0, this, (void *)0);
	}
      break;
    }
}

void
Tape::do_play(int edge)
{
  int len;

  if (_tape_cb)
    _tape_cb->tape_callback(edge);

  if (edge == 1)
    {
      /*
       *  rising edge of signal
       */
      return;
    }

  if (edge == 2)
    {
      /*
       *  trigger signal to get everything running...
       *  this comes (delayed) from the automatic playing so we
       *  ignore this if power is off
       */
      DBG(1, form("KCemu/Tape/play",
		  "Tape::play(): got first signal (power = %d)\n", _power));
      if (!_power)
	return;
    }

  if (_file_type == KCT_TYPE_BASICODE)
    {
      do_play_basicode(edge);
      return;
    }

  switch (Preferences::instance()->get_kc_type())
    {
    case KC_TYPE_A5105:
      do_play_bic(edge);
      return;
    case KC_TYPE_Z1013:
      do_play_z1013(edge);
      return;
    default:
      break;
    }

  /*
   *  sync tape and computer
   */
  if (_init)
    {
      _init--;
      z80->addCallback(BIT_1, this, (void *)1);
      z80->addCallback(2 * BIT_1, this, (void *)0);
      //if (_init < 8)
      //cout << 'I' << flush;
      if (_init == 0)
        {
          _bits = 0;
          //cout << '\n' << flush;
        }
      return;
    }

  /*
   *  check for new block
   */
  if (_byte_counter == BLOCK_SIZE)
    {
      if (_last_block)
        {
          TAPE_IF()->tapeNext();
          /*
           *  end of file
           */
          return;
        }

      if (_is == NULL)
	{
	  /*
	   *  abort if no input stream
	   */
	  DBG(1, form("KCemu/Tape/play",
		      "Tape::play(): no input stream\n"));
	  return;
	}

      _bytes_read = ((memstream *)_is)->tellg(); // FIXME: bug in memstream

      /*
       *  start_block is 0 for KC 85/1, KC 87 .COM files
       *                 1 for KC 85/1, KC 87 basic files
       *                 1 for KC 85/3, KC 85/4 all files
       */
      //_block = _bytes_read / 128;
      //if (_file_type == KCT_TYPE_BAS)
      //_block += 1;
      //else
      //_block += _start_block;

      _byte_counter = 0;

      memset(_buf, 0, 129);

      /*
       *  use get() for now; read() / gcount() is broken for
       *  the current memstream class
       */
      len = 0;
      while (len < 129)
	{
	  int c = _is->get();
	  if (c == EOF)
	    break;
	  _buf[len++] = c;
	}

      _block = _buf[0];
      TAPE_IF()->tapeProgress((100 * _bytes_read) / _file_size);

      if (_block > _start_block)
	{
	  _init = 200;
	  if (_file_type == KCT_TYPE_LIST)
	    _init = 2000;
	}
      else
	{
	  _init = 4000;
	}
      
      int peek = ((memstream *)_is)->peek(); // FIXME: bug in memstream
      if ((len != 129) || (peek == EOF))
        {
          /*
           *  last block
           */
          DBG(1, form("KCemu/Tape/play",
		      "Tape::play(): len = %d, peek() = %d\n",
		      len, peek));
	  
          _last_block = 1;
	  delete _is;
	  _is = NULL;
        }
      
      DBG(1, form("KCemu/Tape/play",
		  "Tape::play(): starting tape block %d [%d/%d]\n",
		  _block, _bytes_read, _file_size));
      
      z80->addCallback(BIT_1, this, (void *)1);
      z80->addCallback(2 * BIT_1, this, (void *)0);
      return;
    }

  /*
   *  handle special bytes:
   *
   *  byte   0: block number
   *  byte 129: checksum
   */
  if (_bits == -1) {
    _bits = 8;
    switch (_byte_counter)
      {
      case 0:
        _byte = _block;
        _crc = 0;
        break;
      case 129:
        _byte = _crc;
        break;
      default:
        _byte = _buf[_byte_counter]; // _buf[0] is block number!
        _crc += _byte;
        break;
      }
    _byte_counter++;
    //if ((_byte_counter % 8) == 0)
    //cout << '\n' << flush;
  }

  /*
   *  sync bit
   */
  if (_bits == 0)
    {
      //cout << 'S' << flush;
      z80->addCallback(BIT_S, this, (void *)1);
      z80->addCallback(2 * BIT_S, this, (void *)0);
    }
  else
    {
      if (_byte & (1 << (8 - _bits)))
        {
          /*
           *  1 bit
           */
          //cout << '1' << flush;
          z80->addCallback(BIT_1, this, (void *)1);
          z80->addCallback(2 * BIT_1, this, (void *)0);
	}
      else
        {
          /*
           *  0 bit
           */
          //cout << '0' << flush;
          z80->addCallback(BIT_0, this, (void *)1);
          z80->addCallback(2 * BIT_0, this, (void *)0);
	}
    }
  _bits--;
}

void
Tape::tape_signal_bic(long diff)
{
  int bit_type;
  static int byte, obit;

  if (diff > 1000)
    bit_type = 0;
  else
    bit_type = 1;

  if (_sync > 0)
    {
      if (bit_type == 1)
	_sync--;
      else
	_sync = 100;
      return;
    }

  if ((_sync == 0) && (bit_type == 1))
    {
      _bits = 11;
      _byte_counter = 0;
      return;
    }

  _sync = -1;

  if (_flip_flop == 0)
    _flip_flop++;
  else if (obit == bit_type)
    _flip_flop++;
  
  if ((bit_type == 0) && (_flip_flop == 2))
    {
      byte >>= 1;
      _flip_flop = 0;
      byte &= 0x3ff;
      _bits--;
    }
  else if ((bit_type == 1) && (_flip_flop == 4))
    {
      byte >>= 1;
      _flip_flop = 0;
      byte |= 0x400;
      _bits--;
    }

  if (_bits == 0)
    {
      _bits = 11;
      if (byte & 1)
	{
	  _sync = 100;
	  cout << "SYNC" << endl;
	}
      else
	{
	  _byte = (byte >> 1) & 0xff;
	  cout << "byte: " << hex << (int)_byte << " - " << (int)byte << endl;
	  _os->write((const char *)&_byte, 1);
	}
    }

  obit = bit_type;
}

void
Tape::tape_signal_z1013(long diff)
{
  int bit, sum;
  static int idx;
  static int sync;
  static int byte;
  static int bidx;
  static int state = 0;
  static bool first;
  static byte_t buf[36];

  if (diff > 20000)
    {
      state = 0;
      sync = 100;
      return;
    }


  switch (state)
    {
    case 0: // SYNC
      if (sync > 0)
	{
	  if (diff > 1200)
	    sync--;
	  else
	    sync = 100;
	}

      if (sync == 0)
	{
	  sync = 100;
	  state++;
	}

      break;
    case 1: // BLOCK SYNC
      if (diff > 1200)
	break;

      if (diff < 580)
	{
	  sync = 100;
	  state = 0;
	  break;
	}

      state++;
      break;
    case 2: // BLOCK START
      if ((diff < 580) || (diff > 1200))
	{
	  sync = 100;
	  state = 0;
	  break;
	}

      idx = 0;
      bidx = 0;
      first = false;
      state++;
      break;
    case 3: // BLOCK DATA
      if (diff > 1200)
	{
	  sync = 100;
	  state = 0;
	  break;
	}

      bit = (diff > 580) ? 128 : 0;
      if (bit == 0)
        {
	  if (first)
	    {
	      first = false;
	    }
	  else
	    {
	      first = true;
	      break;
	    }
        }

      bidx++;
      byte >>= 1;
      byte = (byte & 0x7f) | bit;

      if (bidx == 8)
	{
	  bidx = 0;
	  buf[idx++] = byte;
	  if (idx == 36)
	    {
	      sum = 0;
	      for (idx = 0;idx < 34;idx += 2)
		sum += buf[idx] | (buf[idx + 1] << 8);
	      sum &= 0xffff;

	      //for (idx = 0;idx < 36;idx++)
	      //cout << hex << setw(2) << setfill('0') << (int)buf[idx] << " ";
	      
	      //cout << "=> " << (sum & 0xff) << " " << ((sum >> 8) & 0xff) << endl;

	      for (idx = 0;idx < 36;idx++)
		_os->write((const char *)&buf[idx], 1);

	      idx = 0;
	      state = 1;
	    }
	}
      break;
    }
}

void
Tape::tape_signal(void)
{
  long diff;
  int bit_type;
  static unsigned long long c_old, c_new;

  if (!_power)
    return;

  /*
   *  abort if no output stream
   */
  if (_os == NULL)
    return;

  if (!_record)
    return;

  c_new = z80->getCounter();
  diff = c_new - c_old;
  c_old = c_new;

  switch (Preferences::instance()->get_kc_type())
    {
    case KC_TYPE_A5105:
      tape_signal_bic(diff);
      return;
    case KC_TYPE_Z1013:
      tape_signal_z1013(diff);
      return;
    default:
      break;
    }

  if (diff < ((BIT_0 + BIT_1) / 2))
    bit_type = 0;
  else if (diff > ((BIT_S + BIT_1) / 2))
    bit_type = 2;
  else
    bit_type = 1;

  if (_sync > 0)
    {
      /*
       *  skip sync block
       */
      if (bit_type == 1) return;
      /*
       *  skip first stop bit
       */
      if (bit_type == 2) 
        {
          _sync--;
          cout << '*' << flush;
          _bits = 8;
          _byte_counter = 0;
        }
      else
        {
          cout << '?' << flush;
        }
      return;
    }
  
  if (++_flip_flop == 2)
    {
      _flip_flop = 0;
      switch (bit_type)
        {
	case 0: /* 0 bit */
          DBG(2, form("KCemu/Tape/record", "0"));
          _byte >>= 1;
          _byte &= 0x7f;
          _bits--;
          break;
	case 1: /* 1 bit */
          DBG(2, form("KCemu/Tape/record", "1"));
          _byte >>= 1;
          _byte |= 0x80;
          _bits--;
          break;
	case 2: /* separator bit */
          DBG(2, form("KCemu/Tape/record", "S"));
          _bits = 8;
          break;
	}
      if (_bits == 0)
        {
          _bits = -1;
          switch (_state)
            {
	    case 0:
              /*
               *  block number
               */
              _state++;
              _block = _byte;
              _buf[0] = _byte;
              memset(&_buf[1], 0, 128);
              _crc_calculated = 0;
              DBG(1, form("KCemu/Tape/block-info",
                          "Tape output: block %02x\n",
                          _block));
              break;
	    case 1:
              /*
               *  block data
               */
              _buf[_byte_counter + 1] = _byte;
              _byte_counter++;
              _crc_calculated += _byte;
              if (_byte_counter == 128)
                {
                  _byte_counter = 0;
                  _state++;
                  dump_buf((char *)_buf, _block);
		}
              break;
            case 2:
              /*
               *  block crc
               */
              _state++;
              _crc = _byte;
              if (_crc_calculated == _crc)
                _os->write((char *)_buf, 129);
              else
                DBG(0, form("KCemu/Tape/block-info",
                            "Tape output: wrong crc: %04x != %04x\n",
                            _crc, _crc_calculated));
              break;
            case 3:
              /*
               *  skip one byte ?!?
               */
              _state = 0;
              _sync = 2;
              break;
	    }
	}
    }
}

tape_error_t
Tape::attach(const char *filename, bool create)
{
  KCTDir *dir;
  kct_error_t err;
  
  _kct_file.close();
  if (create)
    {
      DBG(0, form("KCemu/Tape/attach",
                  "Tape::attach(): create (%s)\n",
                  filename));
      if (_kct_file.create(filename) != KCT_OK)
        return TAPE_ERROR;
    }
  else
    {
      DBG(0, form("KCemu/Tape/attach",
                  "Tape::attach(): open (%s)\n",
                  filename));
      err = _kct_file.open(filename);
      switch (err)
        {
        case KCT_OK:
	case KCT_OK_READONLY:
          break;
        case KCT_ERROR_NOENT:
          return TAPE_NOENT;
        default:
          return TAPE_ERROR;
        }
    }

  dir = _kct_file.readdir();

  TAPE_IF()->tapeAttach("");
  for (KCTDir::iterator it = dir->begin();it != dir->end();it++)
    {
      DBG(1, form("KCemu/Tape/attach",
                  "Tape::attach(): %s (0x%02x)\n",
                  (*it)->name, (*it)->type));
      TAPE_IF()->tapeAddFile((*it)->name,
			     (*it)->load_addr,
			     (*it)->start_addr,
			     (*it)->uncompressed_size,
			     (*it)->type);
    }

  if (_kct_file.is_readonly())
    return TAPE_OK_READONLY;
  
  return TAPE_OK;
}

tape_error_t
Tape::detach(void)
{
  DBG(0, form("KCemu/Tape/detach",
	      "Tape::detach(): close\n"));

  stop();
  _kct_file.close();
  TAPE_IF()->tapeDetach();
  TAPE_IF()->tapeProgress(0);
  return TAPE_OK;
}

tape_error_t
Tape::add(const char *name)
{
  fileio_prop_t *ptr, *prop;

  if (fileio_load_file(name, &prop) != 0)
    return TAPE_ERROR;
  
  if (DBG_check("KCemu/Tape/add"))
    fileio_debug_dump(prop, 0);

  for (ptr = prop;ptr != NULL;ptr = ptr->next)
    {
      kct_file_type_t type;

      switch (ptr->type)
        {
        case FILEIO_TYPE_COM:
	  type = KCT_TYPE_COM;
          DBG(0, form("KCemu/Tape/add",
                      "Tape::add(): '%s' %04x/%04x [COM]\n",
                      (const char *)&ptr->name[0],
                      ptr->load_addr, ptr->start_addr));
          break;
        case FILEIO_TYPE_BAS:
	  type = KCT_TYPE_BAS;
          DBG(0, form("KCemu/Tape/add",
                      "Tape::add(): '%s' [BAS]\n",
                      (const char *)&ptr->name[0]));
          break;
        case FILEIO_TYPE_DATA:
	  type = KCT_TYPE_DATA;
          DBG(0, form("KCemu/Tape/add",
                      "Tape::add(): '%s' [DATA]\n",
                      (const char *)&ptr->name[0]));
          break;
        case FILEIO_TYPE_LIST:
	  type = KCT_TYPE_LIST;
          DBG(0, form("KCemu/Tape/add",
                      "Tape::add(): '%s' [LIST]\n",
                      (const char *)&ptr->name[0]));
          break;
        case FILEIO_TYPE_PROT_BAS:
	  type = KCT_TYPE_BAS_P;
          DBG(0, form("KCemu/Tape/add",
                      "Tape::add(): '%s' [BAS_P]\n",
                      (const char *)&ptr->name[0]));
          break;
        case FILEIO_TYPE_BASICODE:
	  type = KCT_TYPE_BASICODE;
          DBG(0, form("KCemu/Tape/add",
                      "Tape::add(): '%s' [BASICODE]\n",
                      (const char *)&ptr->name[0]));
          break;
        default:
	  Error::instance()->info(_("The format of the selected file is not recognized."));
          DBG(0, form("KCemu/Tape/add",
		      "Tape::add(): '%s' %04x/%04x [ignored]\n",
		      (const char *)&ptr->name[0],
                      ptr->load_addr,
		      ptr->start_addr));
          return TAPE_ERROR;
        }

      add_file((const char *)&ptr->name[0], ptr, type, KCT_MACHINE_ALL);
    }
  update_tape_list();
  fileio_free_prop(&prop);

  return TAPE_OK;
}

tape_error_t
Tape::add_file(const char *name,
	       fileio_prop_t *prop,
	       kct_file_type_t type,
	       kct_machine_type_t machine)
{
  char buf[1000];
  kct_error_t err;
  CMD_Args *args = 0;

  err = _kct_file.write(name,
			prop->data,
			prop->size,
			prop->load_addr,
			prop->start_addr,
			type, KCT_MACHINE_ALL);
  if (err != KCT_ERROR_EXIST)
    {
      snprintf(buf, 1000, _("File `%s' [%s] added."), name, prop->filetype);
      Status::instance()->setMessage(buf);
      return TAPE_OK;
    }

  if (args == 0)
    args = new CMD_Args();
  
  args->set_string_arg("tape-rename-title",
		       _("The file you selected has a name that is already\n"
			 "present in the current tape archive.\n"
			 "\n"
			 "Please select a different name to add this file\n"
			 "to the tape archive or use the cancel button to\n"
			 "skip this file."));
  args->set_string_arg("tape-filename", name);
  args->set_pointer_arg("prop", prop);
  args->set_long_arg("type", type);
  args->add_callback("ui-tape-name-edit-CB-ok", _cmd_tape_add_file, 2);
  CMD_EXEC_ARGS("ui-tape-name-edit", args);

  return TAPE_OK;
}

void
Tape::update_tape_list(void)
{
  KCTDir *dir;

  /*
   *  FIXME: update tape listing
   */
  dir = _kct_file.readdir();
  TAPE_IF()->tapeAttach("");
  for (KCTDir::iterator it = dir->begin();it != dir->end();it++)
    {
      TAPE_IF()->tapeAddFile((*it)->name,
                             (*it)->load_addr,
                             (*it)->start_addr,
                             (*it)->uncompressed_size,
                             (*it)->type);
    }
}

tape_error_t
Tape::add_raw(const char     *filename,
              const char     *tape_filename,
              const char     *kc_filename,
	      unsigned short  load,
	      unsigned short  start,
              bool            autostart)
{
  FILE *f;
  int len;
  unsigned char buf[65536 + 256];
      
  if ((f = fopen(filename, "rb")) == NULL)
    {
      cerr << "can't open `" << filename << "'" << endl;
      return TAPE_ERROR;
    }

  memset(buf, 0, 128);
  len = fread(buf + 128, 1, 65536, f);
  fclose(f);
  strcpy((char *)buf, kc_filename);
  buf[17] = load & 0xff;
  buf[18] = (load >> 8) & 0xff;
  load += len + 1;
  buf[19] = load & 0xff;
  buf[20] = (load >> 8) & 0xff;
  if (autostart)
    {
      buf[16] = 3;
      buf[21] = start & 0xff;
      buf[22] = (start >> 8) & 0xff;
    }
  else
    {
      buf[16] = 2;
      buf[21] = 0xff;
      buf[22] = 0xff;
    }
  _kct_file.write(tape_filename,
		  buf, len + 128,
		  load, start,
		  KCT_TYPE_COM, KCT_MACHINE_ALL);

  update_tape_list();

  return TAPE_OK;
}

tape_error_t
Tape::rename(const char *from, const char *to)
{
  if (_kct_file.rename(from, to) != KCT_OK)
    return TAPE_ERROR;

  update_tape_list();
  return TAPE_OK;
}

tape_error_t
Tape::remove(const char *name)
{
  int idx;

  //cout << "Tape::remove(): [1] " << name << endl;

  // FIXME: using the index internal to the user interface
  // is kinda ugly
  if (name == NULL)
    {
      idx = TAPE_IF()->tapeGetSelected();
      name = TAPE_IF()->tapeGetName(idx);
      if (name == NULL)
        return TAPE_ERROR;
    }

  //cout << "Tape::remove(): [2] " << name << endl;

  _kct_file.remove(name);
  TAPE_IF()->tapeRemoveFile(idx);
  update_tape_list();

  return TAPE_OK;
}

tape_error_t
Tape::export_tap(const char *name, const char *filename)
{
  int ret;
  byte_t *buf;
  istream *is;
  unsigned int a;
  kct_file_props_t props;

  is = tape->read(name, &props);
  if (!is)
    return TAPE_ERROR;

  buf = new byte_t[props.size];
  for (a = 0;a < props.size;a++)
    buf[a] = is->get();
  delete is;

  ret = fileio_save_tap(filename, buf, props.size);
  delete[] buf;

  if (ret < 0)
    return TAPE_ERROR;

  return TAPE_OK;
}

tape_error_t
Tape::export_wav(const char *name, const char *filename)
{
  int ret;
  byte_t *buf;
  istream *is;
  unsigned int a;
  kct_file_props_t props;

  is = tape->read(name, &props);
  if (!is)
    return TAPE_ERROR;

  buf = new byte_t[props.size];
  for (a = 0;a < props.size;a++)
    buf[a] = is->get();
  delete is;

  ret = fileio_save_wav(filename, buf, props.size);
  delete[] buf;

  if (ret < 0)
    return TAPE_ERROR;

  return TAPE_OK;
}

istream *
Tape::read(const char *name, kct_file_props_t *props)
{
  return _kct_file.read(name, props);
}
