/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-1998 Torsten Paul
 *
 *  $Id: disk.cc,v 1.1 2000/07/08 18:05:54 tp Exp $
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
    
    filename = NULL;
    switch (context)
      {
      case 0:
      case 1:
      case 2:
      case 3:
	if (!args)
	  args = new CMD_Args();
	
	args->set_int_arg("disk", context);
	CMD_EXEC_ARGS("disk-attach", args);
	break;
      case 4:
      case 5:
      case 6:
      case 7:
	if (!args)
	  args = new CMD_Args();
	
	args->set_int_arg("disk", context & 3);
	CMD_EXEC_ARGS("disk-detach", args);
	break;
      }
  }
};
  
void
DiskWindow::set_name(int idx, const char *name)
{
  if (!name)
    name = "";

  gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(_w.combo[idx])->entry), name);
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
  char e1[] = "/home/tp/projects/KCemu/DISK/caos.dump";
  char e2[] = "/home/tp/projects/KCemu/DISK/microdos.dump";
  char e3[] = "Disk 3";
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
    popdown = g_list_append(popdown, e1);
    popdown = g_list_append(popdown, e2);
    popdown = g_list_append(popdown, e3);
    _w.combo[a] = gtk_combo_new();
    gtk_table_attach_defaults(GTK_TABLE(_w.table), _w.combo[a],
			      1, 2, a, a + 1);
    gtk_combo_set_popdown_strings(GTK_COMBO(_w.combo[a]), popdown);
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(_w.combo[a])->entry), "hello!");
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
  }

  _w.separator = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.separator,
		     FALSE, FALSE, 5);
  gtk_widget_show(_w.separator);
    
  /*
   *  button box
   */
  _w.bbox = gtk_hbutton_box_new();
  gtk_button_box_set_spacing(GTK_BUTTON_BOX(_w.bbox), 0);
  gtk_button_box_set_layout(GTK_BUTTON_BOX(_w.bbox), GTK_BUTTONBOX_SPREAD);
  gtk_button_box_set_child_size(GTK_BUTTON_BOX(_w.bbox), 0, 0);
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.bbox, FALSE, TRUE, 4);
  gtk_widget_show(_w.bbox);

  /*
   *  ok button
   */
  _w.b_ok = gtk_button_new_with_label(_("Ok"));
  gtk_signal_connect(GTK_OBJECT(_w.b_ok), "clicked",
                     GTK_SIGNAL_FUNC(cmd_exec_sf),
                     (gpointer)"ui-disk-window-toggle");
  gtk_container_add(GTK_CONTAINER(_w.bbox), _w.b_ok);
  GTK_WIDGET_SET_FLAGS(_w.b_ok, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(_w.b_ok);
  gtk_widget_show(_w.b_ok);

  /*
   *  apply button
   */
  _w.b_apply = gtk_button_new_with_label(_("Apply"));
  gtk_signal_connect(GTK_OBJECT(_w.b_apply), "clicked",
                     GTK_SIGNAL_FUNC(cmd_exec_sf),
                     (gpointer)"ui-disk-window-toggle");
  gtk_container_add(GTK_CONTAINER(_w.bbox), _w.b_apply);
  GTK_WIDGET_SET_FLAGS(_w.b_apply, GTK_CAN_DEFAULT);
  gtk_widget_show(_w.b_apply);

  /*
   *  close button
   */
  _w.b_close = gtk_button_new_with_label(_("Close"));
  gtk_signal_connect(GTK_OBJECT(_w.b_close), "clicked",
                     GTK_SIGNAL_FUNC(cmd_exec_sf),
                     (gpointer)"ui-disk-window-toggle");
  gtk_container_add(GTK_CONTAINER(_w.bbox), _w.b_close);
  GTK_WIDGET_SET_FLAGS(_w.b_close, GTK_CAN_DEFAULT);
  gtk_widget_show(_w.b_close);

  CMD *cmd;
  cmd = new CMD_ui_disk_window_toggle(this);
  cmd = new CMD_ui_disk_attach(this);
  cmd = new CMD_ui_disk_update(this);
}
