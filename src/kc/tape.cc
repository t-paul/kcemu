/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: tape.cc,v 1.21 2001/12/29 03:50:21 torsten_paul Exp $
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

#include <stdio.h> /* FIXME */
#include <string.h>
#include <ctype.h>

#include <iostream.h>
#include <iomanip.h>

#include "kc/config.h"
#include "kc/system.h"

#include "kc/z80.h"
#include "kc/pio.h"
#include "kc/tape.h"
#include "kc/memory.h"

#include "cmd/cmd.h"

#include "fileio/load.h"

#include "ui/ui.h"
#include "ui/status.h"

#include "libdbg/dbg.h"

#define TAPE_IF() (ui->getTapeInterface())

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
      istream *is;
      const char *name, *shortname;
      char buf[100]; /* FIXME: */
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
	  
          if (memory->loadRAM(is, true))
            {
              shortname = strrchr(name, '/');
              if (shortname)
                shortname++;
              else
                shortname = name;
              sprintf(buf, _("File `%s' loaded."), shortname);
              Status::instance()->setMessage(buf);
              if (context == 1)
		{
		  z80->jump(props.start_addr);
		  TAPE_IF()->tapeNext();
		}
            }
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
      char buf[100]; /* FIXME: */
      tape_error_t err;
      const char *filename, *shortname;

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
              shortname = strrchr(filename, '/');
              if (shortname)
                shortname++;
              else
                shortname = filename;
              sprintf(buf, _("tape-archive `%s' attached."), shortname);
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

class CMD_tape_export : public CMD
{
private:
  Tape *_t;
public:
  CMD_tape_export(Tape *t) : CMD("tape-export")
    {
      _t = t;
      register_cmd("tape-export");
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
              args->add_callback("ui-file-select-CB-ok", this, 1);
              args->add_callback("ui-file-select-CB-cancel", this, 1);
              CMD_EXEC_ARGS("ui-file-select", args);
              return;
            }
          break;
        case 1:
          if (args)
            filename = args->get_string_arg("filename");
          break;
        }
      if (filename)
        {
          if (tape->extract(tapename, filename))
            {
              sprintf(buf, _("File `%s' saved."), filename);
              Status::instance()->setMessage(buf);
            }
          else
            Status::instance()->setMessage(_("Can't export file."));
        }
    }
};

class CMD_tape_add_file : public CMD
{
private:
  Tape *_t;
public:
  CMD_tape_add_file(Tape *t) : CMD("tape-add-file")
    {
      _t = t;
      register_cmd("tape-add-file", 0);
    }

  void execute(CMD_Args *args, CMD_Context context)
    {
      CMD_Args *a;
      const char *filename;

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
	      a->add_callback("ui-file-select-CB-ok", this, 1);
	      CMD_EXEC_ARGS("ui-file-select", a);
	      return;
	    }
	  /* fall through */

        case 1:
          if (!filename)
            return;

	  _t->add(filename);
        }
    }
};

//      unsigned int load, start, autostart;
//      const char *filename, *tapename, *kcname;

//	  case 1:
//	    /* FIXME: may use given args, but we'd need to
//	       unregister the previous callback then! */
//	    a = new CMD_Args();
//	    a->set_string_arg("filename", filename);
//	    a->add_callback("ui-edit-header-CB", this, 2);
//	    CMD_EXEC_ARGS("ui-edit-header", a);
//	    return;
//
//	  case 2:
//	    if (!args->has_arg("filename")) return;
//	    if (!args->has_arg("tape-filename")) return;
//	    if (!args->has_arg("kc-filename")) return;
//	    if (!args->has_arg("load-address")) return;
//	    
//	    
//	    filename  = args->get_string_arg("filename");
//	    tapename  = args->get_string_arg("tape-filename");
//	    kcname    = args->get_string_arg("kc-filename");
//	    load      = args->get_int_arg("load-address");
//	    autostart = args->get_int_arg("autostart");
//	    if (autostart)
//	      start = args->get_int_arg("start-address");
//	    
//	    cout.form("filename: %s\n",  args->get_string_arg("filename"));
//	    cout.form("tapename: %s\n",  args->get_string_arg("tape-filename"));
//	    cout.form("kcname:   %s\n",  args->get_string_arg("kc-filename"));
//	    cout.form("loadaddr: %x\n",  args->get_int_arg("load-address"));
//	    cout.form("startaddr: %x\n", args->get_int_arg("start-address"));
//	    cout.form("autostart: %d\n", args->get_int_arg("autostart"));
//
//	    _t->add_raw(filename, tapename, kcname, load, start, autostart);
//	    break;
//	  case 3:
//	    a = new CMD_Args();
//	    a->set_string_arg("ui-file-select-title",
//			      _("Select memory image..."));
//	    a->add_callback("ui-file-select-CB-ok", this, 4);
//	    a->add_callback("ui-file-select-CB-cancel", this, 4);
//	    CMD_EXEC_ARGS("ui-file-select", a);
//	    return;
//	  case 4:
//	    filename = args->get_string_arg("filename");
//	    if (!filename)
//	      return;
//	    _t->add(filename);
//	    break;
//

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
	    delay = args->get_int_arg("tape-play-delay");

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

    CMD *cmd;
    cmd = new CMD_tape_load(this);
    cmd = new CMD_tape_export(this);
    cmd = new CMD_tape_attach(this);
    cmd = new CMD_tape_add_file(this);
    cmd = new CMD_tape_play(this);
}

