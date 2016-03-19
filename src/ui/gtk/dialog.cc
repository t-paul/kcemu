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

#include <string.h>

#include "kc/system.h"

#include "ui/gtk/dialog.h"

DialogWindow::DialogWindow(const char *ui_xml_file) : UI_Gtk_Window(ui_xml_file), CMD("ui-dialog-yes-no")
{
  _args = NULL;
  _delete_handler_id = 0;
  register_cmd("ui-dialog-ok", 0);
  register_cmd("ui-dialog-yes-no", 1);
  register_cmd("ui-dialog-yes-no-cancel", 2);
  
  init();
}

DialogWindow::~DialogWindow(void)
{
}

void
DialogWindow::execute(CMD_Args *args, CMD_Context context)
{
  char * textbuffer;
  const char *text;
  const char *title;
  const char *text_arg;
  const char *text_val;
  const char *text_arg_val;

  _args = args;
  text = NULL;
  text_val = NULL;
  text_arg = NULL;
  textbuffer = NULL;
  title = _("???");
  if (_args)
    {
      text_val = _args->get_string_arg("ui-dialog-text");
      text_arg = _args->get_string_arg("ui-dialog-text-arg");
      title = _args->get_string_arg("ui-dialog-title");
      if (text_arg)
        {
          text_arg_val = _args->get_string_arg(text_arg);
          if (text_arg_val)
            {
              textbuffer = new char[strlen(text_val) + strlen(text_arg_val) + 1];
              sprintf(textbuffer, text_val, text_arg_val);
              text = textbuffer;
            }
        }
      if (text == NULL)
        text = (char *)text_val; // FIXME:
    }
  if (text == NULL)
    text = "";

  switch (context) {
  case 0:
    init_ok(title, text);
    break;
  case 1:
    init_yes_no(title, text);
    break;
  case 2:
    init_yes_no_cancel(title, text);
    break;
  default:
    break;
  }
  delete[] textbuffer;
}

void
DialogWindow::ok(GtkWidget */*widget*/, gpointer data)
{
  DialogWindow *self = (DialogWindow *)data;

  self->_dialog_result = GTK_RESPONSE_OK;
  
  if (self->_args != NULL)
    self->_args->call_callbacks("ui-dialog-ok-CB");
  
  self->hide();
}

void
DialogWindow::cancel(GtkWidget */*widget*/, gpointer data)
{
  DialogWindow *self = (DialogWindow *)data;

  self->_dialog_result = GTK_RESPONSE_CANCEL;

  if (self->_args != NULL)
    self->_args->call_callbacks("ui-dialog-cancel-CB");

  self->hide();
}

void
DialogWindow::yes(GtkWidget */*widget*/, gpointer data)
{
  DialogWindow *self = (DialogWindow *)data;

  self->_dialog_result = GTK_RESPONSE_YES;

  if (self->_args != NULL)
    self->_args->call_callbacks("ui-dialog-yes-no-CB-yes");

  self->hide();
}

void
DialogWindow::no(GtkWidget */*widget*/, gpointer data)
{
  DialogWindow *self = (DialogWindow *)data;

  self->_dialog_result = GTK_RESPONSE_NO;

  if (self->_args != NULL)
    self->_args->call_callbacks("ui-dialog-yes-no-CB-no");

  self->hide();
}

int
DialogWindow::delete_event_yes_no(GtkWidget *widget, GdkEvent *event, gpointer data)
{
  no(widget, data);
  return true;
}

int
DialogWindow::delete_event_yes_no_cancel(GtkWidget *widget, GdkEvent *event, gpointer data)
{
  cancel(widget, data);
  return true;
}

int
DialogWindow::delete_event_ok(GtkWidget *widget, GdkEvent *event, gpointer data)
{
  ok(widget, data);
  return true;
}

void
DialogWindow::init(void)
{
  _w.button_ok = get_widget("dialog_button_ok");
  _w.button_cancel = get_widget("dialog_button_cancel");
  _w.button_yes = get_widget("dialog_button_yes");
  _w.button_no = get_widget("dialog_button_no");

  _w.image_info = get_widget("header_image_info");
  _w.image_question = get_widget("header_image_question");

  g_signal_connect(_w.button_ok, "clicked", G_CALLBACK(ok), this);
  g_signal_connect(_w.button_cancel, "clicked", G_CALLBACK(cancel), this);
  g_signal_connect(_w.button_yes, "clicked", G_CALLBACK(yes), this);
  g_signal_connect(_w.button_no, "clicked", G_CALLBACK(no), this);

  gtk_widget_hide(_w.button_ok);
  gtk_widget_hide(_w.button_cancel);
  gtk_widget_hide(_w.button_yes);
  gtk_widget_hide(_w.button_no);

  gtk_widget_hide(_w.image_info);
  gtk_widget_hide(_w.image_question);
}

void
DialogWindow::show_buttons(bool button_ok, bool button_cancel, bool button_yes, bool button_no) {
  if (button_ok)
    gtk_widget_show(_w.button_ok);
  if (button_cancel)
    gtk_widget_show(_w.button_cancel);
  if (button_yes)
    gtk_widget_show(_w.button_yes);
  if (button_no)
    gtk_widget_show(_w.button_no);
}

void
DialogWindow::init_misc(const char *title, const char *text, GCallback callback)
{
  _dialog_result = GTK_RESPONSE_NONE;

  _window = get_widget("dialog_window");
  if (_delete_handler_id != 0) {
      g_signal_handler_disconnect(_window, _delete_handler_id);
  }
  _delete_handler_id = g_signal_connect(_window, "delete_event", callback, this);

  gtk_window_set_title(GTK_WINDOW(_window), title);

  GtkWidget *header_label = get_widget("header_label");
  g_assert(GTK_IS_LABEL(header_label));
  gtk_label_set_text(GTK_LABEL(header_label), title);

  GtkWidget *main_textview = get_widget("main_textview");
  g_assert(GTK_IS_TEXT_VIEW(main_textview));
  GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(main_textview));
  gtk_text_buffer_set_text(buf, text, -1);

  init_dialog(NULL, NULL);
}

void
DialogWindow::init_ok(const char *title, const char *text)
{
  init_misc(title, text, G_CALLBACK(delete_event_ok));
  show_buttons(true, false, false, false);
  gtk_widget_show(_w.image_info);
  show();
}

void
DialogWindow::init_yes_no(const char *title, const char *text)
{
  init_misc(title, text, G_CALLBACK(delete_event_yes_no));
  show_buttons(false, false, true, true);
  gtk_widget_show(_w.image_question);
  show();
}

void
DialogWindow::init_yes_no_cancel(const char *title, const char *text)
{
  init_misc(title, text, G_CALLBACK(delete_event_yes_no_cancel));
  show_buttons(false, true, true, true);
  gtk_widget_show(_w.image_question);
  show();
}

void
DialogWindow::show_dialog_ok(const char *title, const char *text)
{
  _args = NULL;
  init_ok(title, text);
}

void
DialogWindow::show_dialog_yes_no(const char *title, const char *text)
{
  _args = NULL;
  init_yes_no(title, text);
}

void
DialogWindow::show_dialog_yes_no_cancel(const char *title, const char *text)
{
  _args = NULL;
  init_yes_no_cancel(title, text);
}

int
DialogWindow::get_dialog_result(void)
{
  return _dialog_result;
}
