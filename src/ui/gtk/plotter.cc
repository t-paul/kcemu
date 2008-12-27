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

#include <math.h>
#include <cairo/cairo.h>
#include <gtk-2.0/gtk/gtkcolorbutton.h>
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

class CMD_ui_plotter_info : public CMD
{
private:
  PlotterWindow *_w;

public:
  CMD_ui_plotter_info(PlotterWindow *w) : CMD("ui-plotter-info")
    {
      _w = w;
      register_cmd("ui-plotter-file-selected", 1);
      register_cmd("ui-plotter-file-closed", 2);
    }

  void execute(CMD_Args *args, CMD_Context context)
    {
      switch (context)
	{
	case 1:
	  if (args == NULL)
	    return;
	  _w->selected(args->get_string_arg("filename"));
	  break;
	case 2:
	  _w->selected(NULL);
	  break;
	}
    }
};

void
PlotterWindow::sf_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  PlotterWindow *self = (PlotterWindow *)data;
  self->expose(event);
}

void
PlotterWindow::sf_configure(GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
  PlotterWindow *self = (PlotterWindow *)data;
  self->configure(event);
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

  plotter->save_as_png(filename, 2970);
  g_free(filename);
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

PlotterWindow::PlotterWindow(const char *glade_xml_file) : UI_Gtk_Window(glade_xml_file)
{
  _cmd_plotter_info = new CMD_ui_plotter_info(this);
  _cmd_plotter_toggle = new CMD_ui_plotter_window_toggle(this);
}

PlotterWindow::~PlotterWindow(void)
{
  delete _cmd_plotter_info;
  delete _cmd_plotter_toggle;
}

gboolean
PlotterWindow::timeout_handler(PlotterWindow *self)
{
  if (self->_window->window == NULL)
    return FALSE;

  if (!self->is_visible())
    return FALSE;

  gtk_widget_queue_draw(self->_w.canvas);
  return TRUE;
}

void
PlotterWindow::toggle(void)
{
  UI_Gtk_Window::toggle();
  if (is_visible())
    {
      g_timeout_add(1000, (GSourceFunc)timeout_handler, this);
    }
}

void
PlotterWindow::init(void)
{
  /*
   *  plotter window
   */
  _window = get_widget("plotter_window");
  g_signal_connect(_window, "delete_event", G_CALLBACK(cmd_exec_sft), (char *)"ui-plotter-window-toggle"); // FIXME:
  
  _w.tooltips = gtk_tooltips_new();

  /*
   *  canvas, needs GDK_EXPOSURE_MASK to be set!
   */
  _w.canvas = get_widget("main_drawingarea");
  g_signal_connect(_w.canvas, "expose_event", G_CALLBACK(sf_expose), this);
  g_signal_connect(_w.canvas, "configure_event", G_CALLBACK(sf_configure), this);

  /*
   *  comboboxentry
   */
  _w.comboboxentry = get_widget("file_comboboxentry");

  /*
   *  open button
   */
  _w.open = get_widget("file_button_open");
  gtk_signal_connect(GTK_OBJECT(_w.open), "clicked",
		     GTK_SIGNAL_FUNC(cmd_exec_sf),
		     (gpointer)"kc-plotter-open");

  gtk_tooltips_set_tip(_w.tooltips, _w.open, _("Open"), NULL);

  /*
   *  close button
   */
  _w.close = get_widget("file_button_close");
  gtk_signal_connect(GTK_OBJECT(_w.close), "clicked",
		     GTK_SIGNAL_FUNC(cmd_exec_sf),
		     (gpointer)"kc-plotter-close");

  gtk_tooltips_set_tip(_w.tooltips, _w.close, _("Close"), NULL);


  _w.next_page = get_widget("button_next_page");
  g_signal_connect(_w.next_page, "clicked", G_CALLBACK(sf_next_page), this);

  _w.save_as_png = get_widget("button_save_as_png");
  g_signal_connect(_w.save_as_png, "clicked", G_CALLBACK(sf_save_as_png), this);

  _w.line_width = get_widget("spinbutton_line_width");
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(_w.line_width), plotter->get_line_width());
  g_signal_connect(_w.line_width, "value-changed", G_CALLBACK(sf_line_width), this);

  GdkColor color;
  _w.pen_color = get_widget("colorbutton_pen_color");
  color.red = plotter->get_pen_red() * 65536;
  color.green = plotter->get_pen_green() * 65536;
  color.blue = plotter->get_pen_blue() * 65536;
  gtk_color_button_set_color(GTK_COLOR_BUTTON(_w.pen_color), &color);
  g_signal_connect(_w.pen_color, "color-set", G_CALLBACK(sf_pen_color), this);

  init_dialog("ui-plotter-window-toggle", "window-plotter");
}

void
PlotterWindow::selected(const char *filename)
{
  gboolean sensitive = TRUE;

  if (filename == NULL)
    {
      filename = "";
      sensitive = FALSE;
    }

  gtk_widget_set_sensitive(_w.close, sensitive);

  GtkEntry *entry = GTK_ENTRY(GTK_BIN(_w.comboboxentry)->child);
  gtk_entry_set_text(GTK_ENTRY(entry), filename);
}

void
PlotterWindow::expose(GdkEventExpose *event)
{
  cairo_t *cr = gdk_cairo_create(_w.canvas->window);

  /* set a clip region for the expose event */
  GdkRectangle *area = &event->area;
  cairo_rectangle(cr, area->x, area->y, area->width, area->height);
  cairo_clip(cr);

  int width = _w.canvas->allocation.width;
  int height = _w.canvas->allocation.height;
  cairo_surface_t *surface = plotter->get_image_surface(width, height);
  if (surface != NULL)
    {
      cairo_set_source_surface(cr, surface, 0, 0);
      cairo_paint(cr);
    }

  cairo_destroy(cr);
}

void
PlotterWindow::configure(GdkEventConfigure *event)
{
}
