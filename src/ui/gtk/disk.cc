/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: disk.cc,v 1.4 2002/01/06 12:53:40 torsten_paul Exp $
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

#include <stdio.h>

#include "kc/config.h"
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
		"got disk-update messge\n"));

    disk_no = args->get_int_arg("disk");
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
	args->set_int_arg("disk", context);
	CMD_EXEC_ARGS("disk-attach", args);
	break;
      case 4:
      case 5:
      case 6:
      case 7:
	args->set_int_arg("disk", context & 3);
	CMD_EXEC_ARGS("disk-detach", args);
	break;
      }
  }
};
  
void
DiskWindow::set_name(int idx, const char *name)
{
  GtkWidget *entry = GTK_COMBO(_w.combo[idx])->entry;

  if (!name)
    name = "";

  gtk_signal_handler_block(GTK_OBJECT(entry), _w.combo_signal_id[idx]);
  gtk_entry_set_text(GTK_ENTRY(entry), name);
  gtk_signal_handler_unblock(GTK_OBJECT(entry), _w.combo_signal_id[idx]);
}

void
DiskWindow::sf_disk_attach(GtkWidget *widget, gpointer data)
{
  CMD_Args *args;
  int nr = (int)data;
  char *text = gtk_entry_get_text(GTK_ENTRY(widget));

  if (text == NULL)
    return;
  if (strlen(text) == 0)
    return;

  args = new CMD_Args();
  args->set_int_arg("disk", nr);
  args->set_string_arg("filename", text);
  CMD_EXEC_ARGS("disk-attach", args);
}

