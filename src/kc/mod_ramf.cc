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
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <iostream>
#include <iomanip>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/mod_ramf.h"

ModuleRAMFloppy::ModuleRAMFloppy(ModuleRAMFloppy &tmpl) :
  ModuleInterface(tmpl.get_name(), tmpl.get_id(), tmpl.get_type())
{
  _addr = 0;

  _ram = NULL;
  _portg = NULL;

  _master = &tmpl;
  _port = _master->allocate_port();

  if (_port < 0)
    {
      char buf[1024];
      snprintf(buf, sizeof(buf),
	       _("It's not possible to have more than two RAM-Floppy\n"
		 "modules. The first one is running on port 98h and\n"
		 "the second one on port 58h."));
      set_error_text(buf);
      return;
    }

  _ram = new byte_t[0x40000];

  for (int a = 0;a < 0x20000;a++)
    _ram[a] = 0xe5;

  init();

  _portg = ports->register_ports(get_name(), _port, 8, this, 0);
  set_valid(true);
}

ModuleRAMFloppy::ModuleRAMFloppy(const char *name) :
  ModuleInterface(name, 0, KC_MODULE_Z1013)
{
  _addr = 0;
  _port = -1;
  _ram = NULL;
  _portg = NULL;
  _master = NULL;

  _disk_a = -0x98;
  _disk_b = -0x58;

  set_valid(true);
}

ModuleRAMFloppy::~ModuleRAMFloppy(void)
{
  if (_portg)
    ports->unregister_ports(_portg);

  if (_port >= 0)
    if (_master)
      _master->release_port(_port);

  delete[] _ram;
}

void
ModuleRAMFloppy::read_file(const char *filename, long size)
{
  int c;
  FILE *f;

  if ((_addr + size) >= 0x3ff00)
    return;

  f = fopen(filename, "rb");
  if (f == NULL)
    return;

  while (242)
    {
      c = fgetc(f);
      if (c == EOF)
	break;

      if (_addr >= 0x3ff00)
	break;

      _ram[_addr++] = c;
    }

  _addr = (_addr + 0xff) & 0x3ff00;

  _ram[1] = (_addr >> 16) & 0x03;
  _ram[2] = (_addr >>  8) & 0xff;

  fclose(f);
}

void
ModuleRAMFloppy::init(void)
{
  int len;
  DIR *dir;
  struct stat statbuf;
  struct dirent *entry;
  char *dirname, *filename;

  if (kcemu_homedir == NULL)
    return;

  _addr = 0x0100;

  _ram[0] = _port;
  _ram[1] = 0;
  _ram[2] = 0;

  len = strlen(kcemu_homedir) + 21;
  dirname = new char[len];
  snprintf(dirname, len, "%s/.z1013-ramfloppy_%02x", kcemu_homedir, _port & 0xff);

  dir = opendir(dirname);
  if (dir == NULL)
    {
      delete[] dirname;
      return;
    }

  while (242)
    {
      entry = readdir(dir);
      if (entry == NULL)
	break;

      len = strlen(dirname) + strlen(entry->d_name) + 2;
      filename = new char[len];
      snprintf(filename, len, "%s/%s", dirname, entry->d_name);

      if (stat(filename, &statbuf) == 0)
	if (S_ISREG(statbuf.st_mode))
	  read_file(filename, statbuf.st_size);

      delete[] filename;
    }

  closedir(dir);
  delete[] dirname;
}

int
ModuleRAMFloppy::allocate_port(void)
{
  if (_disk_a < 0)
    {
      _disk_a = -_disk_a;
      return _disk_a;
    }

  if (_disk_b < 0)
    {
      _disk_b = -_disk_b;
      return _disk_b;
    }

  return -1;
}

void
ModuleRAMFloppy::release_port(int port)
{
  if (_disk_a == port)
    _disk_a = -_disk_a;

  if (_disk_b == port)
    _disk_b = -_disk_b;
}

void
ModuleRAMFloppy::m_out(word_t addr, byte_t val)
{
}

ModuleInterface *
ModuleRAMFloppy::clone(void)
{
  return new ModuleRAMFloppy(*this);
}

void
ModuleRAMFloppy::reset(bool power_on)
{
}

byte_t
ModuleRAMFloppy::in(word_t addr)
{
  byte_t ret = 0xff;

  switch (addr & 7)
    {
    case 0:
    case 1:
    case 2:
    case 3:
      _addr = (_addr & 0xffff) | ((addr & 3) << 16);
      ret = _ram[_addr];
      _addr = (_addr & 0xff00) | (((_addr & 0xff) + 1) & 0xff);
      break;
    default:
      break;
    }

  return ret;
}

void
ModuleRAMFloppy::out(word_t addr, byte_t val)
{
  switch (addr & 7)
    {
    case 0:
    case 1:
    case 2:
    case 3:
      _addr = (_addr & 0xffff) | ((addr & 3) << 16);
      _ram[_addr] = val;
      _addr = (_addr & 0xff00) | (((_addr & 0xff) + 1) & 0xff);
      break;
    case 6:
      _addr = (_addr & 0xff) | (val << 8);
      break;
    case 7:
      _addr = (_addr & 0xff00) | val;
      break;
    default:
      break;
    }
}
