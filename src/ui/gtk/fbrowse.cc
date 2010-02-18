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

#include "kc/system.h"

#include "ui/gtk/fbrowse.h"

FileBrowser::FileBrowser(void) : CMD("ui-file-select")
{
  register_cmd("ui-file-select");
}

FileBrowser::~FileBrowser(void)
{
}

void
FileBrowser::execute(CMD_Args *args, CMD_Context context)
{
  const char *title, *arg_title, *path;

  _args = args;
  title = _("Select File...");
  if (_args)
    arg_title = _args->get_string_arg("ui-file-select-title");
  if (arg_title)
    title = arg_title;

  path = NULL;
  if (_args)
    path = _args->get_string_arg("ui-file-select-path");

  bool dironly = _args && _args->get_long_arg("ui-file-select-dir-only");

  init(title, path, dironly);
}

void
FileBrowser::sf_response(GtkDialog *dialog, gint response_id, gpointer user_data)
{
  FileBrowser *self = (FileBrowser *)user_data;

  if (response_id == GTK_RESPONSE_ACCEPT)
    {
      char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
      self->_args->set_string_arg("filename", filename);
      g_free(filename);
      self->_args->call_callbacks("ui-file-select-CB-ok");
    }
  else
    {
      self->_args->call_callbacks("ui-file-select-CB-cancel");
    }
  gtk_widget_destroy(GTK_WIDGET(dialog));
}

void
FileBrowser::init(const char *title, const char *path, bool dironly)
{
  GtkFileChooserAction action = dironly ? GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER : GTK_FILE_CHOOSER_ACTION_OPEN;
  GtkWidget *filechooser = gtk_file_chooser_dialog_new(title, NULL, action,
                                                       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                       GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                                       NULL);

  gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(filechooser), TRUE);

  if (path)
    gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(filechooser), path);

  g_signal_connect(filechooser, "response", G_CALLBACK(sf_response), this);
  gtk_widget_show(filechooser);
}
