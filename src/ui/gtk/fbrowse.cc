/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: fbrowse.cc,v 1.4 2001/04/14 15:16:55 tp Exp $
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

#include "kc/config.h"
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

  init(title, path);
}

void
FileBrowser::ok(GtkWidget */*widget*/, GtkFileSelection *fs)
{
  FileBrowser *self = (FileBrowser *)gtk_object_get_user_data(GTK_OBJECT(fs));
  self->_args->set_string_arg("filename", gtk_file_selection_get_filename(fs));
  gtk_widget_destroy(GTK_WIDGET(fs));
  self->_args->call_callbacks("ui-file-select-CB-ok");
}

void
FileBrowser::cancel(GtkWidget */*widget*/, GtkFileSelection *fs)
{
  FileBrowser *self = (FileBrowser *)gtk_object_get_user_data(GTK_OBJECT(fs));
  gtk_widget_destroy(GTK_WIDGET(fs));
  self->_args->call_callbacks("ui-file-select-CB-cancel");
}

void
FileBrowser::init(const char *title, const char *path)
{
  _w = gtk_file_selection_new(title);
  if (path)
    gtk_file_selection_set_filename(GTK_FILE_SELECTION(_w), path);
  gtk_object_set_user_data(GTK_OBJECT(_w), this);
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(_w)->ok_button),
                     "clicked",
                     GTK_SIGNAL_FUNC(ok),
                     _w);
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(_w)->cancel_button),
                     "clicked",
                     GTK_SIGNAL_FUNC(cancel),
                     _w);
  gtk_grab_add(_w);
  gtk_widget_show(_w);
}
