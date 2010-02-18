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

#include <stdlib.h>

#include <gtk/gtk.h>

#include "kc/system.h"

#include "ui/error.h"
#include "ui/gtk/image.h"

CMD_ui_image_props::CMD_ui_image_props(void) : CMD("ui-image-get-properties")
{
  register_cmd("ui-image-get-properties", 0);
}

CMD_ui_image_props::~CMD_ui_image_props(void)
{
}

void
CMD_ui_image_props::ok(GtkWidget * /* widget */, gpointer data)
{
  CMD_ui_image_props *self = (CMD_ui_image_props *)data;
  if (!self->check_values(self->_args))
    return;
  gtk_widget_destroy(self->_w.dialog);
  self->_args->call_callbacks("ui-image-get-properties-CB-ok");
}

void
CMD_ui_image_props::cancel(GtkWidget * /* widget */, gpointer data)
{
  CMD_ui_image_props *self = (CMD_ui_image_props *)data;
  gtk_widget_destroy(self->_w.dialog);
  self->_args->call_callbacks("ui-image-get-properties-CB-cancel");
}

int
CMD_ui_image_props::delete_event(GtkWidget *widget,
                                 GdkEvent *event,
                                 gpointer data)
{
  cancel(widget, data);
  return true;
}

void
CMD_ui_image_props::toggle_autostart(GtkWidget *w, gpointer data)
{
  CMD_ui_image_props *self = (CMD_ui_image_props *)data;
  self->_autostart = GTK_TOGGLE_BUTTON(w)->active;
  gtk_widget_set_sensitive(self->_w.start_addr, self->_autostart);
}

bool
CMD_ui_image_props::check_values(CMD_Args *args)
{
  const char *load, *start;
  char *l_endptr, *s_endptr;
  unsigned long load_address, start_address;

  if (!args)
    return false;
  load = gtk_entry_get_text(GTK_ENTRY(_w.load_addr));
  start = gtk_entry_get_text(GTK_ENTRY(_w.start_addr));
  load_address = strtoul(load, &l_endptr, 0);
  start_address = strtoul(start, &s_endptr, 0);

  if (!((*load != '\0') && (*l_endptr == '\0')))
    {
      Error::instance()->info(_("error in given load address!\n\n"
                                "valid addresses have to be in the "
                                "range from 0 to 65535\n"
                                "hexadecimal values need to be "
                                "prefixed with 0x (e.g. 0x1234),\n"
                                "octal values require a leading 0 "
                                "(e.g. 0444)"));
      return false;
    }
  _args->set_long_arg("load-address", load_address);

  if (!_autostart)
    return true;

  if (!((*start != '\0') && (*s_endptr == '\0')))
    {
      Error::instance()->info(_("error in given start address!"));
      return false;
    }
  _args->set_long_arg("start-address", start_address);

  return true;
}

void
CMD_ui_image_props::execute(CMD_Args *args, CMD_Context context)
{
  GtkWidget *button, *label, *table;

  _args = args;

  /*
   *  dialog
   */
  _w.dialog = gtk_dialog_new();
  gtk_signal_connect(GTK_OBJECT (_w.dialog), "delete_event",
                     GTK_SIGNAL_FUNC(delete_event), this);
  gtk_window_set_title(GTK_WINDOW(_w.dialog), _("Input Image Properties"));
  gtk_window_position(GTK_WINDOW(_w.dialog), GTK_WIN_POS_MOUSE);
  gtk_container_set_border_width(GTK_CONTAINER(_w.dialog), 0);

  /*
   *  label
   */
  label = gtk_label_new(_("It was not possible to automatically determine the\n"
                          "type of the selected file. Thus I don't know at which\n"
                          "address the file should be loaded\n\n"
                          "To load the file nevertheless you need to type in the\n"
                          "needed values yourself."));
  gtk_misc_set_padding(GTK_MISC(label), 10, 10);
  gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(_w.dialog)->vbox), 
                     label, TRUE, TRUE, 0);
  gtk_widget_show(label);

  /*
   *  table
   */
  table = gtk_table_new(3, 2, FALSE);
  gtk_table_set_row_spacings(GTK_TABLE(table), 2);
  gtk_table_set_col_spacings(GTK_TABLE(table), 4);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(_w.dialog)->vbox), table, FALSE, TRUE, 0);
  gtk_widget_show(table);

  /*
   *  load addr
   */
  label = gtk_label_new(_("Load address:"));
  gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
  gtk_table_attach_defaults(GTK_TABLE(table), label,
                            0, 1, 0, 1);
  gtk_widget_show(label);
  _w.load_addr = gtk_entry_new();
  gtk_table_attach_defaults(GTK_TABLE(table), _w.load_addr,
                            1, 2, 0, 1);
  gtk_widget_show(_w.load_addr);
  
  /*
   *  start addr
   */
  label = gtk_label_new(_("Start address:"));
  gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
  gtk_table_attach_defaults(GTK_TABLE(table), label,
                            0, 1, 1, 2);
  gtk_widget_show(label);
  _w.start_addr = gtk_entry_new();
  gtk_table_attach_defaults(GTK_TABLE(table), _w.start_addr,
                            1, 2, 1, 2);
  gtk_widget_show(_w.start_addr);

  /*
   *  autostart
   */
  _autostart = false;
  if (args && args->get_long_arg("auto-start"))
    _autostart = true;
  button = gtk_check_button_new_with_label(_("Start Program after Loading"));
  gtk_table_attach_defaults(GTK_TABLE(table), button,
                            1, 2, 2, 3);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), _autostart);
  gtk_widget_set_sensitive(_w.start_addr, _autostart);
  gtk_signal_connect(GTK_OBJECT(button), "toggled",
                     GTK_SIGNAL_FUNC(toggle_autostart), this);
  gtk_widget_show(button);

  /*
   *  ok button
   */
  button = gtk_button_new_with_label(_("OK"));
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(_w.dialog)->action_area),
                     button, TRUE, TRUE, 0);
  gtk_signal_connect(GTK_OBJECT(button), "clicked",
                     GTK_SIGNAL_FUNC(ok), this);
  gtk_widget_grab_default(button);
  gtk_widget_show(button);

  /*
   *  cancel button
   */
  button = gtk_button_new_with_label(_("Cancel"));
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(_w.dialog)->action_area),
                     button, TRUE, TRUE, 0);
  gtk_signal_connect(GTK_OBJECT(button), "clicked",
                     GTK_SIGNAL_FUNC(cancel), this);
  gtk_widget_show(button);
  
  /*
   *  show dialog now
   */
  gtk_widget_show(_w.dialog);
}
