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

#include "kc/system.h"

#include "cmd/cmd.h"

#include "ui/gtk/vdip.h"
#include "ui/gtk/cmd.h"

#include "libdbg/dbg.h"

class CMD_ui_vdip_window_toggle : public CMD
{
private:
  VDIPWindow *_w;
  
public:
  CMD_ui_vdip_window_toggle(VDIPWindow *w) : CMD("ui-vdip-window-toggle")
    {
      _w = w;
      register_cmd("ui-vdip-window-toggle");
    }
  
  void execute(CMD_Args *args, CMD_Context context)
    {
      _w->toggle();
    }
};

class CMD_ui_vdip_update : public CMD
{
private:
  VDIPWindow *_w;
public:
  CMD_ui_vdip_update(VDIPWindow *w) : CMD("ui-vdip-update")
  {
    _w = w;
    register_cmd("ui-vdip-update-MSG");
  }

  void execute(CMD_Args *args, CMD_Context context)
  {
    DBG(2, form("KCemu/VDIP/update",
		"got vdip-update message\n"));

    long slot_no = args->get_long_arg("slot");
    if ((slot_no < 0) || (slot_no > 1))
      return;

    _w->set_name(slot_no, args->get_string_arg("filename"));
  }
};

class CMD_ui_vdip_attach : public CMD
{
private:
  VDIPWindow *_w;
public:
  CMD_ui_vdip_attach(VDIPWindow *w) : CMD("ui-vdip-attach")
  {
    _w = w;
    register_cmd("ui-vdip-attach-1", 0);
    register_cmd("ui-vdip-attach-2", 1);
    register_cmd("ui-vdip-detach-1", 4);
    register_cmd("ui-vdip-detach-2", 5);
  }
  
  void execute(CMD_Args *args, CMD_Context context)
  {
    const char *filename;
    
    if (!args)
      args = new CMD_Args();

    filename = NULL;
    switch (context)
      {
      case 0:
      case 1:
      case 2:
      case 3:
	args->set_long_arg("slot", context);
	CMD_EXEC_ARGS("vdip-attach", args);
	break;
      case 4:
      case 5:
      case 6:
      case 7:
	args->set_long_arg("slot", context & 3);
	CMD_EXEC_ARGS("vdip-detach", args);
	break;
      }
  }
};

VDIPWindow::VDIPWindow(const char *ui_xml_file) : UI_Gtk_Window(ui_xml_file)
{
  init();

  _cmd_attach = new CMD_ui_vdip_attach(this);
  _cmd_update = new CMD_ui_vdip_update(this);
  _cmd_window_toggle = new CMD_ui_vdip_window_toggle(this);
}

VDIPWindow::~VDIPWindow(void)
{
  delete _cmd_attach;
  delete _cmd_update;
  delete _cmd_window_toggle;
}
  
void
VDIPWindow::set_name(int idx, const char *name)
{
  GtkEntry *entry = GTK_ENTRY(GTK_BIN(_w.combo[idx])->child);

  if (!name)
    name = "";

  gtk_signal_handler_block(GTK_OBJECT(_w.combo[idx]), _w.combo_signal_id[idx]);
  gtk_entry_set_text(entry, name);
  gtk_signal_handler_unblock(GTK_OBJECT(_w.combo[idx]), _w.combo_signal_id[idx]);
}

void
VDIPWindow::sf_vdip_attach(GtkWidget *widget, gpointer data)
{
  long nr = (long)data;
  GtkEntry *entry = GTK_ENTRY(GTK_BIN(widget)->child);
  const gchar *text = gtk_entry_get_text(GTK_ENTRY(entry));

  if (text == NULL)
    return;
  if (strlen(text) == 0)
    return;

  CMD_Args *args = new CMD_Args();
  args->set_long_arg("slot", nr);
  args->set_string_arg("filename", text);
  CMD_EXEC_ARGS("vdip-attach", args);
}

void
VDIPWindow::init(void)
{
  static const char * attach_cmd[2] = {
    "ui-vdip-attach-1",
    "ui-vdip-attach-2",
  };
  static const char * detach_cmd[2] = {
    "ui-vdip-detach-1",
    "ui-vdip-detach-2",
  };

  /*
   *  vdip window
   */
  _window = get_widget("vdip_window");
  gtk_signal_connect(GTK_OBJECT(_window), "delete_event",
                     GTK_SIGNAL_FUNC(cmd_exec_sft),
                     (char *)"ui-vdip-window-toggle"); // FIXME:


  _w.combo[0] = get_widget("vdip_comboboxentry_1");
  _w.combo[1] = get_widget("vdip_comboboxentry_2");

  _w.browse[0] = get_widget("vdip_button_open_1");
  _w.browse[1] = get_widget("vdip_button_open_2");
  
  _w.eject[0] = get_widget("vdip_button_close_1");
  _w.eject[1] = get_widget("vdip_button_close_2");

  for (int a = 0;a < 2;a++) {
    _w.combo_signal_id[a] = gtk_signal_connect(GTK_OBJECT(GTK_COMBO_BOX(_w.combo[a])),
					       "changed",
					       GTK_SIGNAL_FUNC(sf_vdip_attach),
					       (gpointer)a);

    gtk_signal_connect(GTK_OBJECT(_w.browse[a]), "clicked",
		       GTK_SIGNAL_FUNC(cmd_exec_sf),
		       (char *)attach_cmd[a]); // FIXME:
    gtk_signal_connect(GTK_OBJECT(_w.eject[a]), "clicked",
		       GTK_SIGNAL_FUNC(cmd_exec_sf),
		       (char *)detach_cmd[a]); // FIXME:
  }

  init_dialog("ui-vdip-window-toggle", "window-vdip");
}
