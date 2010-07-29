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

#include "kc/kc.h"
#include "kc/memory.h"
#include "ui/status.h"
#include "cmd/cmd.h"

class CMD_load : public CMD, public CMD_Caller, public CMD_Change_Listener
{
public:
  
  CMD_load(void) : CMD("load")
    {
      register_cmd("emu-load-image", 0);
    }

  void execute(CMD_Args *args, CMD_Context context)
    {
      CMD_Args *a;

      switch (context)
        {
        case 0:
          Status::instance()->setMessage("*** LOAD IMAGE ***");
          a = new CMD_Args();
          cmd_caller_set_args(a);
          a->add_change_listener(this);
          CMD_Repository::instance()->execute("ui-browse", a);
          a->dump("KCCMD_LOAD_IMAGE");
          break;
        case 1:
          Status::instance()->setMessage("*** LOAD IMAGE OK ***");
          cmd_caller_get_args()->dump("KCCMD_LOAD_IMAGE_OK");
          break;
        }
    }

  void cmd_args_changed(void)
    {
      Status::instance()->setMessage(cmd_caller_get_args()->get_string_arg("file-name"));      
      memory->loadRAM(cmd_caller_get_args()->get_string_arg("file-name"));
    }
};

__force_link(CMD_load);
