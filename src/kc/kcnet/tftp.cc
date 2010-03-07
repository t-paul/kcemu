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

#include <string>

#include <stdio.h>
#include <unistd.h>

#include "kc/system.h"

#include "kc/kc.h"

#include "kc/prefs/prefs.h"

#include "kc/kcnet/tftp.h"

#include "libdbg/dbg.h"

TFTP::TFTP(void)
{
  _block = 0;
  _send_data = NULL;
}

TFTP::~TFTP(void)
{
}

void
TFTP::set_ip_address(byte_t a0, byte_t a1, byte_t a2, byte_t a3)
{

}

void
TFTP::set_port(word_t port)
{

}

bool
TFTP::open(void)
{
  return true;
}

bool
TFTP::is_open(void)
{
  return true;
}

void
TFTP::close(void)
{
}

void
TFTP::poll(void)
{
}

void
TFTP::send(SocketData *data)
{
  printf("TFTP::send(): len = %d\n", data->length());
  if (data->length() < 2)
    return;

  for (int a = 0;a < data->length();a++)
    printf("%02x ", data->get(a));
  printf("\n");
  
  word_t opcode = data->get(0) << 8 | data->get(1);
  if (opcode == TFTP_RRQ)
    {
      init_read(data);
    }
  else if (opcode == TFTP_WRQ)
    {
    }
  else if (opcode == TFTP_ACK)
    {
      printf("ACK!\n");
    }
}

SocketData *
TFTP::receive(void)
{
  if (_send_data == NULL)
    {
      if (_path.length() > 0)
        {
          byte_t buf[512];

          printf("TFTP::receive(): reading block %d\n", _block);

          FILE *f = fopen(_path.c_str(), "rb");
          fseek(f, _block * 512, SEEK_SET);
          int len = fread(buf, 1, 512, f);
          fclose(f);
          _block++;

          printf("TFTP::receive(): block len = %d\n", len);

          SocketData *data = new SocketData(len + 4);
          data->put_word(TFTP_DATA);
          data->put_word(_block);
          for (int a = 0;a < len;a++)
            data->put_byte(buf[a]);

          if (len < 512)
            {
              printf("TFTP::receive(): END\n");
              _path.clear();
            }

          return data;
        }
      return NULL;
    }

  SocketData *ret = _send_data;
  printf("TFTP::receive(): len = %d\n", ret->length());
  for (int a = 0;a < ret->length();a++)
    printf("%02x ", ret->get(a));
  printf("\n");
  _send_data = NULL;
  return ret;
}

void
TFTP::init_read(SocketData *data)
{
  // FIXME: range check!
  string filename((const char *)data->get() + 2);
  printf("init_read: %s\n", filename.c_str());

  const char *root = Preferences::instance()->get_string_value("kcnet_tftp_root", NULL);
  _path = string(root == NULL ? "/tmp" : root) + "/" + filename;

  if (access(_path.c_str(), R_OK) != 0)
    {
      printf("FILE NOT FOUND!\n");
      _send_data = new SocketData(20);
      _send_data->put_word(TFTP_ERROR);
      _send_data->put_word(TFTP_ERR_FILE_NOT_FOUND);
      _send_data->put_text("File not found.");
      _send_data->put_byte(0);
    }
  else
    {
      _block = 0;
      printf("READING FILE %s!\n", _path.c_str());
    }
}
