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

#include "kc/kc.h"
#include "kc/system.h"

#include "cmd/cmd.h"

#include "ui/gtk/cmd.h"
#include "ui/gtk/screenshot.h"

#include "libgtkex/libgtkex.h"

class CMD_ui_screenshot_window_toggle : public CMD
{
private:
  ScreenshotWindow *_w;

public:
  CMD_ui_screenshot_window_toggle(ScreenshotWindow *w) : CMD("ui-screenshot-window-toggle")
    {
      _w = w;
      register_cmd("ui-screenshot-window-toggle");
    }

  void execute(CMD_Args *args, CMD_Context context)
    {
      _w->toggle();
    }
};

void
ScreenshotWindow::on_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
  ScreenshotWindow *self = (ScreenshotWindow *)user_data;
  self->expose(event);
}

void
ScreenshotWindow::on_save_clicked(GtkButton *button, gpointer user_data)
{
  ScreenshotWindow *self = (ScreenshotWindow *) user_data;
  self->save();
}

void
ScreenshotWindow::on_screenshot_clicked(GtkButton *button, gpointer user_data)
{
  ScreenshotWindow *self = (ScreenshotWindow *) user_data;
  self->screenshot();
}

void
ScreenshotWindow::configure_spinbutton(GtkWidget *spinbutton1, GtkWidget *spinbutton2, double range)
{
  GtkAdjustment *adj1 = gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(spinbutton1));
  GtkAdjustment *adj2 = gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(spinbutton2));

  gtk_adjustment_set_upper(adj2, range - gtk_adjustment_get_value(adj1) - 1);

  gtk_widget_queue_draw(_w.canvas);
}

void
ScreenshotWindow::on_spinbutton_left_changed(GtkSpinButton *spinbutton, gpointer user_data)
{
  ScreenshotWindow *self = (ScreenshotWindow *)user_data;
  self->configure_spinbutton(self->_w.crop_left_spinbutton, self->_w.crop_right_spinbutton, self->_width);
}

void
ScreenshotWindow::on_spinbutton_right_changed(GtkSpinButton *spinbutton, gpointer user_data)
{
  ScreenshotWindow *self = (ScreenshotWindow *)user_data;
  self->configure_spinbutton(self->_w.crop_right_spinbutton, self->_w.crop_left_spinbutton, self->_width);
}

void
ScreenshotWindow::on_spinbutton_top_changed(GtkSpinButton *spinbutton, gpointer user_data)
{
  ScreenshotWindow *self = (ScreenshotWindow *)user_data;
  self->configure_spinbutton(self->_w.crop_top_spinbutton, self->_w.crop_bottom_spinbutton, self->_height);
}

void
ScreenshotWindow::on_spinbutton_bottom_changed(GtkSpinButton *spinbutton, gpointer user_data)
{
  ScreenshotWindow *self = (ScreenshotWindow *)user_data;
  self->configure_spinbutton(self->_w.crop_bottom_spinbutton, self->_w.crop_top_spinbutton, self->_height);
}

gboolean
ScreenshotWindow::timeout_handler(ScreenshotWindow *self)
{
  if (self->_window->window == NULL)
    return FALSE;

  if (!self->is_visible())
    return FALSE;

  self->_offset += 1;
  if (self->_offset > 16)
    self->_offset = 0;
  
  gtk_widget_queue_draw(self->_w.canvas);

  return TRUE;
}

ScreenshotWindow::ScreenshotWindow(const char *ui_xml_file, UI_Gtk *ui) : UI_Gtk_Window(ui_xml_file)
{
  _ui = ui;
  _width = 0;
  _height = 0;
  _pixbuf = NULL;
  _cmd_screenshot_toggle = new CMD_ui_screenshot_window_toggle(this);
}

ScreenshotWindow::~ScreenshotWindow(void)
{
  delete _cmd_screenshot_toggle;
}

