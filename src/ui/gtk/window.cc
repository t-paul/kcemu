/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2006 Torsten Paul
 *
 *  $Id: window.cc,v 1.4 2002/10/31 01:38:12 torsten_paul Exp $
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

#include <string>

#include "kc/kc.h"

#include "ui/gtk/window.h"

#include "ui/gtk/cmd.h"

using namespace std;

bool UI_Gtk_Window::_static_init = false;
string UI_Gtk_Window::_icon_path;
string UI_Gtk_Window::_image_path;

UI_Gtk_Window::UI_Gtk_Window(void)
{
  static_init();
  
  _window = 0;
  _visible = false;
  _glade_xml = NULL;
  _help_args = new CMD_Args();
}

UI_Gtk_Window::UI_Gtk_Window(const char *glade_xml_file)
{
  static_init();
  
  _window = 0;
  _visible = false;
  _help_args = new CMD_Args();
  
  string glade_xml_file_path = _image_path + glade_xml_file;
  _glade_xml = glade_xml_new(glade_xml_file_path.c_str(), NULL, NULL);
}

UI_Gtk_Window::~UI_Gtk_Window(void)
{
  delete _help_args;
}

void
UI_Gtk_Window::static_init(void)
{
  if (_static_init)
    return;
  
  _static_init = true;

  string datadir(kcemu_datadir);
  _icon_path = datadir + "/icons/";
  _image_path = datadir + "/images/";

  GtkSettings *settings = gtk_settings_get_default();
  gtk_settings_set_long_property(settings, "gtk-can-change-accels", 1, "UI_Gtk_Window::static_init");
  gtk_settings_set_string_property(settings, "gtk-menu-bar-accel", "", "UI_Gtk_Window::static_init");

  GdkPixbuf *pixbuf16 = get_icon("kcemu-winicon_16x16.png");
  GdkPixbuf *pixbuf32 = get_icon("kcemu-winicon_32x32.png");
  GdkPixbuf *pixbuf48 = get_icon("kcemu-winicon_48x48.png");
    
  GList *icon_list = NULL;
  if (pixbuf16 != NULL)
    icon_list = g_list_append(icon_list, pixbuf16);
  if (pixbuf32 != NULL)
    icon_list = g_list_append(icon_list, pixbuf32);
  if (pixbuf48 != NULL)
    icon_list = g_list_append(icon_list, pixbuf48);
    
  if (icon_list != NULL)
    gtk_window_set_default_icon_list(icon_list);
}

void
UI_Gtk_Window::show(void)
{
  if (_visible)
    return;

  if (_window == NULL)
    init();

  gtk_widget_show(_window);
  _visible = true;
}

void
UI_Gtk_Window::hide(void)
{
  if (!_visible)
    return;

  gtk_widget_hide(_window);
  _visible = false;
}

void
UI_Gtk_Window::toggle(void)
{
  if (_visible)
    hide();
  else
    show();
}

bool
UI_Gtk_Window::is_visible(void)
{
  return _visible;
}

GtkWidget *
UI_Gtk_Window::get_window(void)
{
  return _window;
}

GdkPixbuf *
UI_Gtk_Window::get_pixbuf(string path)
{
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(path.c_str(), NULL);
    return pixbuf;
}

GdkPixbuf *
UI_Gtk_Window::get_icon(const char *name)
{
    return get_pixbuf(_icon_path + name);
}

GdkPixbuf *
UI_Gtk_Window::get_image(const char *name)
{
    return get_pixbuf(_image_path + name);
}

GladeXML *
UI_Gtk_Window::get_glade_xml(void)
{
    return _glade_xml;
}

GtkWidget *
UI_Gtk_Window::get_widget_or_null(const char *name)
{
  g_assert(_glade_xml != NULL);

  GtkWidget *widget = glade_xml_get_widget(_glade_xml, name);

  return widget;
}