Tape::~Tape()
{
}

void
Tape::power(bool val)
{
  _power = val;

  DBG(1, form("KCemu/Tape/power",
	      "Tape::power(): Power %s\n",
	      _power ? "on" : "off"));

  if (_power)
    {
      TAPE_IF()->tapePower(true);
    }
  else
    {
      do_stop();
      TAPE_IF()->tapePower(false);
    }

#if 0
  ofstream os;

  _bits = -1;
  _sync = 2;
  _block = 0;
  _last_val = 0;
  _last_block = 0;
  _init = 0;

  z80->addCallback(BIT_1, this, (void *)1);
  z80->addCallback(2 * BIT_1, this, (void *)0);

  if (_record && (_os != NULL))
    {
      cout << "Tape::stop(): writing output" << endl;
      //_os.close();
      os.open("/tmp/kcemu.output", ios::out | ios::bin);
      if (!os)
        cout << "Tape::stop(): can't open output file" << endl;
      else
        {
          _os->freeze(1);
          os.write(_os->str(), _os->pcount());
          os.close();
          _os->freeze(0);
        }
      delete _os;
      _os = NULL;
    }
  else
    {
      if (_is != NULL)
        {
          delete _is;
          _is = NULL;
        }
    }
  _record = false;
#endif
}

void
Tape::record(void)
{
  cout << "Tape::record()\n";
  _record = true;
  _old_bit = -1;
  _sync = 2;
  _sync_count = 200;
  _flip_flop = 0;
  _os = new ostrstream;
  _byte_counter = 0;
  _state = 0;
  
  _play = false;
  _record = true;
  power(true);
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
      offset = getDelay(delay);
      DBG(1, form("KCemu/Tape/play",
		  "Tape::play(): trigger offset is %ld\n",
		  offset));
      z80->addCallback(offset, this, (void *)2);
    }
}

