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

#include <fstream>
#include <iostream>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/z80.h"
#include "kc/memory.h"

#include "cmd/cmd.h"

#include "fileio/load.h"

#include "libdbg/dbg.h"

class CMD_kc_image_load : public CMD
{
private:
  enum {
    _NO_START_ADDR = -1,
    _NO_LOAD_ADDR  = -2,
    _LOAD_ERROR    = -3,
  };
  static const char * _path;

protected:
 public:
  CMD_kc_image_load(void);
  virtual ~CMD_kc_image_load(void);

  virtual void execute(CMD_Args *args, CMD_Context context);
            
  int image_save(const char *filename, int start_addr, int end_addr);
};

const char * CMD_kc_image_load::_path = NULL;

CMD_kc_image_load::CMD_kc_image_load(void) : CMD("kc-image-load")
{
  register_cmd("kc-image-run", 0);
  register_cmd("kc-image-load", 1);
  register_cmd("kc-image-save", 5);
}

CMD_kc_image_load::~CMD_kc_image_load(void)
{
}

void
CMD_kc_image_load::execute(CMD_Args *args, CMD_Context context)
{
  byte_t *buf;
  long a, size;
  const char *filename;
  int load, start, autostart;
  fileio_prop_t *prop;
  
  filename = 0;
  
  if (args)
    filename = args->get_string_arg("filename");
  else
    args = new CMD_Args;

  switch (context)
    {
    case 0:
      /*
       *  entry kc-image-run
       */
      args->set_long_arg("auto-start", 1);
      /* fall through */

    case 1:
      /*
       *  entry kc-image-load
       */
      if (!args->has_arg("auto-start"))
	args->set_long_arg("auto-start", 0);

      if (!filename)
        {
          args->set_string_arg("ui-file-select-title",
                               _("KCemu: Select File..."));
	  if (_path)
	    args->set_string_arg("ui-file-select-path", _path);
          args->add_callback("ui-file-select-CB-ok", this, 2);
          args->add_callback("ui-file-select-CB-cancel", this, 2);
          CMD_EXEC_ARGS("ui-file-select", args);
          break;
        }

      /* fall through */

    case 2:
      /*
       *  we should have a valid filename now, if not abort command
       */
      filename = args->get_string_arg("filename");
      if (!filename)
	return;

      _path = filename;

      if (fileio_load_file(filename, &prop) != 0)
	{
          args->set_string_arg("ui-dialog-title",
                               _("KCemu: Can't load file..."));
	  args->set_string_arg("ui-dialog-text",
			       _("The file you selected couldn't be loaded!"));
          CMD_EXEC_ARGS("ui-dialog-ok", args);
	  return;
	}

      if (DBG_check("KCemu/CMD/image"))
	fileio_debug_dump(prop, 0);

      args->set_user_data(prop);

      if (prop->next != NULL)
	{
	  args->set_string_arg("ui-dialog-title",
			       _("KCemu: Warning..."));
	  args->set_string_arg("ui-dialog-text",
			       _("The file you selected contains more than one\n"
				 "program. Only the first program will be loaded.\n"
				 "If you want to access all the contained\n"
				 "programs add this file to a tape-archive!"));
	  args->add_callback("ui-dialog-ok-CB", this, 3);
          CMD_EXEC_ARGS("ui-dialog-ok", args);
	  return;
      }

      /* fall through */

    case 3:
      prop = (fileio_prop_t *)args->get_user_data();
      if (prop == NULL)
	{
	  DBG(0, form("KCemu/internal_error",
		      "prop == NULL\n"));
	  return;
	}

      switch (prop->type)
	{
	case FILEIO_TYPE_BAS:
	case FILEIO_TYPE_PROT_BAS:
	  args->set_long_arg("load-address", 1025);
	  break;
	default:
	  if ((prop->valid & FILEIO_V_LOAD_ADDR) != FILEIO_V_LOAD_ADDR)
	    {
	      args->add_callback("ui-image-get-properties-CB-ok", this, 4);
	      CMD_EXEC_ARGS("ui-image-get-properties", args);
	      return;
	    }
	}

      /* fall through */

    case 4:
      if ((prop->valid & FILEIO_V_LOAD_ADDR) == FILEIO_V_LOAD_ADDR)
	load = prop->load_addr;
      if (args->has_arg("load-address"))
	load = args->get_long_arg("load-address");

      start = 0xe000;
      if ((prop->valid & FILEIO_V_START_ADDR) == FILEIO_V_START_ADDR)
	start = prop->start_addr;
      if (args->has_arg("start-address"))
	start = args->get_long_arg("start-address");

      autostart = 0;
      if (args->has_arg("auto-start"))
	autostart = args->get_long_arg("auto-start");
      if ((prop->valid & FILEIO_V_AUTOSTART) == FILEIO_V_AUTOSTART)
	if (prop->autostart == 0)
	  autostart = 0;

      buf = new byte_t[prop->size];
      size = fileio_get_image(prop, buf);

      DBG(1, form("KCemu/CMD/image",
		  "CMD: [image] file  = '%s' (%d bytes / raw image %d)\n"
		  "CMD: [image] load  = %04xh\n"
		  "CMD: [image] start = %04xh\n"
		  "CMD: [image] run   = %s\n",
		  prop->name, prop->size, size, load, start,
		  autostart ? "start automatically" : "load only"));

      for (a = 0;a < size;a++)
	  memory->memWrite8(load + a, buf[a]);

      delete buf;

      if (autostart)
	z80->jump(start);

      break;
    case 5:
      filename = args->get_string_arg("filename");
      if (!filename)
	return;
      if (args->has_arg("start-address") && args->has_arg("end-address"))
        image_save(filename, args->get_long_arg("start-address"), args->get_long_arg("end-address"));
      break;
    }
}

int
CMD_kc_image_load::image_save(const char *filename, int start_addr, int end_addr)
{
  if (start_addr < 0)
    start_addr = 0;
  if (end_addr > 0xffff)
    end_addr = 0xffff;
  if (start_addr > end_addr)
    start_addr = end_addr;

  FILE *f = fopen(filename, "wb");
  if (f == NULL)
    return 1;

  int length = end_addr - start_addr + 1;

  byte_t *buf = new byte_t[length];
  for (int a = 0;a < length;a++)
    buf[a] = memory->memRead8(start_addr + a);
  
  int ret = fwrite(buf, 1, length, f);
  fclose(f);

  delete buf;
    
  return ret != length;
}

__force_link(CMD_kc_image_load);
