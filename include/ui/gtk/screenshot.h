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

#ifndef __ui_gtk_screenshot_h
#define __ui_gtk_screenshot_h

#include <gtk/gtk.h>

#include <cairo/cairo.h>

#include "kc/system.h"

#include "ui/gtk/ui_gtk.h"
#include "ui/gtk/window.h"

class ScreenshotWindow : public UI_Gtk_Window
{
private:

  struct
  {
    GtkWidget *canvas;
    GtkWidget *crop_left_spinbutton;
    GtkWidget *crop_right_spinbutton;
    GtkWidget *crop_top_spinbutton;
    GtkWidget *crop_bottom_spinbutton;
    GtkWidget *button_save;
    GtkWidget *button_screenshot;
    GtkWidget *format_combobox;
    GtkWidget *filebutton;

    GtkListStore *store;
    guint     timeout_id;
  } _w;

  int _width;
  int _height;
  double _offset;
  GdkPixbuf *_pixbuf;

  UI_Gtk *_ui;
  CMD *_cmd_screenshot_toggle;

protected:
  void init(void);
  void expose(GdkEventExpose *event);
  void save(void);
  void screenshot(void);
  GtkListStore * get_pixbuf_formats(void);
  void configure_spinbutton(GtkWidget *spinbutton1, GtkWidget *spinbutton2, double range);

  static void on_save_clicked(GtkButton *button, gpointer user_data);
  static void on_screenshot_clicked(GtkButton *button, gpointer user_data);
  static void on_spinbutton_left_changed(GtkSpinButton *spinbutton, gpointer user_data);
  static void on_spinbutton_right_changed(GtkSpinButton *spinbutton, gpointer user_data);
  static void on_spinbutton_top_changed(GtkSpinButton *spinbutton, gpointer user_data);
  static void on_spinbutton_bottom_changed(GtkSpinButton *spinbutton, gpointer user_data);
  static void on_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data);
  static gboolean timeout_handler(ScreenshotWindow *self);
  
public:
  ScreenshotWindow(const char *ui_xml_file, UI_Gtk *ui);
  virtual ~ScreenshotWindow(void);

  void toggle(void);
};

#endif /* __ui_gtk_screenshot_h */
