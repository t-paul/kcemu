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
#include <pango-1.0/pango/pango-layout.h>

#include "kc/system.h"

#include "cmd/cmd.h"

#include "ui/gtk/disk.h"
#include "ui/gtk/cmd.h"

#include "libdbg/dbg.h"

class CMD_ui_disk_window_toggle : public CMD
{
private:
  DiskWindow *_w;
  
public:
  CMD_ui_disk_window_toggle(DiskWindow *w) : CMD("ui-disk-window-toggle")
    {
      _w = w;
      register_cmd("ui-disk-window-toggle");
    }
  
  void execute(CMD_Args *args, CMD_Context context)
    {
      _w->toggle();
    }
};

class CMD_ui_disk_update : public CMD
{
private:
  DiskWindow *_w;
public:
  CMD_ui_disk_update(DiskWindow *w) : CMD("ui-disk-update")
  {
    _w = w;
    register_cmd("ui-disk-update-MSG");
  }

  void execute(CMD_Args *args, CMD_Context context)
  {
    int disk_no;

    DBG(2, form("KCemu/Disk/update",
		"got disk-update message\n"));

    disk_no = args->get_long_arg("disk");
    if ((disk_no < 0) || (disk_no > 3))
      return;

    _w->set_name(disk_no, args->get_string_arg("filename"));
  }
};

class CMD_ui_disk_attach : public CMD
{
private:
  DiskWindow *_w;
public:
  CMD_ui_disk_attach(DiskWindow *w) : CMD("ui-disk-attach")
  {
    _w = w;
    register_cmd("ui-disk-attach-1", 0);
    register_cmd("ui-disk-attach-2", 1);
    register_cmd("ui-disk-attach-3", 2);
    register_cmd("ui-disk-attach-4", 3);
    register_cmd("ui-disk-detach-1", 4);
    register_cmd("ui-disk-detach-2", 5);
    register_cmd("ui-disk-detach-3", 6);
    register_cmd("ui-disk-detach-4", 7);
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
	args->set_long_arg("disk", context);
	CMD_EXEC_ARGS("disk-attach", args);
	break;
      case 4:
      case 5:
      case 6:
      case 7:
	args->set_long_arg("disk", context & 3);
	CMD_EXEC_ARGS("disk-detach", args);
	break;
      }
  }
};

DiskWindow::DiskWindow(const char *ui_xml_file) : UI_Gtk_Window(ui_xml_file)
{
  init();

  _cmd_attach = new CMD_ui_disk_attach(this);
  _cmd_update = new CMD_ui_disk_update(this);
  _cmd_window_toggle = new CMD_ui_disk_window_toggle(this);
}

DiskWindow::~DiskWindow(void)
{
  delete _cmd_attach;
  delete _cmd_update;
  delete _cmd_window_toggle;
}
  
void
DiskWindow::set_name(int idx, const char *name)
{
  GtkEntry *entry = GTK_ENTRY(GTK_BIN(_w.combo[idx])->child);

  if (!name)
    name = "";

  gtk_signal_handler_block(GTK_OBJECT(_w.combo[idx]), _w.combo_signal_id[idx]);
  gtk_entry_set_text(entry, name);
  gtk_signal_handler_unblock(GTK_OBJECT(_w.combo[idx]), _w.combo_signal_id[idx]);
}

void
DiskWindow::sf_disk_attach(GtkWidget *widget, gpointer data)
{
  long nr = (long)data;
  GtkEntry *entry = GTK_ENTRY(GTK_BIN(widget)->child);
  const gchar *text = gtk_entry_get_text(GTK_ENTRY(entry));

  if (text == NULL)
    return;
  if (strlen(text) == 0)
    return;

  CMD_Args *args = new CMD_Args();
  args->set_long_arg("disk", nr);
  args->set_string_arg("filename", text);
  CMD_EXEC_ARGS("disk-attach", args);
}

void
DiskWindow::init(void)
{
  static const char * attach_cmd[4] = {
    "ui-disk-attach-1",
    "ui-disk-attach-2",
    "ui-disk-attach-3",
    "ui-disk-attach-4",
  };
  static const char * detach_cmd[4] = {
    "ui-disk-detach-1",
    "ui-disk-detach-2",
    "ui-disk-detach-3",
    "ui-disk-detach-4",
  };
  static const char * disk_files[] = {
    "a5105sys.dump",
    "caos.dump",
    "cpmz9.dump",
    "microdos.dump",
    "tools.dump",
    "z1013cpm.dump",
    "z1013gdc.dump",
    NULL
  };

  GtkTreeIter iter;
  GtkListStore *store = gtk_list_store_new(1, G_TYPE_STRING);
//  for (int a = 0;disk_files[a] != NULL;a++)
//    {
//      gtk_list_store_append(store, &iter);
//      gtk_list_store_set(store, &iter, 0, disk_files[a], -1);
//    }

  /*
   *  disk window
   */
  _window = get_widget("disk_window");
  gtk_signal_connect(GTK_OBJECT(_window), "delete_event",
                     GTK_SIGNAL_FUNC(cmd_exec_sft),
                     (char *)"ui-disk-window-toggle"); // FIXME:


  _w.combo[0] = get_widget("disk_comboboxentry_1");
  _w.combo[1] = get_widget("disk_comboboxentry_2");
  _w.combo[2] = get_widget("disk_comboboxentry_3");
  _w.combo[3] = get_widget("disk_comboboxentry_4");

  for (int a = 0;a < 4;a++)
    {
      gtk_combo_box_set_model(GTK_COMBO_BOX(_w.combo[a]), GTK_TREE_MODEL(store));
      GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
      gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(_w.combo[a]), renderer, TRUE);
      gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(_w.combo[a]), renderer, "text", 0, NULL);
    }

  _w.browse[0] = get_widget("disk_button_open_1");
  _w.browse[1] = get_widget("disk_button_open_2");
  _w.browse[2] = get_widget("disk_button_open_3");
  _w.browse[3] = get_widget("disk_button_open_4");
  
  _w.eject[0] = get_widget("disk_button_close_1");
  _w.eject[1] = get_widget("disk_button_close_2");
  _w.eject[2] = get_widget("disk_button_close_3");
  _w.eject[3] = get_widget("disk_button_close_4");

  for (int a = 0;a < NR_OF_DISKS;a++) {
    _w.combo_signal_id[a] = gtk_signal_connect(GTK_OBJECT(GTK_COMBO_BOX(_w.combo[a])),
					       "changed",
					       GTK_SIGNAL_FUNC(sf_disk_attach),
					       (gpointer)a);

    gtk_signal_connect(GTK_OBJECT(_w.browse[a]), "clicked",
		       GTK_SIGNAL_FUNC(cmd_exec_sf),
		       (char *)attach_cmd[a]); // FIXME:
    gtk_signal_connect(GTK_OBJECT(_w.eject[a]), "clicked",
		       GTK_SIGNAL_FUNC(cmd_exec_sf),
		       (char *)detach_cmd[a]); // FIXME:
  }

  init_dialog("ui-disk-window-toggle", "window-disk");
}