void
DiskWindow::init(void)
{
  int a;
  int ltextl;
  char *ltext;
  char *ltextf;
  GList *popdown;
  static char * attach_cmd[4] = {
    "ui-disk-attach-1",
    "ui-disk-attach-2",
    "ui-disk-attach-3",
    "ui-disk-attach-4",
  };
  static char * detach_cmd[4] = {
    "ui-disk-detach-1",
    "ui-disk-detach-2",
    "ui-disk-detach-3",
    "ui-disk-detach-4",
  };
  char *disk1;
  char *disk2;
  char *disk3;
  GtkTooltips *tips;

  tips = gtk_tooltips_new();
  gtk_tooltips_enable(tips);
  
  /*
   *  disk window
   */
  _window = gtk_window_new(GTK_WINDOW_DIALOG);
  gtk_widget_set_name(_window, "DiskWindow");
  gtk_window_set_title(GTK_WINDOW(_window), _("KCemu: Disk"));
  gtk_window_position(GTK_WINDOW(_window), GTK_WIN_POS_MOUSE);
  gtk_signal_connect(GTK_OBJECT(_window), "delete_event",
                     GTK_SIGNAL_FUNC(cmd_exec_sft),
                     (char *)"ui-disk-window-toggle"); // FIXME:
  
  /*
   *  vbox
   */
  _w.vbox = gtk_vbox_new(FALSE, 2);
  gtk_container_border_width(GTK_CONTAINER(_w.vbox), 6);
  gtk_container_add(GTK_CONTAINER(_window), _w.vbox);
  gtk_widget_show(_w.vbox);

  /*
   *  table
   */
  _w.table = gtk_table_new(NR_OF_DISKS, 4, FALSE);
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.table, FALSE, TRUE, 0);
  gtk_table_set_row_spacings(GTK_TABLE(_w.table), 4);
  gtk_table_set_col_spacings(GTK_TABLE(_w.table), 6);
  gtk_widget_show(_w.table);

  for (a = 0;a < NR_OF_DISKS;a++) {
    /*
     *  label
     */
    ltextf = _("Disk %d");
    ltextl = strlen(ltextf) + 3;
    ltext = new char[ltextl];
    snprintf(ltext, ltextl, ltextf, a + 1);
    _w.label[a] = gtk_label_new(ltext);
    gtk_table_attach(GTK_TABLE(_w.table), _w.label[a],
		     0, 1, a, a + 1,
		     (GtkAttachOptions)0, (GtkAttachOptions)0, 0, 0);
    gtk_widget_show(_w.label[a]);
    delete(ltext);

    /*
     *  combo box
     */
    popdown = NULL;
    disk1 = new char[strlen(DATADIR) + 15];
    strcpy(disk1, DATADIR);
    strcat(disk1, "/");
    strcat(disk1, "microdos.dump");

    disk2 = new char[strlen(DATADIR) + 11];
    strcpy(disk2, DATADIR);
    strcat(disk2, "/");
    strcat(disk2, "caos.dump");

    disk3 = new char[strlen(DATADIR) + 12];
    strcpy(disk3, DATADIR);
    strcat(disk3, "/");
    strcat(disk3, "tools.dump");

    popdown = g_list_append(popdown, disk1);
    popdown = g_list_append(popdown, disk2);
    //popdown = g_list_append(popdown, disk3);
    _w.combo[a] = gtk_combo_new();
    gtk_table_attach_defaults(GTK_TABLE(_w.table), _w.combo[a],
			      1, 2, a, a + 1);
    gtk_combo_set_popdown_strings(GTK_COMBO(_w.combo[a]), popdown);
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(_w.combo[a])->entry), "");
    gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(_w.combo[a])->entry), FALSE);
    _w.combo_signal_id[a] = gtk_signal_connect(GTK_OBJECT(GTK_COMBO(_w.combo[a])->entry),
					       "changed",
					       GTK_SIGNAL_FUNC(sf_disk_attach),
					       (gpointer)a);
    gtk_widget_show(_w.combo[a]);

    /*
     *  browse button
     */
    _w.browse[a] = gtk_button_new_with_label(_("Browse"));
    gtk_table_attach(GTK_TABLE(_w.table), _w.browse[a],
		     2, 3, a, a + 1,
		     (GtkAttachOptions)0, (GtkAttachOptions)0, 0, 0);
    gtk_signal_connect(GTK_OBJECT(_w.browse[a]), "clicked",
		       GTK_SIGNAL_FUNC(cmd_exec_sf),
		       (char *)attach_cmd[a]); // FIXME:
    gtk_widget_show(_w.browse[a]);
    GTK_WIDGET_UNSET_FLAGS(_w.browse[a], GTK_CAN_DEFAULT);

    /*
     *  eject button
     */
    _w.eject[a] = gtk_button_new_with_label(_("Eject"));
    gtk_table_attach(GTK_TABLE(_w.table), _w.eject[a],
		     3, 4, a, a + 1,
		     (GtkAttachOptions)0, (GtkAttachOptions)0, 0, 0);
    gtk_signal_connect(GTK_OBJECT(_w.eject[a]), "clicked",
		       GTK_SIGNAL_FUNC(cmd_exec_sf),
		       (char *)detach_cmd[a]); // FIXME:
    gtk_widget_show(_w.eject[a]);
    GTK_WIDGET_UNSET_FLAGS(_w.eject[a], GTK_CAN_DEFAULT);
  }

  _w.separator = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.separator,
		     FALSE, FALSE, 5);
  gtk_widget_show(_w.separator);
    
  /*
   *  close button
   */
  _w.close = gtk_button_new_with_label(_("Close"));
  gtk_signal_connect(GTK_OBJECT(_w.close), "clicked",
                     GTK_SIGNAL_FUNC(cmd_exec_sf),
                     (gpointer)"ui-disk-window-toggle");
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.close, FALSE, FALSE, 5);
  GTK_WIDGET_SET_FLAGS(_w.close, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(_w.close);
  gtk_widget_show(_w.close);

  CMD *cmd;
  cmd = new CMD_ui_disk_window_toggle(this);
  cmd = new CMD_ui_disk_attach(this);
  cmd = new CMD_ui_disk_update(this);
}