void
ScreenshotWindow::init(void)
{
  /*
   *  screenshot window
   */
  _window = get_widget("screenshot_window");
  g_signal_connect(_window, "delete_event", G_CALLBACK(cmd_exec_sft), (char *)"ui-screenshot-window-toggle"); // FIXME:
  
  /*
   *  canvas, needs GDK_EXPOSURE_MASK to be set!
   */
  _w.canvas = get_widget("drawingarea");
  g_signal_connect(_w.canvas, "expose_event", G_CALLBACK(on_expose_event), this);

  _w.button_screenshot = get_widget("dialog_button_screenshot");
  g_signal_connect(_w.button_screenshot, "clicked", G_CALLBACK(on_screenshot_clicked), this);

  _w.button_save = get_widget("dialog_button_save");
  g_signal_connect(_w.button_save, "clicked", G_CALLBACK(on_save_clicked), this);

  _w.format_combobox = get_widget("format_combobox");
  _w.store = get_pixbuf_formats();
  gtk_combo_box_set_model(GTK_COMBO_BOX(_w.format_combobox), GTK_TREE_MODEL(_w.store));
  gtk_combo_box_set_active(GTK_COMBO_BOX(_w.format_combobox), 0);
  bind_list_model_column(GTK_COMBO_BOX(_w.format_combobox), 0, "xalign", 1, NULL);

  _w.filebutton = gtk_filebutton_new("Image");
  GtkWidget *c = get_widget("image_file_container");
  gtk_container_add(GTK_CONTAINER(c), _w.filebutton);
  gtk_widget_show(_w.filebutton);

  _w.crop_left_spinbutton = get_widget("crop_left_spinbutton");
  _w.crop_right_spinbutton = get_widget("crop_right_spinbutton");
  _w.crop_top_spinbutton = get_widget("crop_top_spinbutton");
  _w.crop_bottom_spinbutton = get_widget("crop_bottom_spinbutton");
  g_signal_connect(_w.crop_left_spinbutton, "value-changed", G_CALLBACK(on_spinbutton_left_changed), this);
  g_signal_connect(_w.crop_right_spinbutton, "value-changed", G_CALLBACK(on_spinbutton_right_changed), this);
  g_signal_connect(_w.crop_top_spinbutton, "value-changed", G_CALLBACK(on_spinbutton_top_changed), this);
  g_signal_connect(_w.crop_bottom_spinbutton, "value-changed", G_CALLBACK(on_spinbutton_bottom_changed), this);

  init_dialog("ui-screenshot-window-toggle", "window-screenshot");
}

GtkListStore *
ScreenshotWindow::get_pixbuf_formats(void)
{
  GtkListStore *store = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_FLOAT, G_TYPE_STRING, G_TYPE_STRING);

  GtkTreeIter iter;
  GSList *formats = gdk_pixbuf_get_formats();
  for (GSList *ptr = formats;ptr != NULL;ptr = ptr->next)
    {
      GdkPixbufFormat *format = (GdkPixbufFormat *)ptr->data;
      if (!gdk_pixbuf_format_is_writable(format))
        continue;

      gchar *name = gdk_pixbuf_format_get_name(format);
      if (strcmp(name, "ico") == 0)
        continue;

      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, name, 1, 1.0, 2, NULL, 3, NULL, -1);

      if (strcmp(name, "png") == 0)
        gtk_list_store_set(store, &iter, 2, "compression", 3, "9", -1);
      if (strcmp(name, "jpeg") == 0)
        gtk_list_store_set(store, &iter, 2, "quality", 3, "90", -1);
    }
  g_slist_free(formats);

  return store;
}

void
ScreenshotWindow::toggle(void)
{
  UI_Gtk_Window::toggle();

  if (_w.timeout_id != 0)
    g_source_remove(_w.timeout_id);

  if (!is_visible())
    return;

  screenshot();
  _w.timeout_id = g_timeout_add(200, (GSourceFunc)timeout_handler, this);
}

