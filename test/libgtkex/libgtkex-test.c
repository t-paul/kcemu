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

#include <glib-2.0/glib/gmain.h>

#include "libgtkex/libgtkex.h"

static guint statusid = 0;
static GtkWidget *statusbar = NULL;

static gboolean
on_timeout(gpointer data)
{
  guint id = (guint) data;
  if (statusid == id)
    gtk_statusbar_pop(GTK_STATUSBAR(statusbar), 0);

  return FALSE;
}

static void
log_text(const char *text)
{
  gtk_statusbar_pop(GTK_STATUSBAR(statusbar), 0);
  gtk_statusbar_push(GTK_STATUSBAR(statusbar), 0, text);

  printf("LOG: %s\n", text);

  statusid++;
  g_timeout_add_seconds(5, on_timeout, (gpointer)statusid);
}

static void
on_filebutton_changed(GtkFilebutton *filebutton, gpointer user_data)
{
  char buf[4096];
  snprintf(buf, sizeof(buf), "Filebutton changed: %s", gtk_filebutton_get_filename(filebutton));
  log_text(buf);
}

static void
on_clear_clicked(GtkButton *button, GtkComboBox *combobox)
{
  gtk_combo_box_set_active(combobox, -1);
  printf("clear\n");
}
int
main(int argc, char **argv)
{
  gtk_init(&argc, &argv);

  GtkBuilder *builder = gtk_builder_new();
  gtk_builder_add_from_file(builder, "libgtkex-test.xml", NULL);
  gtk_builder_connect_signals(builder, NULL);

  GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
  g_signal_connect(window, "delete_event", G_CALLBACK(gtk_main_quit), NULL);

  GtkWidget *container = GTK_WIDGET(gtk_builder_get_object(builder, "container"));
  statusbar = GTK_WIDGET(gtk_builder_get_object(builder, "statusbar"));
  g_object_unref(G_OBJECT(builder));

  GtkWidget *led_line = gtk_led_line_new(8);
  gtk_box_pack_start(GTK_BOX(container), led_line, TRUE, TRUE, 4);

  GtkWidget *filebutton = gtk_filebutton_new("Image");
  g_signal_connect(filebutton, "changed", G_CALLBACK(on_filebutton_changed), NULL);
  gtk_box_pack_start(GTK_BOX(container), filebutton, TRUE, TRUE, 4);

  GtkWidget *comboboxentry = gtk_combo_box_entry_new_text();
  gtk_combo_box_append_text(GTK_COMBO_BOX(comboboxentry), "blah");
  gtk_combo_box_append_text(GTK_COMBO_BOX(comboboxentry), "blubb");
  gtk_box_pack_start(GTK_BOX(container), comboboxentry, TRUE, TRUE, 4);

  GtkWidget *button = gtk_button_new_with_label("clear");
  g_signal_connect(button, "clicked", G_CALLBACK(on_clear_clicked), comboboxentry);
  gtk_box_pack_start(GTK_BOX(container), button, TRUE, TRUE, 4);

  gtk_widget_show_all(window);

  gtk_main();
  
  return 0;
}
