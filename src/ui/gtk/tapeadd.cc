/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: tapeadd.cc,v 1.3 2002/06/09 14:24:34 torsten_paul Exp $
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

#include "cmd/cmd.h"

#include "ui/status.h"

#include "ui/gtk/cmd.h"
#include "ui/gtk/tapeadd.h"

TapeAddWindow::TapeAddWindow(void) : CMD("ui-tape-name-edit")
{
  register_cmd("ui-tape-name-edit");
}

TapeAddWindow::~TapeAddWindow(void)
{
}

void
TapeAddWindow::execute(CMD_Args *args, CMD_Context context)
{
  const char *filename, *text;

  _args = args;

  filename = _args->get_string_arg("tape-filename");
  if (!filename)
    return;

  text = _args->get_string_arg("tape-rename-title");
  if (!text)
    return;

  init(filename, text);
}

void
TapeAddWindow::ok(GtkWidget *widget, gpointer *data)
{
  CMD_Args *args;

  TapeAddWindow *self = (TapeAddWindow *)data;
  args = self->_args;

  args->set_string_arg("tape-filename", gtk_entry_get_text(GTK_ENTRY(self->_tape_add.tapename)));
  gtk_widget_destroy(self->_window);
  gtk_main_quit();

  args->call_callbacks("ui-tape-name-edit-CB-ok");
}

void
TapeAddWindow::cancel(GtkWidget *widget, gpointer *data)
{
  CMD_Args *args;

  TapeAddWindow *self = (TapeAddWindow *)data;
  args = self->_args;

  gtk_widget_destroy(self->_window);
  gtk_main_quit();

  args->call_callbacks("ui-tape-name-edit-CB-cancel");
}

void
TapeAddWindow::init(const char *tapename, const char *text)
{
  GtkWidget *w;

  /*
   *  window
   */
  _window = gtk_window_new(GTK_WINDOW_DIALOG);
  gtk_window_set_title(GTK_WINDOW(_window), _("KCemu: Rename File"));
  gtk_window_position(GTK_WINDOW(_window), GTK_WIN_POS_MOUSE);
  gtk_signal_connect(GTK_OBJECT(_window), "destroy",
                     GTK_SIGNAL_FUNC(gtk_widget_destroyed),
                     &_window);

  /*
   *  vbox
   */
  _tape_add.vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_border_width(GTK_CONTAINER(_tape_add.vbox), 6);
  gtk_container_add(GTK_CONTAINER(_window), _tape_add.vbox);
  gtk_widget_show(_tape_add.vbox);

  /*
   *  table
   */
  _tape_add.table = gtk_table_new(5, 3, FALSE);
  gtk_box_pack_start(GTK_BOX(_tape_add.vbox), _tape_add.table, FALSE, TRUE, 0);
  gtk_widget_show(_tape_add.table);

  /*
   *  label
   */
  w = gtk_label_new(text);
  gtk_table_attach_defaults(GTK_TABLE(_tape_add.table), w, 0, 2, 0, 1);
  gtk_misc_set_padding(GTK_MISC(w), 6, 6);
  gtk_misc_set_alignment(GTK_MISC(w), 0.0, 0.5);
  gtk_widget_show(w);

  /*
   *  tapename
   */
  w = gtk_label_new(_("Tapename:"));
  gtk_table_attach_defaults(GTK_TABLE(_tape_add.table), w, 0, 1, 1, 2);
  gtk_widget_show(w);
  _tape_add.tapename = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(_tape_add.tapename), tapename);
  gtk_signal_connect(GTK_OBJECT(_tape_add.tapename), "activate",
		     GTK_SIGNAL_FUNC(ok), this);
  gtk_table_attach_defaults(GTK_TABLE(_tape_add.table), _tape_add.tapename,
                            1, 2, 1, 2);
  gtk_entry_select_region(GTK_ENTRY(_tape_add.tapename), 0, -1);
  gtk_widget_grab_focus(_tape_add.tapename);
  gtk_widget_show(_tape_add.tapename);

  /*
   *  kc name
   */
  /*
    w = gtk_label_new(_("KC name:"));
    gtk_table_attach_defaults(GTK_TABLE(_tape_add.table), w, 0, 1, 2, 3);
    gtk_widget_show(w);
    _tape_add.kcname = gtk_entry_new();
    gtk_table_attach_defaults(GTK_TABLE(_tape_add.table), _tape_add.kcname, 1, 3, 2, 3);
    gtk_widget_show(_tape_add.kcname);
  */

  /*
   *  separator
   */
  _tape_add.separator = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(_tape_add.vbox), _tape_add.separator,
                     FALSE, FALSE, 5);
  gtk_widget_show(_tape_add.separator);

  /*
   *  button box
   */
  _tape_add.bbox = gtk_hbutton_box_new();
  //gtk_button_box_set_spacing(GTK_BUTTON_BOX(_tape_add.bbox), 0);
  gtk_button_box_set_layout(GTK_BUTTON_BOX(_tape_add.bbox), GTK_BUTTONBOX_SPREAD);
  //gtk_button_box_set_child_size(GTK_BUTTON_BOX(_tape_add.bbox), 0, 0);
  gtk_box_pack_start(GTK_BOX(_tape_add.vbox), _tape_add.bbox, FALSE, TRUE, 0);
  gtk_widget_show(_tape_add.bbox);
  
  /*
   *  ok button
   */
  _tape_add.b_ok = gtk_button_new_with_label(_("Ok"));
  gtk_container_add(GTK_CONTAINER(_tape_add.bbox), _tape_add.b_ok);
  gtk_signal_connect(GTK_OBJECT(_tape_add.b_ok), "clicked",
		     GTK_SIGNAL_FUNC(ok), this);
  GTK_WIDGET_SET_FLAGS(_tape_add.b_ok, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(_tape_add.b_ok);
  gtk_widget_show(_tape_add.b_ok);

  /*
   *  cancel button
   */
  _tape_add.b_cancel = gtk_button_new_with_label(_("Cancel"));
  gtk_container_add(GTK_CONTAINER(_tape_add.bbox), _tape_add.b_cancel);
  gtk_signal_connect(GTK_OBJECT(_tape_add.b_cancel), "clicked",
		     GTK_SIGNAL_FUNC(cancel), this);
  GTK_WIDGET_SET_FLAGS(_tape_add.b_cancel, GTK_CAN_DEFAULT);
  gtk_widget_show(_tape_add.b_cancel);

  gtk_window_set_modal(GTK_WINDOW(_window),TRUE);
  gtk_widget_show(_window);
  gtk_main();
}
