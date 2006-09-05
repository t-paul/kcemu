/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: window.h,v 1.4 2002/10/31 01:38:07 torsten_paul Exp $
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

#ifndef __ui_gtk_window_h
#define __ui_gtk_window_h

#include <gtk/gtk.h>

#include "cmd/cmd.h"

#include "ui/window.h"

class UI_Gtk_Window : public UI_Window
{
 private:
  bool _visible;
  CMD_Args *_help_args;

 protected:
  GtkWidget *_window;

  virtual void init(void) = 0;

  static void sf_help(GtkWidget *widget, gpointer data);
  static void sf_help_recursive(GtkWidget *widget, gpointer data);
  
 public:
  UI_Gtk_Window(void);
  virtual ~UI_Gtk_Window(void);

  void show(void);
  void hide(void);
  void toggle(void);

  bool is_visible(void);

  GtkWidget * get_window(void);
  GtkWidget * get_widget(const char *name);

  void init_dialog(const char *close_button_func, const char *help_topic);
};

#endif /* __ui_gtk_window_h */
