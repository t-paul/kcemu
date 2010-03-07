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

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "kc/system.h"

#include "ui/error.h"
#include "ui/gtk/hedit.h"

EditHeaderWindow::EditHeaderWindow(void) : CMD("ui-tape-header-edit")
{
  register_cmd("ui-tape-header-edit");
}

EditHeaderWindow::~EditHeaderWindow(void)
{
}

void
EditHeaderWindow::execute(CMD_Args *args, CMD_Context context)
{
  int a;
  char buf[10];
  const char *name, *ptr;

  if (args == NULL)
    return;

  _args = args;
  name = args->get_string_arg("filename");
  if (name == NULL)
    return;

  ptr = strrchr(name, '/');
  if (ptr)
    {
      ptr++;
      if (*ptr != '\0')
        name = ptr;
    }

  if (!name)
    name = "";

  a = 0;
  ptr = name;
  while (242)
    {
      buf[a++] = toupper(*ptr++);
      if (*ptr == '\0')
        break;
      if (a == 8)
        break;
    }
  buf[a] = '\0';
                       
  init(name, buf);
}

bool
EditHeaderWindow::check_values(CMD_Args *args)
{
  const char *load, *start;
  char *l_endptr, *s_endptr;

  if (!args)
    return false;
  _tape_filename = get_tape_filename();
  _kc_filename = get_kc_filename();
  load = get_load_address();
  start = get_start_address();
  _load_address = strtoul(load, &l_endptr, 0);
  _start_address = strtoul(start, &s_endptr, 0);

  if (strlen(_tape_filename) < 1)
    {
      Error::instance()->info(_("no tape-filename given!"));
      return false;
    }
  if (strlen(_kc_filename) < 1)
    {
      Error::instance()->info(_("no KC-filename given!"));
      return false;
    }
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
  if (!_autostart)
    return true;
  if (!((*start != '\0') && (*s_endptr == '\0')))
    {
      Error::instance()->info(_("error in given start address!"));
      return false;
    }

  return true;
}

void
EditHeaderWindow::ok(GtkWidget * /* widget */, gpointer data)
{
  EditHeaderWindow *self = (EditHeaderWindow *)data;
  if (!self->check_values(self->_args))
    return;
  self->_args->set_string_arg("tape-filename", self->_tape_filename);
  self->_args->set_string_arg("kc-filename", self->_kc_filename);
  self->_args->set_long_arg("load-address", self->_load_address);
  self->_args->set_long_arg("start-address", self->_start_address);
  gtk_widget_destroy(self->_w.window);
  self->_args->call_callbacks("ui-edit-header-CB");
}

void
EditHeaderWindow::cancel(GtkWidget * /* widget */, gpointer data)
{
  EditHeaderWindow *self = (EditHeaderWindow *)data;
  gtk_widget_destroy(self->_w.window);
  self->_args->call_callbacks("ui-edit-header-CB");
}

int
EditHeaderWindow::delete_event(GtkWidget *widget,
                               GdkEvent *event,
                               gpointer data)
{
  cancel(widget, data);
  return true;
}

void
EditHeaderWindow::sf_toggle_autostart(GtkWidget *w, gpointer data)
{
  EditHeaderWindow *self = (EditHeaderWindow *)data;
  self->_autostart = GTK_TOGGLE_BUTTON(w)->active;
  gtk_widget_set_sensitive(self->_w.start_addr, self->_autostart);
}