void
ScreenshotWindow::screenshot(void)
{
  if (_pixbuf != NULL)
    gdk_pixbuf_unref(_pixbuf);
  
  _pixbuf = _ui->get_screenshot();
  
  int width = gdk_pixbuf_get_width(_pixbuf);
  int height = gdk_pixbuf_get_height(_pixbuf);

  if ((_width != width) || (_height != height))
    {
      _width = width;
      _height = height;
      gtk_widget_set_size_request(_w.canvas, width, height);

      gtk_spin_button_set_value(GTK_SPIN_BUTTON(_w.crop_left_spinbutton), 0.0);
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(_w.crop_right_spinbutton), 0.0);
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(_w.crop_top_spinbutton), 0.0);
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(_w.crop_bottom_spinbutton), 0.0);
    }

  gtk_widget_queue_draw(_w.canvas);
}

void
ScreenshotWindow::expose(GdkEventExpose *event)
{
  if (_pixbuf == NULL)
    return;

  GtkStyle *style = gtk_widget_get_style(_w.canvas);
  gdk_pixbuf_render_to_drawable(_pixbuf, _w.canvas->window, style->black_gc, 0, 0, 0, 0, _width, _height, GDK_RGB_DITHER_NONE, 0, 0);

  cairo_t *cr = gdk_cairo_create(_w.canvas->window);

  double x1 = gtk_spin_button_get_value(GTK_SPIN_BUTTON(_w.crop_left_spinbutton));
  double y1 = gtk_spin_button_get_value(GTK_SPIN_BUTTON(_w.crop_top_spinbutton));
  double x2 = gtk_spin_button_get_value(GTK_SPIN_BUTTON(_w.crop_right_spinbutton));
  double y2 = gtk_spin_button_get_value(GTK_SPIN_BUTTON(_w.crop_bottom_spinbutton));

  if (x1 + x2 + y1 + y2 < 0.5)
    return;

  // align on pixel
  x1 += 0.5;
  y1 += 0.5;
  x2 += 0.5;
  y2 += 0.5;
  
  double dash[2] = { 4.0, 12.0 };
  cairo_set_line_width(cr, 1);
  cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
  cairo_move_to(cr, x1, y1);
  cairo_line_to(cr, _width - x2, y1);
  cairo_line_to(cr, _width - x2, _height - y2);
  cairo_line_to(cr, x1, _height - y2);
  cairo_line_to(cr, x1, y1);
  cairo_set_dash(cr, dash, 2, _offset);
  cairo_stroke(cr);
  cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
  cairo_move_to(cr, x1, y1);
  cairo_line_to(cr, _width - x2, y1);
  cairo_line_to(cr, _width - x2, _height - y2);
  cairo_line_to(cr, x1, _height - y2);
  cairo_line_to(cr, x1, y1);
  cairo_set_dash(cr, dash, 2, _offset + 8.0);
  cairo_stroke(cr);
  cairo_destroy(cr);
}

void
ScreenshotWindow::save(void)
{
  gchar *filename = gtk_filebutton_get_filename(GTK_FILEBUTTON(_w.filebutton));
  if (filename == NULL)
    return;

  int x1 = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(_w.crop_left_spinbutton));
  int y1 = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(_w.crop_top_spinbutton));
  int x2 = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(_w.crop_right_spinbutton));
  int y2 = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(_w.crop_bottom_spinbutton));

  GdkPixbuf *pixbuf = gdk_pixbuf_new_subpixbuf(_pixbuf, x1, y1, _width - x2 - x1, _height - y2 - y1);
  if (pixbuf == NULL)
    return;

  GtkTreeIter iter;
  if (!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(_w.format_combobox), &iter))
    return;

  gchar *type, *option, *value;
  gtk_tree_model_get(GTK_TREE_MODEL(_w.store), &iter, 0, &type, 2, &option, 3, &value, -1);

  gdk_pixbuf_save(pixbuf, filename, type, NULL, option, value, NULL);
  g_free(type);
  g_free(option);
  g_free(value);
  g_free(filename);
}
