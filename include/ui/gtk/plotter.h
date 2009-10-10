/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2008 Torsten Paul
 *
 *  $Id$
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

#ifndef __ui_gtk_plotter_h
#define __ui_gtk_plotter_h

#include <gtk/gtk.h>

#include <cairo/cairo.h>

#include "kc/system.h"

#include "ui/gtk/window.h"

class PlotterWindow : public UI_Gtk_Window
{
private:

  struct
  {
    GtkWidget *canvas;
    GtkWidget *comboboxentry;
    GtkWidget *open;
    GtkWidget *close;
    GtkWidget *next_page;
    GtkWidget *save_as_png;
    GtkWidget *line_width;
    GtkWidget *origin_x;
    GtkWidget *origin_y;
    GtkWidget *bg_color;
    GtkWidget *pen_color;
    GtkWidget *show_pen;
    GtkWidget *show_plot_area;
    GtkWidget *display_refresh;

    guint timeout_id;
  } _w;

  guint _display_refresh;
  
  CMD *_cmd_plotter_toggle;

protected:
  static void sf_next_page(GtkWidget *widget, gpointer data);
  static void sf_open_pdf(GtkWidget *widget, gpointer data);
  static void sf_close_pdf(GtkWidget *widget, gpointer data);
  static void sf_save_as_png(GtkWidget *widget, gpointer data);
  static void sf_bg_color(GtkColorButton *widget, gpointer data);
  static void sf_pen_color(GtkColorButton *widget, gpointer data);
  static void sf_line_width(GtkSpinButton *widget, gpointer data);
  static void sf_origin_x(GtkSpinButton *widget, gpointer data);
  static void sf_origin_y(GtkSpinButton *widget, gpointer data);
  static void sf_display_refresh(GtkSpinButton *widget, gpointer data);
  static void sf_show_pen(GtkToggleButton *widget, gpointer data);
  static void sf_show_plot_area(GtkToggleButton *widget, gpointer data);
  static void sf_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data);
  static gboolean timeout_handler(PlotterWindow *self);

  void init(void);
  void init_timeout(void);
  void expose(GdkEventExpose *event);

public:
  PlotterWindow(const char *ui_xml_file);
  virtual ~PlotterWindow(void);

  void toggle(void);
  void selected(const char *filename);
};

#endif /* __ui_gtk_plotter_h */
