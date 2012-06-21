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

#include <cairo/cairo.h>
#include <glib-2.0/glib.h>
#include <gtk-2.0/gtk/gtkspinbutton.h>

#include "kc/kc.h"
#include "kc/system.h"

#include "kc/plotter.h"

#include "cmd/cmd.h"

#include "ui/gtk/cmd.h"
#include "ui/gtk/plotter.h"

class CMD_ui_plotter_window_toggle : public CMD
{
private:
  PlotterWindow *_w;

public:
  CMD_ui_plotter_window_toggle(PlotterWindow *w) : CMD("ui-plotter-window-toggle")
    {
      _w = w;
      register_cmd("ui-plotter-window-toggle");
    }

  void execute(CMD_Args *args, CMD_Context context)
    {
      _w->toggle();
    }
};

void
PlotterWindow::sf_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  PlotterWindow *self = (PlotterWindow *)data;
  self->expose(event);
}

void
PlotterWindow::sf_next_page(GtkWidget *widget, gpointer data)
{
  PlotterWindow *self = (PlotterWindow *)data;

  plotter->show_page();
  gtk_widget_queue_draw(self->_w.canvas);
}

void
PlotterWindow::sf_save_as_png(GtkWidget *widget, gpointer data)
{
  PlotterWindow *self = (PlotterWindow *)data;
  
  GtkWidget *filechooser = gtk_file_chooser_dialog_new(_("Save As PNG..."),
                                                       GTK_WINDOW(self->_window),
                                                       GTK_FILE_CHOOSER_ACTION_SAVE,
                                                       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                       GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                                       NULL);
  gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(filechooser), TRUE);

  char *filename = NULL;
  if (gtk_dialog_run(GTK_DIALOG(filechooser)) == GTK_RESPONSE_ACCEPT)
    {
      filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(filechooser));
    }

  gtk_widget_destroy (filechooser);

  if (filename == NULL)
    return;

  plotter->save_as_png(filename);
  g_free(filename);
}

void
PlotterWindow::sf_open_pdf(GtkWidget *widget, gpointer data)
{
  PlotterWindow *self = (PlotterWindow *)data;

  GtkWidget *filechooser = gtk_file_chooser_dialog_new(_("Plot to PDF..."),
                                                       GTK_WINDOW(self->_window),
                                                       GTK_FILE_CHOOSER_ACTION_SAVE,
                                                       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                       GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                                       NULL);
  gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(filechooser), TRUE);

  char *filename = NULL;
  if (gtk_dialog_run(GTK_DIALOG(filechooser)) == GTK_RESPONSE_ACCEPT)
    {
      filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(filechooser));
    }

  gtk_widget_destroy (filechooser);

  if (filename == NULL)
    return;

  plotter->open_pdf(filename);

  gtk_widget_set_sensitive(self->_w.close, TRUE);
  GtkEntry *entry = GTK_ENTRY(GTK_BIN(self->_w.comboboxentry)->child);
  gtk_entry_set_text(GTK_ENTRY(entry), filename);

  g_free(filename);
}

void
PlotterWindow::sf_close_pdf(GtkWidget *widget, gpointer data)
{
  PlotterWindow *self = (PlotterWindow *)data;

  plotter->close_pdf();

  gtk_widget_set_sensitive(self->_w.close, FALSE);
  GtkEntry *entry = GTK_ENTRY(GTK_BIN(self->_w.comboboxentry)->child);
  gtk_entry_set_text(GTK_ENTRY(entry), "");
}

void
PlotterWindow::sf_bg_color(GtkColorButton *widget, gpointer data)
{
  const double f = 65536.0;

  GdkColor color;
  gtk_color_button_get_color(widget, &color);
  plotter->set_bg_color((double)color.red / f, (double)color.green / f, (double)color.blue / f);
}

void
PlotterWindow::sf_pen_color(GtkColorButton *widget, gpointer data)
{
  const double f = 65536.0;

  GdkColor color;
  gtk_color_button_get_color(widget, &color);
  plotter->set_pen_color((double)color.red / f, (double)color.green / f, (double)color.blue / f);
}

void
PlotterWindow::sf_line_width(GtkSpinButton *widget, gpointer data)
{
  gdouble line_width = gtk_spin_button_get_value(widget);
  plotter->set_line_width(line_width);
}

void
PlotterWindow::sf_origin_x(GtkSpinButton *widget, gpointer data)
{
  gdouble origin_x = gtk_spin_button_get_value(widget);
  plotter->set_origin_x(origin_x);
}

void
PlotterWindow::sf_origin_y(GtkSpinButton *widget, gpointer data)
{
  gdouble origin_y = gtk_spin_button_get_value(widget);
  plotter->set_origin_y(origin_y);
}

void
PlotterWindow::sf_display_refresh(GtkSpinButton *widget, gpointer data)
{
  PlotterWindow *self = (PlotterWindow *)data;
  self->_display_refresh = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(self->_w.display_refresh));
  self->init_timeout();
}

void
PlotterWindow::sf_show_pen(GtkToggleButton *widget, gpointer data)
{
  plotter->set_show_pen(gtk_toggle_button_get_active(widget));
}

void
PlotterWindow::sf_show_plot_area(GtkToggleButton *widget, gpointer data)
{
  plotter->set_show_plot_area(gtk_toggle_button_get_active(widget));
}

