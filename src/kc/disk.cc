/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: disk.cc,v 1.5 2002/10/31 01:46:35 torsten_paul Exp $
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

#include "kc/system.h"

#include "kc/rc.h"
#include "kc/kc.h"
#include "kc/fdc.h"
#include "kc/disk.h"
#include "kc/floppy.h"

#include "cmd/cmd.h"

#include "ui/status.h"

#include "libdbg/dbg.h"

class CMD_disk_attach : public CMD
{
private:
  Disk *_d;

protected:
  int get_disk_no(CMD_Args *args)
  {
    int n = 0;
    
    if (args && args->has_arg("disk"))
      n = args->get_int_arg("disk");

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
      char buf[100]; /* FIXME: */
      disk_error_t err;
      const char *filename, *shortname;

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
	  CMD_EXEC_ARGS("ui-disk-update-MSG", args);
          return;
        }
      
      if (filename)
        {
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
              shortname = strrchr(filename, '/');
              if (shortname)
                shortname++;
              else
                shortname = filename;
              sprintf(buf, _("disk-file `%s' attached."), shortname);
              Status::instance()->setMessage(buf);
	      CMD_EXEC_ARGS("ui-disk-update-MSG", args);
              break;
            default:
              Status::instance()->setMessage(_("Can't attach disk-file."));
              break;
            }
        }
    }
};

Disk::Disk()
{
  _cmd = new CMD_disk_attach(this);
}

Disk::~Disk()
{
  delete _cmd;
}

disk_error_t
Disk::attach(int disk_no, const char *filename, bool create)
{
  if (create)
    {
      DBG(1, form("KCemu/Disk/attach",
                  "Disk::attach(): [disk %d] create (%s)\n",
                  disk_no, filename));
    }
  else
    {
      DBG(1, form("KCemu/Disk/attach",
                  "Disk::attach(): [disk %d] open (%s)\n",
                  disk_no, filename));

      Floppy *floppy = fdc_fdc->get_floppy(disk_no);
      if (floppy != NULL)
	{
	  if (!floppy->attach(filename))
	    return DISK_ERROR;
	}
    }

  return DISK_OK;
}

disk_error_t
Disk::detach(int disk_no)
{
  DBG(1, form("KCemu/Disk/detach",
	      "Disk::detach(): [disk %d] close\n",
	      disk_no));

  Floppy *floppy = fdc_fdc->get_floppy(disk_no);
  if (floppy != NULL)
    floppy->attach(NULL);

  return DISK_OK;
}