GtkWidget *
UI_Gtk_Window::get_widget(const char *name)
{
  GtkWidget *widget = get_widget_or_null(name);

  if (widget == NULL)
      g_error("widget with name '%s' not found!", name);

  return widget;
}

GtkCellRenderer *
UI_Gtk_Window::add_text_renderer(GtkTreeView *treeview, GtkTreeViewColumn *column, const char *title, ...) {
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    
    gtk_tree_view_column_set_title(column, title);
    gtk_tree_view_column_pack_end(column, renderer, TRUE);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_expand(column, TRUE);

    va_list ap;
    va_start(ap, title);

    while (242) {
        const char *attr = va_arg(ap, const char *);
        if (attr == NULL)
            break;

        int index = va_arg(ap, int);
        gtk_tree_view_column_add_attribute(column, renderer, attr, index);
    }

    return renderer;
}

GtkCellRenderer *
UI_Gtk_Window::add_icon_renderer(GtkTreeView *treeview, GtkTreeViewColumn *column, const char *title, ...) {
    GtkCellRenderer *renderer = gtk_cell_renderer_pixbuf_new();
    
    gtk_tree_view_column_set_title(column, title);
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_set_resizable(column, FALSE);
    gtk_tree_view_column_set_expand(column, FALSE);

    va_list ap;
    va_start(ap, title);

    while (242) {
        const char *attr = va_arg(ap, const char *);
        if (attr == NULL)
            break;

        int index = va_arg(ap, int);
        gtk_tree_view_column_add_attribute(column, renderer, attr, index);
    }
    
    return renderer;
}

void
UI_Gtk_Window::sf_help(GtkWidget *widget, gpointer data)
{
  UI_Gtk_Window *w = (UI_Gtk_Window *)data;

  char *help_topic = (char *)g_object_get_data(G_OBJECT(widget), "help-topic");
  if (help_topic == NULL)
    return;

  w->_help_args->set_string_arg("help-topic", help_topic);

  CMD_EXEC_ARGS("ui-help-window-show", w->_help_args);
}

void
UI_Gtk_Window::sf_help_recursive(GtkWidget *widget, gpointer data)
{
  UI_Gtk_Window *w = (UI_Gtk_Window *)data;

  while (widget != NULL)
    {
      char *help_topic = (char *)g_object_get_data(G_OBJECT(widget), "help-topic");
      if (help_topic != NULL)
	{
	  w->_help_args->set_string_arg("help-topic", help_topic);
	  CMD_EXEC_ARGS("ui-help-window-show", w->_help_args);
	  break;
	}
      
      widget = gtk_widget_get_parent(widget);
    }
}

void
UI_Gtk_Window::init_dialog(const char *close_button_func, const char *help_topic)
{
  g_object_set_data(G_OBJECT(_window), "help-topic", (gpointer)help_topic);
  
  if (close_button_func != NULL)
    {
      GtkWidget *button_close = get_widget("dialog_button_close");
      g_assert(GTK_IS_BUTTON(button_close));
      
      g_signal_connect(G_OBJECT(button_close), "clicked",
		       G_CALLBACK(cmd_exec_sf),
		       (gpointer)close_button_func);
      
      GTK_WIDGET_SET_FLAGS(button_close, GTK_CAN_DEFAULT);
      gtk_widget_grab_default(button_close);
    }
  
  if (help_topic != NULL)
    {
      GtkWidget *button_help = get_widget("dialog_button_help");
      g_assert(GTK_IS_BUTTON(button_help));
      
      g_object_set_data(G_OBJECT(button_help), "help-topic", (gpointer)help_topic);
      
      g_signal_connect(G_OBJECT(button_help), "clicked", G_CALLBACK(sf_help), (gpointer)this);
    }
  
  GtkWidget *header_label = get_widget("header_label");
  g_assert(GTK_IS_LABEL(header_label));
  
  PangoFontDescription *font_desc = pango_font_description_from_string("Sans 16");
  gtk_widget_modify_font(header_label, font_desc);
  pango_font_description_free(font_desc);
}