PlotterWindow::PlotterWindow(const char *ui_xml_file) : UI_Gtk_Window(ui_xml_file)
{
  _display_refresh = 20;
  _cmd_plotter_toggle = new CMD_ui_plotter_window_toggle(this);
}

PlotterWindow::~PlotterWindow(void)
{
  delete _cmd_plotter_toggle;
}

gboolean
PlotterWindow::timeout_handler(PlotterWindow *self)
{
  if (self->_window->window == NULL)
    return FALSE;

  if (!self->is_visible())
    return FALSE;

  if (plotter->is_dirty())
    {
      cairo_rectangle_t *rect = plotter->get_dirty_rectangle();
      if (rect != NULL)
        gtk_widget_queue_draw_area(self->_w.canvas, rect->x, rect->y, rect->width, rect->height);
    }

  return TRUE;
}

void
PlotterWindow::toggle(void)
{
  UI_Gtk_Window::toggle();
  init_timeout();
}

void
PlotterWindow::init_timeout()
{
  if (_w.timeout_id != 0)
    g_source_remove(_w.timeout_id);

  if (is_visible())
    _w.timeout_id = g_timeout_add(_display_refresh, (GSourceFunc)timeout_handler, this);
}

void
PlotterWindow::init(void)
{
  _w.timeout_id = 0;
  
  /*
   *  plotter window
   */
  _window = get_widget("plotter_window");
  g_signal_connect(_window, "delete_event", G_CALLBACK(cmd_exec_sft), (char *)"ui-plotter-window-toggle"); // FIXME:
  
  /*
   *  canvas, needs GDK_EXPOSURE_MASK to be set!
   */
  _w.canvas = get_widget("main_drawingarea");
  g_signal_connect(_w.canvas, "expose_event", G_CALLBACK(sf_expose), this);

  /*
   *  comboboxentry
   */
  _w.comboboxentry = get_widget("file_comboboxentry");

  /*
   *  bg color
   */
  GdkColor bg_color;
  _w.bg_color = get_widget("colorbutton_bg_color");
  bg_color.red = plotter->get_bg_red() * 65535;
  bg_color.green = plotter->get_bg_green() * 65535;
  bg_color.blue = plotter->get_bg_blue() * 65535;
  gtk_color_button_set_color(GTK_COLOR_BUTTON(_w.bg_color), &bg_color);
  g_signal_connect(_w.bg_color, "color-set", G_CALLBACK(sf_bg_color), this);
  
  /*
   *  pen color
   */
  GdkColor color;
  _w.pen_color = get_widget("colorbutton_pen_color");
  color.red = plotter->get_pen_red() * 65535;
  color.green = plotter->get_pen_green() * 65535;
  color.blue = plotter->get_pen_blue() * 65535;
  gtk_color_button_set_color(GTK_COLOR_BUTTON(_w.pen_color), &color);
  g_signal_connect(_w.pen_color, "color-set", G_CALLBACK(sf_pen_color), this);

  /*
   *  check boxes
   */
  _w.show_pen = get_widget("checkbutton_show_pen");
  g_signal_connect(_w.show_pen, "toggled", G_CALLBACK(sf_show_pen), this);

  _w.show_plot_area = get_widget("checkbutton_show_plot_area");
  g_signal_connect(_w.show_plot_area, "toggled", G_CALLBACK(sf_show_plot_area), this);

  /*
   *  buttons
   */
  _w.open = get_widget("file_button_open");
  g_signal_connect(_w.open, "clicked", G_CALLBACK(sf_open_pdf), this);

  _w.close = get_widget("file_button_close");
  g_signal_connect(_w.close, "clicked", G_CALLBACK(sf_close_pdf), this);

  _w.next_page = get_widget("button_next_page");
  g_signal_connect(_w.next_page, "clicked", G_CALLBACK(sf_next_page), this);

  _w.save_as_png = get_widget("button_save_as_png");
  g_signal_connect(_w.save_as_png, "clicked", G_CALLBACK(sf_save_as_png), this);

  /*
   *  spin buttons
   */
  _w.line_width = get_widget("spinbutton_line_width");
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(_w.line_width), plotter->get_line_width());
  g_signal_connect(_w.line_width, "value-changed", G_CALLBACK(sf_line_width), this);

  _w.origin_x = get_widget("spinbutton_origin_x");
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(_w.origin_x), plotter->get_origin_x());
  g_signal_connect(_w.origin_x, "value-changed", G_CALLBACK(sf_origin_x), this);

  _w.origin_y = get_widget("spinbutton_origin_y");
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(_w.origin_y), plotter->get_origin_y());
  g_signal_connect(_w.origin_y, "value-changed", G_CALLBACK(sf_origin_y), this);

  _w.display_refresh = get_widget("spinbutton_display_refresh");
  g_signal_connect(_w.display_refresh, "value-changed", G_CALLBACK(sf_display_refresh), this);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(_w.display_refresh), _display_refresh);

  init_dialog("ui-plotter-window-toggle", "window-plotter");
}

void
PlotterWindow::expose(GdkEventExpose *event)
{
  cairo_t *cr = gdk_cairo_create(_w.canvas->window);

  /* set a clip region for the expose event */
  GdkRectangle *area = &event->area;
  cairo_rectangle(cr, area->x, area->y, area->width, area->height);
  cairo_clip(cr);

  cairo_surface_t *surface = plotter->get_onscreen_surface();
  if (surface != NULL)
    {
      cairo_set_source_surface(cr, surface, 0, 0);
      cairo_paint(cr);
    }

  cairo_destroy(cr);
}
