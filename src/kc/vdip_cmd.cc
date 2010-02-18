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

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <utime.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/vdip.h"

#include "sys/sysdep.h"

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
    DIR *dir = opendir(path.c_str());
    add_string("\r");
    if (dir != NULL)
      {
        if (!get_vdip()->is_root())
          add_string(". DIR\r.. DIR\r");

        // hmm, can't use dirent->d_type as MinGW does not provide
        // this information :-(. falling back to stat every entry.
        while (242)
          {
            struct stat buf;
            struct dirent *dirent = readdir(dir);
            if (dirent == NULL)
              break;

            string file = path + "/" + dirent->d_name;
            if (stat(file.c_str(), &buf) != 0)
              continue;

            if (S_ISREG(buf.st_mode))
              {
                add_string(dirent->d_name);
                add_string("\r");
              }
            else if (S_ISDIR(buf.st_mode))
              {
                if ((strcmp(dirent->d_name, ".") != 0) && (strcmp(dirent->d_name, "..") != 0))
                  {
                    add_string(dirent->d_name);
                    add_string(" DIR\r");
                  }
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
    if (strcmp(dir.c_str(), ".") == 0)
      {
        add_prompt();
      }
    else if (strcmp(dir.c_str(), "..") == 0)
      {
        vdip->chdir_up();
        add_prompt();
      }
    else if (strncmp(dir.c_str(), "/", 1) == 0)
      {
        vdip->chdir_root();
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
    word_t block_size;
    dword_t free_bytes, total_bytes;

    if (sys_getdiskinfo(get_vdip()->get_cwd().c_str(), &total_bytes, &free_bytes, &block_size) != 0)
      {
        block_size = 0;
        free_bytes = 0;
        total_bytes = 0;
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

  dword_t get_datetime(long datetime)
  {
    int year, month, day, hour, minute, second;
    sys_converttime((long)datetime, &year, &month, &day, &hour, &minute, &second);
    return ((year - 80) << 25) | ((month) << 21) | (day << 16) | (hour << 11) | (minute << 5) | (second / 2);
  }
  
  void execute(void)
  {
    struct stat buf;
    const char *arg = get_arg(0).c_str();
    string filename = get_vdip()->get_path(arg);
    if (stat(filename.c_str(), &buf) == 0)
      {
        // as we have no creation time, we use the modification time
        dword_t ctime = get_datetime(buf.st_mtime);
        word_t atime = get_datetime(buf.st_atime) >> 16;
        dword_t mtime = get_datetime(buf.st_mtime);

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

class VDIP_CMD_FWV : public VDIP_CMD
{
public:
  VDIP_CMD_FWV(VDIP *vdip) : VDIP_CMD(vdip) { }
  virtual ~VDIP_CMD_FWV(void) { }

  void execute(void)
  {
    add_string("\rMAIN ");
    add_string(get_vdip()->get_firmware_version().c_str());
    add_string("\rRPRG 1.00R\r");
    add_prompt();
  }
};

class VDIP_CMD_MKD : public VDIP_CMD
{
public:
  VDIP_CMD_MKD(VDIP *vdip) : VDIP_CMD(vdip) { }
  virtual ~VDIP_CMD_MKD(void) { }

  void execute(void)
  {
    if (get_vdip()->get_file() != NULL)
      add_error(ERR_FILE_OPEN);
    else if (get_arg_count() == 1)
      execute_with_name(get_arg(0));
    else if (get_arg_count() >= 2)
      execute_with_name_and_time(get_arg(0), get_dword_arg(1));
    else
      add_error(ERR_BAD_COMMAND);
  }

  long get_time(dword_t time)
  {
    struct tm tm;

    tm.tm_year = ((time >> 25) & 127) + 80;
    tm.tm_mon = ((time >> 21) & 15) - 1;
    tm.tm_mday = ((time >> 16) & 31);
    tm.tm_hour = ((time >> 11) & 31);
    tm.tm_min = ((time >> 5) & 64);
    tm.tm_sec = (time & 31) * 2;

    return mktime(&tm);
  }

  void execute_with_name(string arg)
  {
    // default date: 2004-12-20 00:00:00
    // date given in the documentation is wrong!
    execute_with_name_and_time(arg, 0x31940000);
  }

  void execute_with_name_and_time(string arg, dword_t time)
  {
    struct stat buf;
    string filename = get_vdip()->get_path(arg);
    if (stat(filename.c_str(), &buf) != 0)
      {
        if (sys_mkdir(filename.c_str(), 0755) == 0)
          {
            struct utimbuf utimbuf;
            
            utimbuf.actime = get_time(time);
            if (utimbuf.actime != -1)
              {
                utimbuf.modtime = utimbuf.actime;
                utime(filename.c_str(), &utimbuf);
              }
            add_prompt();
            return;
          }
      }

    add_error(ERR_COMMAND_FAILED);
  }
};

class VDIP_CMD_DLD : public VDIP_CMD
{
public:
  VDIP_CMD_DLD(VDIP *vdip) : VDIP_CMD(vdip) { }
  virtual ~VDIP_CMD_DLD(void) { }

  void execute(void)
  {
    if (get_vdip()->get_file() != NULL)
      add_error(ERR_FILE_OPEN);
    else if (get_arg_count() >= 1)
      execute_with_name(get_arg(0));
    else
      add_error(ERR_BAD_COMMAND);
  }

  void execute_with_name(string arg)
  {
    string filename = get_vdip()->get_path(arg);
    if (rmdir(filename.c_str()) == 0)
      {
        add_prompt();
      }
    else
      {
        if (errno == ENOTEMPTY)
          add_error(ERR_DIR_NOT_EMPTY);
        else
          add_error(ERR_COMMAND_FAILED);
      }
  }
};

class VDIP_CMD_DLF : public VDIP_CMD
{
public:
  VDIP_CMD_DLF(VDIP *vdip) : VDIP_CMD(vdip) { }
  virtual ~VDIP_CMD_DLF(void) { }

  void execute(void)
  {
    if (get_vdip()->get_file() != NULL)
      add_error(ERR_FILE_OPEN);
    else if (get_arg_count() >= 1)
      execute_with_name(get_arg(0));
    else
      add_error(ERR_BAD_COMMAND);
  }

  void execute_with_name(string arg)
  {
    string filename = get_vdip()->get_path(arg);

    if (access(filename.c_str(), W_OK) != 0)
      {
        add_error(ERR_READ_ONLY);
      }
    else if (unlink(filename.c_str()) == 0)
      {
        add_prompt();
      }
    else
      {
        switch (errno)
          {
          case EISDIR:
            add_error(ERR_INVALID);
            break;
          case EACCES:
          case EPERM:
          case EROFS:
            add_error(ERR_READ_ONLY);
            break;
          default:
            add_error(ERR_COMMAND_FAILED);
            break;
          }
      }
  }
};

class VDIP_CMD_RD : public VDIP_CMD
{
public:
  VDIP_CMD_RD(VDIP *vdip) : VDIP_CMD(vdip) { }
  virtual ~VDIP_CMD_RD(void) { }

  void execute(void)
  {
    if (get_vdip()->get_file() != NULL)
      add_error(ERR_FILE_OPEN);
    else if (get_arg_count() >= 1)
      execute_with_name(get_arg(0));
    else
      add_error(ERR_BAD_COMMAND);
  }

  void execute_with_name(string arg)
  {
    string filename = get_vdip()->get_path(arg);

    struct stat buf;
    if (stat(filename.c_str(), &buf) != 0)
      {
        add_error(ERR_COMMAND_FAILED);
        return;
      }

    // this is not specified in the real VDIP firmware
    // but we refuse to read files that are bigger than
    // one megabyte.
    if (buf.st_size > 1024 * 1024)
      {
        add_error(ERR_INVALID);
        return;
      }

    FILE *f = fopen(filename.c_str(), "rb");
    if (f == NULL)
      {
        add_error(ERR_COMMAND_FAILED);
        return;
      }

    while (242)
      {
        int c = fgetc(f);
        if (c == EOF)
          break;
        add_char(c);
      }

    fclose(f);
  }
};

class VDIP_CMD_REN : public VDIP_CMD
{
public:
  VDIP_CMD_REN(VDIP *vdip) : VDIP_CMD(vdip) { }
  virtual ~VDIP_CMD_REN(void) { }

  void execute(void)
  {
    if (get_vdip()->get_file() != NULL)
      add_error(ERR_FILE_OPEN);
    else if (get_arg_count() >= 2)
      execute_with_names(get_arg(0), get_arg(1));
    else
      add_error(ERR_BAD_COMMAND);
  }

  void execute_with_names(string source, string target)
  {
    string source_path = get_vdip()->get_path(source);
    string target_path = get_vdip()->get_path(target);

    struct stat buf;
    if (stat(target_path.c_str(), &buf) == 0)
      {
        add_error(ERR_COMMAND_FAILED);
      }
    else if (rename(source_path.c_str(), target_path.c_str()) == 0)
      {
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
VDIP_CMD::has_args(void) const
{
  return get_arg_count() > 0;
}

int
VDIP_CMD::get_arg_count(void) const
{
  if (_args == NULL)
    return 0;

  return _args->size();
}

string
VDIP_CMD::get_arg(unsigned int arg) const
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
VDIP_CMD::get_dword_arg(unsigned int arg) const
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

vdip_command_t
VDIP_CMD::map_extended_command(string cmd)
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
  else if (strcmp(cmd.c_str(), "FWV") == 0)
    return CMD_FWV;
  else if (strcmp(cmd.c_str(), "MKD") == 0)
    return CMD_MKD;
  else if (strcmp(cmd.c_str(), "DLD") == 0)
    return CMD_DLD;
  else if (strcmp(cmd.c_str(), "DLF") == 0)
    return CMD_DLF;
  else if (strcmp(cmd.c_str(), "RD") == 0)
    return CMD_RD;
  else if (strcmp(cmd.c_str(), "REN") == 0)
    return CMD_REN;

  return CMD_UNKNOWN;
}

VDIP_CMD *
VDIP_CMD::create_command(VDIP *vdip, string cmd)
{
  return create_command(vdip, map_extended_command(cmd));
}

VDIP_CMD *
VDIP_CMD::create_command(VDIP *vdip, vdip_command_t code)
{
  VDIP_CMD *vdip_cmd;

  switch (code)
    {
    case CMD_SCS:
      vdip_cmd = new VDIP_CMD_SCS(vdip);
      break;
    case CMD_ECS:
      vdip_cmd = new VDIP_CMD_ECS(vdip);
      break;
    case CMD_DIR:
      vdip_cmd = new VDIP_CMD_DIR(vdip);
      break;
    case CMD_CD:
      vdip_cmd = new VDIP_CMD_CD(vdip);
      break;
    case CMD_CLF:
      vdip_cmd = new VDIP_CMD_CLF(vdip);
      break;
    case CMD_OPW:
      vdip_cmd = new VDIP_CMD_OPW(vdip);
      break;
    case CMD_OPR:
      vdip_cmd = new VDIP_CMD_OPR(vdip);
      break;
    case CMD_RDF:
      vdip_cmd = new VDIP_CMD_RDF(vdip);
      break;
    case CMD_WRF:
      vdip_cmd = new VDIP_CMD_WRF(vdip);
      break;
    case CMD_IDD:
      vdip_cmd = new VDIP_CMD_IDD(vdip);
      break;
    case CMD_SEK:
      vdip_cmd = new VDIP_CMD_SEK(vdip);
      break;
    case CMD_IPH:
      vdip_cmd = new VDIP_CMD_IPH(vdip);
      break;
    case CMD_IPA:
      vdip_cmd = new VDIP_CMD_IPA(vdip);
      break;
    case CMD_E:
      vdip_cmd = new VDIP_CMD_SYNC(vdip, 'E');
      break;
    case CMD_e:
      vdip_cmd = new VDIP_CMD_SYNC(vdip, 'e');
      break;
    case CMD_DIRT:
      vdip_cmd = new VDIP_CMD_DIRT(vdip);
      break;
    case CMD_EMPTY:
      vdip_cmd = new VDIP_CMD_EMPTY(vdip);
      break;
    case CMD_FWV:
      vdip_cmd = new VDIP_CMD_FWV(vdip);
      break;
    case CMD_MKD:
      vdip_cmd = new VDIP_CMD_MKD(vdip);
      break;
    case CMD_DLD:
      vdip_cmd = new VDIP_CMD_DLD(vdip);
      break;
    case CMD_DLF:
      vdip_cmd = new VDIP_CMD_DLF(vdip);
      break;
    case CMD_RD:
      vdip_cmd = new VDIP_CMD_RD(vdip);
      break;
    case CMD_REN:
      vdip_cmd = new VDIP_CMD_REN(vdip);
      break;
    default:
      vdip_cmd = new VDIP_CMD_UNKNOWN(vdip);
      break;
    }

  return vdip_cmd;
}
