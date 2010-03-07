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

#include "cmd/cmd.h"

#include "ui/status.h"

#include "ui/gtk/cmd.h"
#include "ui/gtk/tapeadd.h"

TapeAddWindow::TapeAddWindow(const char *ui_xml_file) : UI_Gtk_Window(ui_xml_file), CMD("ui-tape-name-edit")
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
  gtk_widget_hide(self->_window);

  args->call_callbacks("ui-tape-name-edit-CB-ok");
}

void
TapeAddWindow::cancel(GtkWidget *widget, gpointer *data)
{
  CMD_Args *args;

  TapeAddWindow *self = (TapeAddWindow *)data;
  args = self->_args;

  gtk_widget_hide(self->_window);

  args->call_callbacks("ui-tape-name-edit-CB-cancel");
}

void
TapeAddWindow::init(void)
{
}

void
TapeAddWindow::init(const char *tapename, const char *text)
{
  /*
   *  window
   */
  _window = get_widget("tapeedit_window");
  gtk_window_set_title(GTK_WINDOW(_window), _("KCemu: Rename File"));
  gtk_signal_connect(GTK_OBJECT(_window), "delete_event",
		     GTK_SIGNAL_FUNC(cancel), this);
  
  /*
  gtk_signal_connect(GTK_OBJECT(_window), "destroy",
                     GTK_SIGNAL_FUNC(gtk_widget_destroyed),
                     &_window);
   */


  /*
   *  label
   */
  GtkWidget *label = get_widget("label_text");
  gtk_label_set_text(GTK_LABEL(label), text);

  _tape_add.tapename = get_widget("tapename_entry");
  gtk_entry_set_text(GTK_ENTRY(_tape_add.tapename), tapename);
  gtk_signal_connect(GTK_OBJECT(_tape_add.tapename), "activate",
		     GTK_SIGNAL_FUNC(ok), this);
  gtk_entry_select_region(GTK_ENTRY(_tape_add.tapename), 0, -1);
  gtk_widget_grab_focus(_tape_add.tapename);
  
  /*
   *  ok button
   */
  _tape_add.b_ok = get_widget("dialog_button_ok");
  gtk_signal_connect(GTK_OBJECT(_tape_add.b_ok), "clicked",
		     GTK_SIGNAL_FUNC(ok), this);

  /*
   *  cancel button
   */
  _tape_add.b_cancel = get_widget("dialog_button_cancel");
  gtk_signal_connect(GTK_OBJECT(_tape_add.b_cancel), "clicked",
		     GTK_SIGNAL_FUNC(cancel), this);
  
  init_dialog(NULL, NULL);

  gtk_widget_show(_window);
}
