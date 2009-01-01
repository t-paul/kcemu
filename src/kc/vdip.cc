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

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/pio.h"
#include "kc/z80.h"
#include "kc/vdip.h"

VDIP::VDIP(void) : Callback("Vinculum USB")
{
  _cbval = 0;
  _pio = NULL;
  _input = true;
  _reset = true;
  _output = -1;
  _pio_ext = 0;
  _short_command_set = false;
  _file = NULL;
  _cwd = new StringList("/data/t/kc-club/KC-Club/Homepage/DOWNLOAD/DISK151", '/');
}

VDIP::~VDIP(void)
{
}

void
VDIP::register_pio(PIO *pio)
{
  _pio = pio;
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
VDIP::send_prompt(void)
{
  send_string(_short_command_set ? ">\r" : "D:\\>\r");
  set_pio_ext_b(0x02);
  z80->addCallback(20000, this, (void *)_cbval);
}

void
VDIP::send_error(vdip_error_t error)
{
  switch (error)
    {
    case ERR_COMMAND_FAILED:
      send_string(_short_command_set ? "CF\r" : "Command Failed\r");
      break;
    case ERR_INVALID:
      send_string(_short_command_set ? "FI\r" : "Invalid\r");
      break;
    case ERR_BAD_COMMAND:
      send_string(_short_command_set ? "BC\r" : "Bad Command\r");
      break;
    }
  set_pio_ext_b(0x02);
  z80->addCallback(20000, this, (void *)_cbval);
}

void
VDIP::send_char(int c)
{
  _input = false;
  _output_buffer += c;
}

void
VDIP::send_dword(dword_t val)
{
  _input = false;
  for (int a = 0;a <= 24;a += 8)
    send_char((val >> a) & 0xff);
}

void
VDIP::send_string(const char *text)
{
  //printf("%Ld: send_string: '%s'\n", z80->getCounter(), text);
  _input = false;
  _output_buffer += string(text);
}

void
VDIP::reset(void)
{
  _cbval++;
  _reset = false;
  send_string("\rVer 03.60VDAPF On-Line:\r");
  set_pio_ext_b(0x02);
  z80->addCallback(20000, this, (void *)_cbval);
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
    z80->addCallback(20000, this, (void *)_cbval);
}

void
VDIP::write_byte(byte_t val)
{
  if (_reset)
    return;

  _input_data = val;
  //printf("VDIP: write: %02xh / '%c'\n", val, isprint(val) ? val : '.');
}

void
VDIP::write_end(void)
{
  if (_reset)
    return;

  //printf("%Ld: write_end\n", z80->getCounter());
  if (_input_data != 0x0d)
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

      if (_input_buffer.at(0) == CMD_SCS)
        {
          _short_command_set = true;
          send_prompt();
        }
      else if ((_input_buffer.at(0) == CMD_ECS))
        {
          _short_command_set = false;
          send_prompt();
        }
      else if ((strncmp(_input_buffer.c_str(), "CD ", 3) == 0) || ((_input_buffer.at(0) == CMD_CD) && (_input_buffer.at(1) == ' ')))
        {
          string dir = _input_buffer.substr(_input_buffer.find(' ') + 1);
          printf("CD %s\n", dir.c_str());
          if (strcmp(dir.c_str(), ".") == 0)
            {
              send_prompt();
            }
          else if ((strcmp(dir.c_str(), "..") == 0) && (_cwd->size() > 0))
            {
              _cwd->pop_back();
              send_prompt();
            }
          else
            {
              struct stat buf;
              string path = string("/") + _cwd->join('/') + "/" + dir;
              if (stat(path.c_str(), &buf) == 0)
                {
                  _cwd->push_back(dir);
                  send_prompt();
                }
              else
                {
                  send_error(ERR_COMMAND_FAILED);
                }
            }
        }
      else if ((strcmp(_input_buffer.c_str(), "DIR") == 0) || ((_input_buffer.at(0) == CMD_DIR) && (_input_buffer.at(1) == 0x0d)))
        {
          string path = string("/") + _cwd->join('/');
          printf("DIR (cwd = '%s')\n", path.c_str());
          DIR *dir = opendir(path.c_str());
          send_string("\r");
          if (dir != NULL)
            {
              send_string(". DIR\r.. DIR\r");
              while (242)
                {
                  struct dirent *dirent = readdir(dir);
                  if (dirent == NULL)
                    break;

                  switch (dirent->d_type)
                    {
                    case DT_REG:
                      send_string(dirent->d_name);
                      send_string("\r");
                      break;
                    case DT_DIR:
                      if ((strcmp(dirent->d_name, ".") != 0) && (strcmp(dirent->d_name, "..") != 0))
                        {
                          send_string(dirent->d_name);
                          send_string(" DIR\r");
                        }
                      break;
                    default:
                      break;
                    }
                }
              send_prompt();
            }
        }
      else if ((_input_buffer.at(0) == CMD_DIR) && (_input_buffer.at(1) == ' '))
        {
          struct stat buf;
          printf("DIR FOR FILE: %s\n", _input_buffer.substr(2).c_str());
          string filename = string("/") + _cwd->join('/') + "/" + _input_buffer.substr(2);
          if (stat(filename.c_str(), &buf) == 0)
            {
              send_string("\r");
              send_string(_input_buffer.substr(2).c_str());
              send_char(' ');
              send_dword(buf.st_size);
              send_char('\r');
              send_prompt();
            }
          else
            {
              send_error(ERR_COMMAND_FAILED);
            }
        }
      else if ((_input_buffer.at(0) == CMD_CLF) && (_input_buffer.at(1) == ' '))
        {
          printf("CLOSE FILE: %s\n", _input_buffer.substr(2).c_str());
          if (_file == NULL)
            {
              send_error(ERR_COMMAND_FAILED);
            }
          else
            {
              fclose(_file);
              send_prompt();
              _file = NULL;
            }
        }
      else if ((_input_buffer.at(0) == CMD_OPR) && (_input_buffer.at(1) == ' '))
        {
          printf("OPEN FOR READ: %s\n", _input_buffer.substr(2).c_str());
          string filename = string("/") + _cwd->join('/') + "/" + _input_buffer.substr(2);
          _file = fopen(filename.c_str(), "rb");
          if (_file == NULL)
            send_error(ERR_COMMAND_FAILED);
          else
            send_prompt();
        }
      else if ((_input_buffer.at(0) == CMD_RDF) && (_input_buffer.at(1) == ' '))
        {
          if (_file == NULL)
            {
              send_error(ERR_INVALID);
            }
          else if (feof(_file))
            {
              send_error(ERR_COMMAND_FAILED);
            }
          else
            {
              dword_t len = 0;
              for (int a = 2;a < 6;a++)
                len = (len << 8) | (_input_buffer.at(a) & 0xff);
              printf("READ FROM FILE (%ld bytes)\n", len);
              for (dword_t a = 0;a < len;a++)
                send_char(fgetc(_file)); // use EOF as padding
              send_prompt();
            }
        }
      else
        {
          send_error(ERR_BAD_COMMAND);
        }
      _input_buffer = "";
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
