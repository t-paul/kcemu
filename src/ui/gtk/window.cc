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

#include "ui/gtk/window.h"

#include "ui/gtk/cmd.h"
#include "ui/gtk/glade/glade_util.h"

UI_Gtk_Window::UI_Gtk_Window(void)
{
  _window = 0;
  _visible = false;
  _help_args = new CMD_Args();
}

UI_Gtk_Window::~UI_Gtk_Window(void)
{
  delete _help_args;
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

GtkWidget *
UI_Gtk_Window::get_widget(const char *name)
{
  g_assert(GTK_IS_WIDGET(_window));

  GtkWidget *widget = lookup_widget(_window, name);
  g_assert(GTK_IS_WIDGET(widget));

  return widget;
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
  
  PangoFontDescription *font_desc = pango_font_description_from_string("Sans Bold 18");
  gtk_widget_modify_font(header_label, font_desc);
  pango_font_description_free(font_desc);
}
