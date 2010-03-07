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

#ifndef __ui_gtk_window_h
#define __ui_gtk_window_h

#include <string>

#include <gtk/gtk.h>

#include "cmd/cmd.h"

#include "ui/window.h"

using namespace std;

class UI_Gtk_Window : public UI_Window
{
 private:
  bool        _visible;
  CMD_Args   *_help_args;
  GtkBuilder *_gtk_builder;

  static bool   _static_init;
  static string _icon_path;
  static string _image_path;

  GtkSettingsValue _setting_gtk_can_change_accels;
  
private:
  void static_init(void);

protected:
  GtkWidget *_window;

  virtual void init(void) = 0;

  GdkPixbuf * get_pixbuf(string path);
  GdkPixbuf * get_icon(const char *name);
  GdkPixbuf * get_image(const char *name);

  GtkCellRenderer * add_text_renderer(GtkTreeView *treeview, GtkTreeViewColumn *, const char *title, ...);
  GtkCellRenderer * add_icon_renderer(GtkTreeView *treeview, GtkTreeViewColumn *, const char *title, ...);
  GtkCellRenderer * bind_list_model_column(GtkComboBox *combobox, int column, ...);
  
  static void sf_help(GtkWidget *widget, gpointer data);
  static void sf_help_recursive(GtkWidget *widget, gpointer data);
  
 public:
  UI_Gtk_Window(void);
  UI_Gtk_Window(const char *ui_xml_file);
  virtual ~UI_Gtk_Window(void);

  void show(void);
  void hide(void);
  void toggle(void);

  bool is_visible(void);

  GtkWidget * get_window(void);
  GtkWidget * get_widget(const char *name);
  GtkWidget * get_widget(const char *name, int nr);
  GtkWidget * get_widget_or_null(const char *name);

  void init_dialog(const char *close_button_func, const char *help_topic);
};

#endif /* __ui_gtk_window_h */
