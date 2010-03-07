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

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/fdc.h"
#include "kc/disk.h"
#include "kc/floppy.h"

#include "cmd/cmd.h"

#include "sys/sysdep.h"

#include "ui/status.h"

#include "libdbg/dbg.h"

class CMD_disk_attach : public CMD
{
private:
  Disk *_d;
  static const char * _path;

protected:
  int get_disk_no(CMD_Args *args)
  {
    int n = 0;
    
    if (args && args->has_arg("disk"))
      n = args->get_long_arg("disk");

    return n;
  }
  
public:
  CMD_disk_attach(Disk *d) : CMD("disk-attach")
  {
    _d = d;
    register_cmd("disk-attach", 0);
    register_cmd("disk-detach", 3);
  }
  
  void execute(CMD_Args *args, CMD_Context context)
    {
      bool create;
      char buf[1000];
      disk_error_t err;
      char *shortname;
      const char *filename;

      create = false;
      filename = NULL;
      switch (context)
        {
          /*
           *  disk-attach
           */
        case 0:
          if (!args)
            args = new CMD_Args();
          filename = args->get_string_arg("filename");
          if (!filename)
            {
              args->set_string_arg("ui-file-select-title",
                                   _("Select disk..."));
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
           *  ui-dialog-yes-no-CB-yes
           */
        case 2:
          if (args)
            filename = args->get_string_arg("filename");
          create = true;
          break;
          /*
           *  disk-detach
           */
        case 3:
          _d->detach(get_disk_no(args));
          return;
        }
      
      if (filename)
        {
	  _path = filename;
          err = _d->attach(get_disk_no(args), filename, create);
          switch (err)
            {
            case DISK_NOENT:
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
            case DISK_OK:
              shortname = sys_basename(filename);
              snprintf(buf, sizeof(buf), _("disk-file `%s' attached."), shortname);
	      free(shortname);
              Status::instance()->setMessage(buf);
              break;
            default:
              Status::instance()->setMessage(_("Can't attach disk-file."));
              break;
            }
        }
    }
};

const char * CMD_disk_attach::_path = NULL;

Disk::Disk()
{
  _cmd = new CMD_disk_attach(this);
}

Disk::~Disk()
{
  delete _cmd;
}

bool
Disk::create_disk_file(FILE *f)
{
  for (int c = 0;c < 80;c++)
    for (int h = 0;h < 2;h++)
      for (int s = 1;s < 6;s++)
	if (!write_sector(f, c, h, s))
	  return false;

  return true;
}

bool
Disk::write_sector(FILE *f, int c, int h, int s)
{
  if (fputc(c, f) == EOF) // acyl
    return false;
  if (fputc(h, f) == EOF) // asid
    return false;
  if (fputc(c, f) == EOF) // lcyl
    return false;
  if (fputc(h, f) == EOF) // lsid
    return false;
  if (fputc(s, f) == EOF) // lsec
    return false;
  if (fputc(3, f) == EOF) // llen
    return false;
  if (fputc(0, f) == EOF) // count low
    return false;
  if (fputc(4, f) == EOF) // count high
    return false;

  for (int a = 0;a < 1024;a++)
    if (fputc(0xe5, f) == EOF)
      return false;

  return true;
}

disk_error_t
Disk::attach(int disk_no, const char *filename, bool create)
{
  char *ptr;
  disk_error_t ret;

  if (fdc_fdc == NULL)
    return DISK_ERROR;

  if (filename == NULL)
    return DISK_ERROR;

  ret = DISK_OK;

  if (create)
    {
      DBG(1, form("KCemu/Disk/attach",
		  "Disk::attach(): [disk %d] create (%s)\n",
		  disk_no, filename));

      FILE *f = fopen(filename, "wb");
      if (f == NULL)
	return DISK_ERROR;

      bool create_ok = create_disk_file(f);
      fclose(f);

      if (!create_ok)
	return DISK_ERROR;
    }

  DBG(1, form("KCemu/Disk/attach",
	      "Disk::attach(): [disk %d] open (%s)\n",
	      disk_no, filename));

  Floppy *floppy = fdc_fdc->get_floppy(disk_no);
  if (floppy != NULL)
    {
      if (access(filename, R_OK) == 0)
	{
	  ptr = strdup(filename);
	}
      else
	{
	  ptr = (char *)malloc(strlen(kcemu_datadir) + strlen(filename) + 11);
	  strcpy(ptr, kcemu_datadir);
	  strcat(ptr, "/disks/");
	  strcat(ptr, filename);
	  if (access(ptr, R_OK) != 0)
	    strcat(ptr, ".gz");
	}

      if (access(ptr, R_OK) == 0)
	{
	  if (!floppy->attach(ptr))
	    ret = DISK_ERROR;
	}
      else
	{
	  ret = DISK_NOENT;
	}

      free(ptr);
    }

  CMD_Args *args = new CMD_Args();
  args->set_long_arg("disk", disk_no);
  args->set_string_arg("filename", filename);
  CMD_EXEC_ARGS("ui-disk-update-MSG", args);
  delete args;

  return ret;
}

disk_error_t
Disk::detach(int disk_no)
{
  if (fdc_fdc == NULL)
    return DISK_ERROR;

  DBG(1, form("KCemu/Disk/detach",
	      "Disk::detach(): [disk %d] close\n",
	      disk_no));

  Floppy *floppy = fdc_fdc->get_floppy(disk_no);
  if (floppy != NULL)
    floppy->attach(NULL);

  CMD_Args *args = new CMD_Args();
  args->set_long_arg("disk", disk_no);
  args->set_string_arg("filename", "");
  CMD_EXEC_ARGS("ui-disk-update-MSG", args);
  delete args;

  return DISK_OK;
}
