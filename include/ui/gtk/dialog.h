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

#ifndef __ui_gtk_dialog_h
#define __ui_gtk_dialog_h

#include <gtk/gtk.h>

#include "kc/system.h"

#include "ui/gtk/window.h"

#include "cmd/cmd.h"

class DialogWindow : public UI_Gtk_Window, public CMD
{
 private:
  CMD_Args *_args;
  int _dialog_result;
  gint _delete_handler_id;

  struct {
      GtkWidget *button_ok;
      GtkWidget *button_cancel;
      GtkWidget *button_yes;
      GtkWidget *button_no;
      GtkWidget *image_info;
      GtkWidget *image_question;
  } _w;
  
 public:
  DialogWindow(const char *ui_xml_file);
  virtual ~DialogWindow(void);
  
 protected:
  void init(void);
  void init_ok(const char *title, const char *text);
  void init_yes_no(const char *title, const char *text);
  void init_yes_no_cancel(const char *title, const char *text);
  void init_misc(const char *title, const char *text, GCallback callback);
  
  void show_buttons(bool button_ok, bool button_cancel, bool button_yes, bool button_no);

  static void ok(GtkWidget */*widget*/, gpointer data);
  static void no(GtkWidget */*widget*/, gpointer data);
  static void yes(GtkWidget */*widget*/, gpointer data);
  static void cancel(GtkWidget */*widget*/, gpointer data);

  static int delete_event_ok(GtkWidget *widget, GdkEvent *event, gpointer data);
  static int delete_event_yes_no(GtkWidget *widget, GdkEvent *event, gpointer data);
  static int delete_event_yes_no_cancel(GtkWidget *widget, GdkEvent *event, gpointer data);

 public:
  void execute(CMD_Args *args, CMD_Context context);

  void show_dialog_ok(const char *title, const char *text);
  void show_dialog_yes_no(const char *title, const char *text);
  void show_dialog_yes_no_cancel(const char *title, const char *text);
  
  int get_dialog_result(void);
};


#endif /* __ui_gtk_dialog_h */
