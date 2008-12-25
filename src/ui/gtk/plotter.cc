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

#include "kc/system.h"

#include "cmd/cmd.h"

#include "ui/gtk/cmd.h"
#include "ui/gtk/plotter.h"

#include "kc/mod_4131.h"

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
PlotterWindow::sf_expose(GtkWidget *widget, GdkEventExpose *event, gpointer *data)
{
  PlotterWindow *self = (PlotterWindow *)data;
  self->expose(event);
}

void
PlotterWindow::sf_configure(GtkWidget *widget, GdkEventConfigure *event, gpointer *data)
{
  PlotterWindow *self = (PlotterWindow *)data;
  self->configure(event);
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
  gtk_widget_set_size_request(_w.canvas, WINDOW_WIDTH, WINDOW_HEIGHT);
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

  init_dialog("ui-plotter-window-toggle", "window-plotter");

  _w.gc = NULL;
  _image_y = 0;
  _image  = gdk_image_new(GDK_IMAGE_FASTEST,
			  gdk_visual_get_system(),
			  WINDOW_WIDTH,
			  2 * WINDOW_HEIGHT);
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
  gtk_widget_set_sensitive(_w.play, sensitive);
  gtk_widget_set_sensitive(_w.stop, sensitive);

  gtk_widget_set_sensitive(_w.record, FALSE);

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

  if (ModuleXY4131::_surface != NULL)
   cairo_set_source_surface(cr, ModuleXY4131::_surface, 0, 0);
  
  cairo_move_to(cr, 0, 0);
  cairo_line_to(cr, width, height);
  cairo_stroke(cr);
  cairo_paint(cr);
  
  cairo_destroy(cr);

  printf("expose: %d, %d\n", width, height);
}

void
PlotterWindow::configure(GdkEventConfigure *event)
{
  printf("configure: %d, %d\n", event->width, event->height);
  //gtk_widget_set_size_request(_w.canvas, event->width, event->height);
}
