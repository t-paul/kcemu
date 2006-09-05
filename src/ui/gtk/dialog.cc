/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2006 Torsten Paul
 *
 *  $Id: dialog.cc,v 1.6 2002/10/31 01:38:12 torsten_paul Exp $
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

#include "kc/system.h"

#include "ui/gtk/dialog.h"
#include "ui/gtk/glade/interface.h"
#include "ui/gtk/glade/glade_util.h"

DialogWindow::DialogWindow(void) : CMD("ui-dialog-yes-no")
{
  _args = NULL;
  register_cmd("ui-dialog-ok", 0);
  register_cmd("ui-dialog-yes-no", 1);
}

DialogWindow::~DialogWindow(void)
{
}

void
DialogWindow::execute(CMD_Args *args, CMD_Context context)
{
  char *text;
  const char *title;
  const char *text_arg;
  const char *text_val;
  const char *text_arg_val;

  _args = args;
  text = NULL;
  text_val = NULL;
  text_arg = NULL;
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
              text = new char[strlen(text_val) + strlen(text_arg_val) + 1];
              sprintf(text, text_val, text_arg_val);
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
  default:
    break;
  }
}

void
DialogWindow::ok(GtkWidget */*widget*/, gpointer data)
{
  DialogWindow *self = (DialogWindow *)data;
  gtk_widget_destroy(self->_window);

  if (self->_args != NULL)
    self->_args->call_callbacks("ui-dialog-ok-CB");
}

void
DialogWindow::yes(GtkWidget */*widget*/, gpointer data)
{
  DialogWindow *self = (DialogWindow *)data;
  gtk_widget_destroy(self->_window);

  if (self->_args != NULL)
    self->_args->call_callbacks("ui-dialog-yes-no-CB-yes");
}

void
DialogWindow::no(GtkWidget */*widget*/, gpointer data)
{
  DialogWindow *self = (DialogWindow *)data;
  gtk_widget_destroy(self->_window);

  if (self->_args != NULL)
    self->_args->call_callbacks("ui-dialog-yes-no-CB-no");
}

int
DialogWindow::delete_event_yes_no(GtkWidget *widget, GdkEvent *event, gpointer data)
{
  no(widget, data);
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
}

void
DialogWindow::init_misc(const char *title, const char *text)
{
  _window = create_dialog_window();

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
  init_misc(title, text);
  g_signal_connect(G_OBJECT(_window), "delete_event", G_CALLBACK(delete_event_ok), this);

  /*
   *  image
   */
  GtkWidget *image = get_widget("header_image_info");
  gtk_widget_show(image);

  /*
   *  ok button
   */
  GtkWidget *button_ok = get_widget("dialog_button_ok");
  gtk_widget_show(button_ok);
  g_signal_connect(G_OBJECT(button_ok), "clicked", G_CALLBACK(ok), this);

  gtk_widget_show(_window);
}

void
DialogWindow::init_yes_no(const char *title, const char *text)
{
  init_misc(title, text);
  g_signal_connect(G_OBJECT(_window), "delete_event", G_CALLBACK(delete_event_yes_no), this);

  /*
   *  image
   */
  GtkWidget *image = get_widget("header_image_question");
  gtk_widget_show(image);

  /*
   *  yes button
   */
  GtkWidget *button_yes = get_widget("dialog_button_yes");
  gtk_widget_show(button_yes);
  g_signal_connect(G_OBJECT(button_yes), "clicked", G_CALLBACK(yes), this);

  /*
   *  no button
   */
  GtkWidget *button_no = get_widget("dialog_button_no");
  gtk_widget_show(button_no);
  g_signal_connect(G_OBJECT(button_no), "clicked", G_CALLBACK(no), this);

  gtk_widget_show(_window);
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