void
Tape::stop(void)
{
  byte_t *ptr;
  ofstream os;
  int load, start;
  kct_file_type_t type;

  _play = false;
  _record = false;
  power(false);

  if (!_os)
    return;
  if (_os->pcount() == 0)
    return;

  os.open("/tmp/kcemu.output", ios::out | ios::binary);
  if (!os)
    cout << "Tape::stop(): can't open output file" << endl;
  else
    {
      _os->freeze(1);
      ptr = (byte_t *)_os->str();
      DBG(1, form("KCemu/Tape/write",
		  "Tape::stop(): writing output file (%d bytes)\n",
		  _os->pcount()));
      os.write((char *)ptr, _os->pcount());

      load = 0;
      start = 0;
      if ((ptr[1] == 0xd3) && (ptr[2] == 0xd3) && (ptr[3] == 0xd3))
	{
	  type = KCT_TYPE_BAS;
	}
      else if ((ptr[1] == 0xd4) && (ptr[2] == 0xd4) && (ptr[3] == 0xd4))
	{
	  type = KCT_TYPE_MINTEX;
	}
      else if ((ptr[1] == 0xd7) && (ptr[2] == 0xd7) && (ptr[3] == 0xd7))
	{
	  type = KCT_TYPE_BAS_P;
	}
      else
	{
	  type = KCT_TYPE_COM;
	  load = ptr[18] | (ptr[19] << 8);
	  start = ptr[22] | (ptr[23] << 8);
	}

      DBG(1, form("KCemu/Tape/write",
		  "Tape::stop(): type = %s, load = %04xh, start = %04xh\n",
		  _kct_file.type_name(type), load, start));
      _kct_file.write((const char *)"new file",
                      ptr, _os->pcount(), load, start,
                      type, KCT_MACHINE_ALL);

      update_tape_list();
      os.close();
      _os->freeze(0);
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
Tape::getDelay(int seconds)
{
  return 600 * BIT_S * seconds;
}

void
Tape::callback(void *data)
{
  int edge = (int)data;
  
//  if (!_power)
//    {
//	DBG(2, form("KCemu/Tape/play",
//		    "Tape::callback(): tape callback while power is off!\n"));
//	return;
//    }

  do_play(edge);
}

void
Tape::do_stop(void)
{
  TAPE_IF()->tapeProgress(0);
}

void
Tape::do_play(int edge)
{
  pio->strobe_A();

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
       *  this comes from the automatic playing se we
       *  ignore this if power is off
       */
      DBG(1, form("KCemu/Tape/play",
		  "Tape::play(): got first signal (power = %d)\n", _power));
      if (!_power)
	return;
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

      _bytes_read = _is->tellg();
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
      _is->read((char *)_buf, 129);
      _block = _buf[0];
      TAPE_IF()->tapeProgress((100 * _bytes_read) / _file_size);

      if (_block > _start_block)
        _init = 200;
      else
        _init = 4000;
      
      if ((_is->gcount() != 129) || (_is->peek() == EOF))
        {
          /*
           *  last block
           */
          DBG(1, form("KCemu/Tape/play",
		      "Tape::play(): gcount() = %d, peek() = %d\n",
		      _is->gcount(), _is->peek()));

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
Tape::ctcSignal(void)
{
  int bit_type;
  unsigned long diff;
  static unsigned long long c_old, c_new;

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

  if (diff < ((BIT_0 + BIT_1) / 2))
    bit_type = 0;
  else if (diff > ((BIT_S + BIT_1) / 2))
    bit_type = 2;
  else
    bit_type = 1;

  if (!_power) return;

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
  
  _old_bit = bit_type;
}

tape_error_t
Tape::attach(const char *filename, bool create)
{
  KCTDir *dir;
  CMD_Args *args;
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
      switch (prop->type)
        {
        case FILEIO_TYPE_COM:
          DBG(0, form("KCemu/Tape/add",
                      "Tape::add(): '%s' %04x/%04x [COM]\n",
                      (const char *)&ptr->name[0],
                      ptr->load_addr, ptr->start_addr));
          _kct_file.write((const char *)&ptr->name[0],
                          ptr->data, ptr->size,
                          ptr->load_addr, ptr->start_addr,
                          KCT_TYPE_COM, KCT_MACHINE_ALL);
          break;
        case FILEIO_TYPE_BAS:
          DBG(0, form("KCemu/Tape/add",
                      "Tape::add(): '%s' [BAS]\n",
                      (const char *)&ptr->name[0]));
          _kct_file.write((const char *)&ptr->name[0],
                          ptr->data, ptr->size,
                          ptr->load_addr, ptr->start_addr,
                          KCT_TYPE_BAS, KCT_MACHINE_ALL);
          break;
        case FILEIO_TYPE_MINTEX:
          DBG(0, form("KCemu/Tape/add",
                      "Tape::add(): '%s' [MINTEX]\n",
                      (const char *)&ptr->name[0]));
          _kct_file.write((const char *)&ptr->name[0],
                          ptr->data, ptr->size,
                          ptr->load_addr, ptr->start_addr,
                          KCT_TYPE_MINTEX, KCT_MACHINE_ALL);
          break;
        case FILEIO_TYPE_PROT_BAS:
          DBG(0, form("KCemu/Tape/add",
                      "Tape::add(): '%s' [BAS_P]\n",
                      (const char *)&ptr->name[0]));
          _kct_file.write((const char *)&ptr->name[0],
                          ptr->data, ptr->size,
                          ptr->load_addr, ptr->start_addr,
                          KCT_TYPE_BAS_P, KCT_MACHINE_ALL);
          break;
        default:
          DBG(0, form("KCemu/Tape/add",
                      "Tape::add(): '%s' %04x/%04x [default]\n",
                      (const char *)&ptr->name[0],
                      ptr->load_addr, ptr->start_addr));
          _kct_file.write((const char *)&ptr->name[0], ptr->data, ptr->size);
          break;
        }
    }
  update_tape_list();
  fileio_free_prop(&prop);

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
Tape::remove(const char *name)
{
  int idx;

  cout << "Tape::remove(): [1] " << name << endl;

  // FIXME: using the index internal to the user interface
  // is kinda ugly
  idx = TAPE_IF()->tapeGetSelected();
  if (name == NULL)
    {
      if (name == NULL)
        name = TAPE_IF()->tapeGetName(idx);
      if (name == NULL)
        return TAPE_ERROR;
    }

  cout << "Tape::remove(): [2] " << name << endl;

  _kct_file.remove(name);
  TAPE_IF()->tapeRemoveFile(idx);

  return TAPE_OK;
}

tape_error_t
Tape::extract(const char *name, const char *filename)
{
  int a, c;
  fstream f;
  istream *is;
  kct_file_props_t props;

  is = tape->read(name, &props);
  if (!is)
    return TAPE_ERROR;

  f.open(filename, ios::out | ios::binary);
  if (!f)
    return TAPE_ERROR;

  f.write("\xc3KC-TAPE by AF. ", 16);
  a = 0;
  while (242)
    {
      c = is->get();
      if (c == EOF)
	break;
      a++;
      f.put(c);
    }
  while ((a % 129) != 0)
    {
      a++;
      f.put('\0'); /* pad to block size */
    }
  
  f.close();
  return TAPE_OK;
}

istream *
Tape::read(const char *name, kct_file_props_t *props)
{
  return _kct_file.read(name, props);
}