void
EditHeaderWindow::init(const char *tapename, const char *kcname)
{
  GtkWidget *w;
  
  /*
   *  window
   */
  _w.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(_w.window), _("File Properties"));
  gtk_window_position(GTK_WINDOW(_w.window), GTK_WIN_POS_MOUSE);
  gtk_signal_connect(GTK_OBJECT(_w.window), "delete_event",
                     GTK_SIGNAL_FUNC(delete_event), this);

  /*
   *  vbox
   */
  _w.vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_border_width(GTK_CONTAINER(_w.vbox), 6);
  gtk_container_add(GTK_CONTAINER(_w.window), _w.vbox);
  gtk_widget_show(_w.vbox);

  /*
   *  table
   */
  _w.table = gtk_table_new(5, 3, FALSE);
  gtk_table_set_row_spacings(GTK_TABLE(_w.table), 2);
  gtk_table_set_col_spacings(GTK_TABLE(_w.table), 4);
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.table, FALSE, TRUE, 0);
  gtk_widget_show(_w.table);

  /*
   *  tapename
   */
  w = gtk_label_new(_("Name in tape-directory:"));
  gtk_table_attach_defaults(GTK_TABLE(_w.table), w,
                            0, 1, 1, 2);
  gtk_widget_show(w);
  _w.tapename = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(_w.tapename), tapename);
  gtk_table_attach_defaults(GTK_TABLE(_w.table), _w.tapename,
                            1, 3, 1, 2);
  gtk_widget_show(_w.tapename);

  /*
   *  kc name
   */
  w = gtk_label_new(_("Name in KC file-header:"));
  gtk_table_attach_defaults(GTK_TABLE(_w.table), w,
                            0, 1, 2, 3);
  gtk_widget_show(w);
  _w.kcname = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(_w.kcname), kcname);
  gtk_table_attach_defaults(GTK_TABLE(_w.table), _w.kcname,
                            1, 3, 2, 3);
  gtk_widget_show(_w.kcname);

  /*
   *  load addr
   */
  w = gtk_label_new(_("Load address:"));
  gtk_misc_set_alignment(GTK_MISC(w), 1.0, 0.5);
  gtk_table_attach_defaults(GTK_TABLE(_w.table), w,
                            0, 1, 3, 4);
  gtk_widget_show(w);
  _w.load_addr = gtk_entry_new();
  gtk_table_attach_defaults(GTK_TABLE(_w.table), _w.load_addr,
                            1, 3, 3, 4);
  gtk_widget_show(_w.load_addr);

  /*
   *  start addr
   */
  w = gtk_label_new(_("Start address:"));
  gtk_misc_set_alignment(GTK_MISC(w), 1.0, 0.5);
  gtk_table_attach_defaults(GTK_TABLE(_w.table), w,
                            0, 1, 4, 5);
  gtk_widget_show(w);
  _w.start_addr = gtk_entry_new();
  gtk_table_attach_defaults(GTK_TABLE(_w.table), _w.start_addr,
                            1, 3, 4, 5);
  gtk_widget_show(_w.start_addr);

  /*
   *  autostart
   */
  w = gtk_check_button_new_with_label(_("autostart"));
  gtk_table_attach_defaults(GTK_TABLE(_w.table), w,
                            1, 3, 5, 6);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), TRUE);
  gtk_signal_connect(GTK_OBJECT(w), "toggled",
                     GTK_SIGNAL_FUNC(sf_toggle_autostart), this);
  gtk_widget_show(w);
                           

  /*
   *  separator
   */
  _w.separator = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.separator,
                     FALSE, FALSE, 5);
  gtk_widget_show(_w.separator);

  /*
   *  button box
   */
  _w.bbox = gtk_hbutton_box_new();
  //gtk_button_box_set_spacing(GTK_BUTTON_BOX(_w.bbox), 0);
  gtk_button_box_set_layout(GTK_BUTTON_BOX(_w.bbox), GTK_BUTTONBOX_SPREAD);
  //gtk_button_box_set_child_size(GTK_BUTTON_BOX(_w.bbox), 0, 0);
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.bbox, FALSE, TRUE, 0);
  gtk_widget_show(_w.bbox);
  
  /*
   *  ok button
   */
  _w.b_ok = gtk_button_new_with_label("ok");
  gtk_container_add(GTK_CONTAINER(_w.bbox), _w.b_ok);
  gtk_signal_connect(GTK_OBJECT(_w.b_ok), "clicked",
                     GTK_SIGNAL_FUNC(ok), this);
  GTK_WIDGET_SET_FLAGS(_w.b_ok, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(_w.b_ok);
  gtk_widget_show(_w.b_ok);

  /*
   *  close button
   */
  _w.b_close = gtk_button_new_with_label("close");
  gtk_container_add(GTK_CONTAINER(_w.bbox), _w.b_close);
  gtk_signal_connect(GTK_OBJECT(_w.b_close), "clicked",
                     GTK_SIGNAL_FUNC(cancel), this);
  GTK_WIDGET_SET_FLAGS(_w.b_close, GTK_CAN_DEFAULT);
  gtk_widget_show(_w.b_close);

  gtk_widget_show(_w.window);
}
