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

#include <unistd.h>
#include <sys/stat.h>

#include "kc/system.h"

#include "kc/floppy.h"

#include "cmd/cmd.h"

#include "libdbg/dbg.h"

class CMD_floppy_attach : public CMD
{
private:
  Floppy *_f;
public:
  CMD_floppy_attach(Floppy *f, const char *cmdname) : CMD("floppy-attach")
    {
      _f = f;
      register_cmd(cmdname);
    }

  void execute(CMD_Args *args, CMD_Context context)
    {
      const char *filename;

      if (!args)
        return;

      filename = args->get_string_arg("filename");
      if (!filename)
        return;

      _f->attach(filename);
    }
};

Floppy::Floppy(const char *cmdname)
{
  _disk_prop = NULL;

  CMD *cmd;
  cmd = new CMD_floppy_attach(this, cmdname);
}

Floppy::~Floppy(void)
{
}

int
Floppy::get_head_count(void)
{
  if (_disk_prop == NULL)
    return -1;

  return _disk_prop->head_count;
}

int
Floppy::get_cylinder_count(void)
{
  if (_disk_prop == NULL)
    return -1;

  return _disk_prop->cylinder_count;
}

int
Floppy::get_sector_size(void)
{
  if (_disk_prop == NULL)
    return -1;

  return _disk_prop->sector_size;
}

int
Floppy::get_sectors_per_cylinder(void)
{
  if (_disk_prop == NULL)
    return -1;

  return _disk_prop->sectors_per_cylinder;
}

bool
Floppy::attach(const char *filename)
{
  if (filename == NULL)
    {
      libdisk_close(&_disk_prop);
      return true;
    }

  if (libdisk_open(&_disk_prop, filename) < 0)
    {
      //cerr << "Can't attach disk-image '" << filename << "'!" << endl;
      return false;
    }
  //cerr << "Attached disk-image '" << filename << "'" << endl;
  return true;
}

bool
Floppy::seek(int head, int cylinder, int sector)
{
  if (_disk_prop == NULL)
    return false;

  if (libdisk_seek(&_disk_prop, head, cylinder, sector) < 0)
    return false;

  return true;
}

int
Floppy::read_sector(byte_t *buf, int len)
{
  return libdisk_read_sector(&_disk_prop, buf, len);
}

int
Floppy::write_sector(byte_t *buf, int len)
{
  return libdisk_write_sector(&_disk_prop, buf, len);
}
