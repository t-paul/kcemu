/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-1998 Torsten Paul
 *
 *  $Id: dialog.cc,v 1.3 2001/01/05 18:17:51 tp Exp $
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

#include "ui/gtk/dialog.h"

DialogWindow::DialogWindow(void) : CMD("ui-dialog-yes-no")
{
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
  const char *filename;

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
  gtk_widget_destroy(self->_dialog);
  self->_args->call_callbacks("ui-dialog-ok-CB");
}

void
DialogWindow::yes(GtkWidget */*widget*/, gpointer data)
{
  DialogWindow *self = (DialogWindow *)data;
  gtk_widget_destroy(self->_dialog);
  self->_args->call_callbacks("ui-dialog-yes-no-CB-yes");
}

void
DialogWindow::no(GtkWidget */*widget*/, gpointer data)
{
  DialogWindow *self = (DialogWindow *)data;
  gtk_widget_destroy(self->_dialog);
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

GtkWidget *
DialogWindow::init_misc(const char *title, const char *text)
{
  GtkWidget *label, *bbox;

  /*
   *  dialog
   */
  _dialog = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(_dialog), title);
  gtk_window_position(GTK_WINDOW(_dialog), GTK_WIN_POS_MOUSE);
  gtk_container_set_border_width(GTK_CONTAINER(_dialog), 6);

  /*
   *  label
   */
  label = gtk_label_new(text);
  gtk_misc_set_padding(GTK_MISC(label), 10, 10);
  gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(_dialog)->vbox), 
                     label, TRUE, TRUE, 0);
  gtk_widget_show(label);

  /*
   *  bbox
   */
  bbox = gtk_hbutton_box_new();
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(_dialog)->action_area), bbox,
                     FALSE, TRUE, 0);
  gtk_widget_show(bbox);

  return bbox;
}

void
DialogWindow::init_ok(const char *title, const char *text)
{
  GtkWidget *button, *bbox;
  
  bbox = init_misc(title, text);
  gtk_signal_connect(GTK_OBJECT(_dialog), "delete_event",
                     GTK_SIGNAL_FUNC(delete_event_ok), this);

  /*
   *  OK button
   */
  button = gtk_button_new_with_label(_("OK"));
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_container_add(GTK_CONTAINER(bbox), button);
  gtk_signal_connect(GTK_OBJECT(button), "clicked",
                     GTK_SIGNAL_FUNC(ok), this);
  gtk_widget_grab_default(button);
  gtk_widget_show(button);

  gtk_grab_add(_dialog);
  gtk_widget_show(_dialog);
}

void
DialogWindow::init_yes_no(const char *title, const char *text)
{
  GtkWidget *button, *bbox;

  bbox = init_misc(title, text);
  gtk_signal_connect(GTK_OBJECT(_dialog), "delete_event",
                     GTK_SIGNAL_FUNC(delete_event_yes_no), this);

  /*
   *  yes button
   */
  button = gtk_button_new_with_label(_("Yes"));
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_container_add(GTK_CONTAINER(bbox), button);
  gtk_signal_connect(GTK_OBJECT(button), "clicked",
                     GTK_SIGNAL_FUNC(yes), this);
  gtk_widget_grab_default(button);
  gtk_widget_show(button);

  /*
   *  no button
   */
  button = gtk_button_new_with_label(_("No"));
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_container_add(GTK_CONTAINER(bbox), button);
  gtk_signal_connect(GTK_OBJECT(button), "clicked",
                     GTK_SIGNAL_FUNC(no), this);
  gtk_widget_show(button);

  gtk_grab_add(_dialog);
  gtk_widget_show(_dialog);
}
