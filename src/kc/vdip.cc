/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2008 Torsten Paul
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

#include <time.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/statvfs.h>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/pio.h"
#include "kc/z80.h"
#include "kc/vdip.h"

#include "cmd/cmd.h"

class CMD_vdip_attach : public CMD
{
private:
  VDIP *_vdip;
  static const char * _path;

protected:
  int get_slot_no(CMD_Args *args)
  {
    int n = 0;

    if (args && args->has_arg("slot"))
      n = args->get_int_arg("slot");

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

VDIP_CMD::VDIP_CMD(VDIP *vdip, bool check_disk)
{
  _vdip = vdip;
  _args = NULL;
  _check_disk = check_disk;
}

VDIP_CMD::~VDIP_CMD(void)
{
  if (_args)
    delete _args;
}

VDIP *
VDIP_CMD::get_vdip(void)
{
  return _vdip;
}

void
VDIP_CMD::add_prompt(void)
{
  add_string(vdip->is_short_command_set() ? ">\r" : "D:\\>\r");
}

void
VDIP_CMD::add_error(vdip_error_t error)
{
  switch (error)
    {
    case ERR_BAD_COMMAND:
      add_string(get_vdip()->is_short_command_set() ? "BC\r" : "Bad Command\r");
      break;
    case ERR_COMMAND_FAILED:
      add_string(get_vdip()->is_short_command_set() ? "CF\r" : "Command Failed\r");
      break;
    case ERR_DISK_FULL:
      add_string(get_vdip()->is_short_command_set() ? "DF\r" : "Disk Full\r");
      break;
    case ERR_INVALID:
      add_string(get_vdip()->is_short_command_set() ? "FI\r" : "Invalid\r");
      break;
    case ERR_READ_ONLY:
      add_string(get_vdip()->is_short_command_set() ? "RO\r" : "Read Only\r");
      break;
    case ERR_FILE_OPEN:
      add_string(get_vdip()->is_short_command_set() ? "FO\r" : "File Open\r");
      break;
    case ERR_DIR_NOT_EMPTY:
      add_string(get_vdip()->is_short_command_set() ? "NE\r" : "Dir Not Empty\r");
      break;
    case ERR_FILENAME_INVALID:
      add_string(get_vdip()->is_short_command_set() ? "FN\r" : "Filename Invalid\r");
      break;
    case ERR_NO_UPGRADE:
      add_string(get_vdip()->is_short_command_set() ? "NU\r" : "No Upgrade\r");
      break;
    case ERR_NO_DISK:
      add_string(get_vdip()->is_short_command_set() ? "ND\r" : "No Disk\r");
      break;
    }
}

void
VDIP_CMD::add_char(int c)
{
  _response += c;
}

void
VDIP_CMD::add_hex(int digits, dword_t val)
{
  char buf[128];
  snprintf(buf, sizeof(buf), "%0*lx", digits, val);
  add_string(buf);
}

void
VDIP_CMD::add_word(word_t val)
{
  add_char(val & 0xff);
  add_char((val >> 8) & 0xff);
}

void
VDIP_CMD::add_dword(dword_t val)
{
  for (int a = 0;a <= 24;a += 8)
    add_char((val >> a) & 0xff);
}

void
VDIP_CMD::add_string(const char *text)
{
  _response += string(text);
}

bool
VDIP_CMD::has_args(void)
{
  return (_args != NULL) && (_args->size() > 0);
}

string
VDIP_CMD::get_arg(unsigned int arg)
{
  if (!has_args())
    return "";

  if ((arg < 0) || (arg >= _args->size()))
    return "";

  unsigned int a = 0;
  for (StringList::iterator it = _args->begin();it != _args->end();it++)
    if (a++ == arg)
      return *it;

  return "";
}

dword_t
VDIP_CMD::get_dword_arg(unsigned int arg)
{
  string data = get_arg(arg);
  if (data.length() != 4)
    return 0;

  dword_t val = 0;
  for (int a = 0; a < 4; a++)
    val = (val << 8) | (data.at(a) & 0xff);

  return val;
}

void
VDIP_CMD::set_args(StringList *args)
{
  _args = args;
}

string
VDIP_CMD::get_response(string input)
{
  return _response;
}

void
VDIP_CMD::exec(void)
{
  if (!get_vdip()->has_disk())
    add_error(ERR_NO_DISK);
  else
    execute();
}

void
VDIP_CMD::handle_input(byte_t data)
{
}

class VDIP_CMD_UNKNOWN : public VDIP_CMD
{
public:
  VDIP_CMD_UNKNOWN(VDIP *vdip) : VDIP_CMD(vdip) {}
  virtual ~VDIP_CMD_UNKNOWN(void) {}

  void execute(void) { add_error(ERR_BAD_COMMAND); }
};

class VDIP_CMD_DIR : public VDIP_CMD
{
public:
  VDIP_CMD_DIR(VDIP *vdip) : VDIP_CMD(vdip, true) { }
  virtual ~VDIP_CMD_DIR(void) { }

  void execute(void)
  {
    if (has_args())
      execute_with_arg();
    else
      execute_without_arg();
  }

  void execute_with_arg()
  {
    struct stat buf;
    const char *arg = get_arg(0).c_str();
    printf("DIR FOR FILE: '%s'\n", arg);
    string filename = get_vdip()->get_path(arg);
    if (stat(filename.c_str(), &buf) == 0)
      {
        add_string("\r");
        add_string(arg);
        add_char(' ');
        add_dword(buf.st_size);
        add_char('\r');
        add_prompt();
      }
    else
      {
        // ml-dos expects FI error in UGET
        add_error(ERR_INVALID);
        //add_error(ERR_COMMAND_FAILED);
      }
  }

  void execute_without_arg()
  {
    string path = get_vdip()->get_cwd();
    printf("DIR (cwd = '%s')\n", path.c_str());
    DIR *dir = opendir(path.c_str());
    add_string("\r");
    if (dir != NULL)
      {
        if (!get_vdip()->is_root())
          add_string(". DIR\r.. DIR\r");
        
        while (242)
          {
            struct dirent *dirent = readdir(dir);
            if (dirent == NULL)
              break;

            switch (dirent->d_type)
              {
              case DT_REG:
                add_string(dirent->d_name);
                add_string("\r");
                break;
              case DT_DIR:
                if ((strcmp(dirent->d_name, ".") != 0) && (strcmp(dirent->d_name, "..") != 0))
                  {
                    add_string(dirent->d_name);
                    add_string(" DIR\r");
                  }
                break;
              default:
                break;
              }
          }
        add_prompt();
      }
  }
};

class VDIP_CMD_CD : public VDIP_CMD
{
public:
  VDIP_CMD_CD(VDIP *vdip) : VDIP_CMD(vdip, true) { }
  virtual ~VDIP_CMD_CD(void) { }

  void execute(void)
  {
    string dir = get_arg(0);
    printf("CD %s\n", dir.c_str());
    if (strcmp(dir.c_str(), ".") == 0)
      {
        add_prompt();
      }
    else if (strcmp(dir.c_str(), "..") == 0)
      {
        vdip->chdir_up();
        add_prompt();
      }
    else
      {
        struct stat buf;
        string path = get_vdip()->get_path(dir);
        if (stat(path.c_str(), &buf) == 0)
          {
            get_vdip()->chdir(dir);
            add_prompt();
          }
        else
          {
            add_error(ERR_COMMAND_FAILED);
          }
      }
  }
};

class VDIP_CMD_IDD : public VDIP_CMD
{
public:
  VDIP_CMD_IDD(VDIP *vdip) : VDIP_CMD(vdip) { }
  virtual ~VDIP_CMD_IDD(void) { }

  void execute(void)
  {
    struct statvfs buf;

    word_t block_size = 0;
    dword_t free_bytes = 0;
    dword_t total_bytes = 0;
    if (statvfs(get_vdip()->get_cwd().c_str(), &buf) == 0)
      {
        unsigned long long free = (unsigned long long) buf.f_bavail * buf.f_bsize;
        unsigned long long total = (unsigned long long) buf.f_blocks * buf.f_frsize;
        free_bytes = free > 0xffffffffUL ? 0xffffffffUL : free;
        total_bytes = total > 0xffffffffUL ? 0xffffffffUL : total;
        block_size = buf.f_bsize;
      }
    add_string("\rUSB VID = $0000\rUSB PID = $0000\rVendor Id = KCemu   \rProduct Id = Virtual Disk    \rRevision Level = 0000\r");
    add_string("I/F = SCSI\rFAT32\rBytes/Sector = $");
    add_hex(4, 0x0200);
    add_string("\rBytes/Cluster = $");
    add_hex(6, block_size);
    add_string("\rCapacity = $");
    add_hex(8, total_bytes);
    add_string(" Bytes\rFree Space = $");
    add_hex(8, free_bytes);
    add_string(" Bytes\r\r");
    add_prompt();
    }
};

class VDIP_CMD_CLF : public VDIP_CMD
{
public:
  VDIP_CMD_CLF(VDIP *vdip) : VDIP_CMD(vdip, true) { }
  virtual ~VDIP_CMD_CLF(void) { }

  void execute(void)
  {
    printf("CLOSE FILE: '%s'\n", get_arg(0).c_str());
    if (vdip->get_file() == NULL)
      {
        add_error(ERR_COMMAND_FAILED);
      }
    else
      {
        fclose(get_vdip()->get_file());
        add_prompt();
        get_vdip()->set_file(NULL);
      }
    }
};

class VDIP_CMD_OPR : public VDIP_CMD
{
public:
  VDIP_CMD_OPR(VDIP *vdip) : VDIP_CMD(vdip, true) { }
  virtual ~VDIP_CMD_OPR(void) { }

  void execute(void)
  {
    string filename = get_vdip()->get_path(get_arg(0));
    printf("OPEN FOR READ: %s\n", filename.c_str());

    if (get_vdip()->get_file() != NULL)
      fclose(get_vdip()->get_file());

    FILE *f = fopen(filename.c_str(), "rb");
    if (f == NULL)
      {
        // ml-dos expects FI error in UPUT
        add_error(ERR_INVALID);
        //add_error(ERR_COMMAND_FAILED);
      }
    else
      {
        get_vdip()->set_file(f);
        add_prompt();
      }
  }
};

class VDIP_CMD_RDF : public VDIP_CMD
{
public:
  VDIP_CMD_RDF(VDIP *vdip) : VDIP_CMD(vdip, true) { }
  virtual ~VDIP_CMD_RDF(void) { }

  void execute(void)
  {
    FILE *f = get_vdip()->get_file();
    if (f == NULL)
      {
        add_error(ERR_INVALID);
      }
    else if (feof(f))
      {
        add_error(ERR_COMMAND_FAILED);
      }
    else
      {
        dword_t len = get_dword_arg(0);
        printf("READ FROM FILE (%ld bytes)\n", len);
        for (dword_t a = 0; a < len; a++)
          add_char(fgetc(f)); // use EOF as padding
        add_prompt();
      }
  }
};

class VDIP_CMD_SCS : public VDIP_CMD
{
public:
  VDIP_CMD_SCS(VDIP *vdip) : VDIP_CMD(vdip) { }
  virtual ~VDIP_CMD_SCS(void) { }

  void execute(void)
  {
    get_vdip()->set_short_command_set(true);
    add_prompt();
  }
};

class VDIP_CMD_ECS : public VDIP_CMD
{
public:
  VDIP_CMD_ECS(VDIP *vdip) : VDIP_CMD(vdip) { }
  virtual ~VDIP_CMD_ECS(void) { }

  void execute(void)
  {
    get_vdip()->set_short_command_set(false);
    add_prompt();
  }
};

class VDIP_CMD_OPW : public VDIP_CMD
{
public:
  VDIP_CMD_OPW(VDIP *vdip) : VDIP_CMD(vdip, true) { }
  virtual ~VDIP_CMD_OPW(void) { }

  void execute(void)
  {
    string filename = get_vdip()->get_path(get_arg(0));
    printf("OPEN FOR WRITE: '%s'\n", filename.c_str());

    if (get_vdip()->get_file() != NULL)
      fclose(get_vdip()->get_file());

    struct stat buf;
    bool stat_ok = (stat(filename.c_str(), &buf) == 0);
    if (stat_ok && (!S_ISREG(buf.st_mode)))
      {
        add_error(ERR_INVALID); // existing but not a regular file
      }
    else if (stat_ok && (access(filename.c_str(), W_OK) != 0))
      {
        add_error(ERR_READ_ONLY); // existing regular file but read-only
      }
    else
      {
        FILE *f = fopen(filename.c_str(), stat_ok ? "r+b" : "w+b");
        if (f == NULL)
          {
            add_error(ERR_COMMAND_FAILED);
          }
        else
          {
            if (fseek(f, 0, SEEK_END) == 0)
              {
                get_vdip()->set_file(f);
                add_prompt();
              }
            else
              {
                add_error(ERR_COMMAND_FAILED);
              }
          }
      }
  }
};

class VDIP_CMD_WRF : public VDIP_CMD
{
private:
  dword_t _wrf_len;

public:
  VDIP_CMD_WRF(VDIP *vdip) : VDIP_CMD(vdip, true) { }
  virtual ~VDIP_CMD_WRF(void) { }

  void execute(void)
  {
    FILE *f = get_vdip()->get_file();
    if (f == NULL)
      {
        add_error(ERR_FILE_OPEN);
      }
    else
      {
        dword_t len = get_dword_arg(0);
        printf("WRITE TO FILE (%ld bytes)\n", len);
        _wrf_len = len;
        // no prompt here! waiting for data
      }
    }

  void handle_input(byte_t data)
  {
    FILE *f = get_vdip()->get_file();
    
    if (f)
        fputc(data, f);

    _wrf_len--;
    if (_wrf_len == 0)
      add_prompt();
  }
};

class VDIP_CMD_SEK : public VDIP_CMD
{
public:
  VDIP_CMD_SEK(VDIP *vdip) : VDIP_CMD(vdip, true) { }
  virtual ~VDIP_CMD_SEK(void) { }

  void execute(void)
  {
    FILE *f = get_vdip()->get_file();
    if (f == NULL)
      {
        add_error(ERR_INVALID);
      }
    else if (feof(f))
      {
        add_error(ERR_COMMAND_FAILED);
      }
    else
      {
        dword_t offset = get_dword_arg(0);
        printf("SEEK IN FILE (to offset %lu)\n", offset);
        if ((fseek(f, offset, SEEK_SET) != 0) || ((unsigned int)ftell(f) != offset))
          add_error(ERR_COMMAND_FAILED);
        else
          add_prompt();
      }
  }
};

class VDIP_CMD_EMPTY : public VDIP_CMD
{
public:
  VDIP_CMD_EMPTY(VDIP *vdip) : VDIP_CMD(vdip, true) { }
  virtual ~VDIP_CMD_EMPTY(void) { }

  void execute(void)
  {
    add_prompt();
  }
};

class VDIP_CMD_SYNC : public VDIP_CMD
{
private:
  byte_t _val;
public:
  VDIP_CMD_SYNC(VDIP *vdip, byte_t val) : VDIP_CMD(vdip), _val(val) { }
  virtual ~VDIP_CMD_SYNC(void) { }

  void execute(void)
  {
    add_char(_val);
    add_char('\r');
  }
};

class VDIP_CMD_IPH : public VDIP_CMD
{
private:
  byte_t _val;
public:
  VDIP_CMD_IPH(VDIP *vdip) : VDIP_CMD(vdip) { }
  virtual ~VDIP_CMD_IPH(void) { }

  void execute(void)
  {
    get_vdip()->set_binary_mode(true);
    add_prompt();
  }
};

class VDIP_CMD_IPA : public VDIP_CMD
{
private:
  byte_t _val;
public:
  VDIP_CMD_IPA(VDIP *vdip) : VDIP_CMD(vdip) { }
  virtual ~VDIP_CMD_IPA(void) { }

  void execute(void)
  {
    get_vdip()->set_binary_mode(false);
    add_prompt();
  }
};

class VDIP_CMD_DIRT : public VDIP_CMD
{
private:
  byte_t _val;
public:
  VDIP_CMD_DIRT(VDIP *vdip) : VDIP_CMD(vdip, true) { }
  virtual ~VDIP_CMD_DIRT(void) { }

  dword_t get_datetime(timespec datetime)
  {
    struct tm buf;
    localtime_r((time_t *)&datetime, &buf);
    //printf("%04d-%02d-%02d %02d:%02d:%02d\n", buf.tm_year + 1900, buf.tm_mon + 1, buf.tm_mday, buf.tm_hour, buf.tm_min, buf.tm_sec);
    return ((buf.tm_year - 80) << 25) | ((buf.tm_mon + 1) << 21) | (buf.tm_mday << 16) | (buf.tm_hour << 11) | (buf.tm_min << 5) | (buf.tm_sec / 2);
  }
  
  void execute(void)
  {
    struct stat buf;
    const char *arg = get_arg(0).c_str();
    printf("DIR TIMES FOR FILE: '%s'\n", arg);
    string filename = get_vdip()->get_path(arg);
    if (stat(filename.c_str(), &buf) == 0)
      {
        // as we have no creation time, we use the modification time
        dword_t ctime = get_datetime(buf.st_mtim);
        word_t atime = get_datetime(buf.st_atim) >> 16;
        dword_t mtime = get_datetime(buf.st_mtim);

        add_string("\r");
        add_string(arg);
        add_char(' ');
        add_dword(ctime);
        add_word(atime);
        add_dword(mtime);
        add_char('\r');
        add_prompt();
      }
    else
      {
        add_error(ERR_COMMAND_FAILED);
      }
  }
};

/*
class VDIP_CMD_CD : public VDIP_CMD
{
public:
  VDIP_CMD_CD(VDIP *vdip) : VDIP_CMD(vdip) { }
  virtual ~VDIP_CMD_CD(void) { }

  void execute(void)
  {
  }
};
*/

VDIP::VDIP(void) : Callback("Vinculum USB")
{
  _pio = NULL;
  _file = NULL;
  _root = "";
  _cwd = new StringList();
  _attach_cmd = new CMD_vdip_attach(this);
  reset();
}

VDIP::~VDIP(void)
{
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
  _root = root;

  if (_pio)
    {
      if (has_disk())
        _output_buffer = is_short_command_set() ? "DD1\r" : "Device Detected P1\r";
      else
        _output_buffer = is_short_command_set() ? "DR1\r" : "Device Removed P1\r";

      set_pio_ext_b(0x02);
      z80->addCallback(20000, this, NULL);
    }

  CMD_Args *args = new CMD_Args();
  args->set_int_arg("slot", 0);
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

  _input_buffer = "";

  _input = false;
  _output = -1;
  _output_buffer = "\rVer 03.60VDAPF On-Line:\r";

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
  printf("%Ld: read_byte -> %02x ('%c')\n", z80->getCounter(), a & 0xff, isprint(a) ? a : '.');
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
  //printf("%Ld: latch_byte -> %02x ('%c')\n", z80->getCounter(), _output, isprint(_output) ? _output : '.');
}

void
VDIP::read_end(void)
{
  if (_reset)
    return;

  //printf("%Ld: read_end, buffer size = %d\n", z80->getCounter(), _output_buffer.length());

  if (_output_buffer.empty())
    set_pio_ext_b(0x01);
  else
    z80->addCallback(50, this, NULL);
}

void
VDIP::write_byte(byte_t val)
{
  if (_reset)
    return;

  _input_data = val;
  //printf("VDIP: write: %02xh / '%c'\n", val, isprint(val) ? val : '.');
}

vdip_command_t
VDIP::map_extended_command(string cmd)
{
  if (strcmp(cmd.c_str(), "DIR") == 0)
    return CMD_DIR;
  else if (strcmp(cmd.c_str(), "CD") == 0)
    return CMD_CD;
  else if (strcmp(cmd.c_str(), "IDD") == 0)
    return CMD_IDD;
  else if (strcmp(cmd.c_str(), "CLF") == 0)
    return CMD_CLF;
  else if (strcmp(cmd.c_str(), "OPR") == 0)
    return CMD_OPR;
  else if (strcmp(cmd.c_str(), "RDF") == 0)
    return CMD_RDF;
  else if (strcmp(cmd.c_str(), "SCS") == 0)
    return CMD_SCS;
  else if (strcmp(cmd.c_str(), "ECS") == 0)
    return CMD_ECS;
  else if (strcmp(cmd.c_str(), "OPW") == 0)
    return CMD_OPW;
  else if (strcmp(cmd.c_str(), "WRF") == 0)
    return CMD_WRF;
  else if (strcmp(cmd.c_str(), "SEK") == 0)
    return CMD_SEK;
  else if (strcmp(cmd.c_str(), "IPH") == 0)
    return CMD_IPH;
  else if (strcmp(cmd.c_str(), "IPA") == 0)
    return CMD_IPA;
  else if (strcmp(cmd.c_str(), "DIRT") == 0)
    return CMD_DIRT;

  return CMD_UNKNOWN;
}

VDIP_CMD *
VDIP::decode_command(string buf)
{
  if (buf.length() == 0)
    return new VDIP_CMD_EMPTY(this);

  StringList *list = new StringList(buf, ' ');
  if (list->size() == 0)
    return new VDIP_CMD_UNKNOWN(this);

  vdip_command_t code = CMD_UNKNOWN;

  string cmd = list->front();
  if (cmd.length() == 1)
    code = (vdip_command_t)(cmd.at(0) & 0xff);
  else
    code = map_extended_command(list->front());

  VDIP_CMD *vdip_cmd;
  switch (code)
    {
    case CMD_SCS:
      vdip_cmd = new VDIP_CMD_SCS(this);
      break;
    case CMD_ECS:
      vdip_cmd = new VDIP_CMD_ECS(this);
      break;
    case CMD_DIR:
      vdip_cmd = new VDIP_CMD_DIR(this);
      break;
    case CMD_CD:
      vdip_cmd = new VDIP_CMD_CD(this);
      break;
    case CMD_CLF:
      vdip_cmd = new VDIP_CMD_CLF(this);
      break;
    case CMD_OPW:
      vdip_cmd = new VDIP_CMD_OPW(this);
      break;
    case CMD_OPR:
      vdip_cmd = new VDIP_CMD_OPR(this);
      break;
    case CMD_RDF:
      vdip_cmd = new VDIP_CMD_RDF(this);
      break;
    case CMD_WRF:
      vdip_cmd = new VDIP_CMD_WRF(this);
      break;
    case CMD_IDD:
      vdip_cmd = new VDIP_CMD_IDD(this);
      break;
    case CMD_SEK:
      vdip_cmd = new VDIP_CMD_SEK(this);
      break;
    case CMD_IPH:
      vdip_cmd = new VDIP_CMD_IPH(this);
      break;
    case CMD_IPA:
      vdip_cmd = new VDIP_CMD_IPA(this);
      break;
    case CMD_E:
      vdip_cmd = new VDIP_CMD_SYNC(this, 'E');
      break;
    case CMD_e:
      vdip_cmd = new VDIP_CMD_SYNC(this, 'e');
      break;
    case CMD_DIRT:
      vdip_cmd = new VDIP_CMD_DIRT(this);
      break;
    default:
      vdip_cmd = new VDIP_CMD_UNKNOWN(this);
      break;
    }

  list->pop_front();
  if (list->size() > 0)
    vdip_cmd->set_args(list);

  return vdip_cmd;
}

void
VDIP::write_end(void)
{
  if (_reset)
    return;

  //printf("%Ld: write_end\n", z80->getCounter());

  if (_cmd != NULL)
    {
      _cmd->handle_input(_input_data);
    }
  else if (_input_data != 0x0d)
    {
      _input_buffer += _input_data;
    }
  else
    {
      printf("input_string: ");
      for (unsigned int a = 0;a < _input_buffer.length();a++)
        printf("%c", isprint(_input_buffer.at(a)) ? _input_buffer.at(a) : '.');
      printf("\ninput_string: ");
      for (unsigned int a = 0;a < _input_buffer.length();a++)
        printf("%02x ", _input_buffer.at(a) & 0xff);
      printf("\n");

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
