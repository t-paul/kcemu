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

#include "kc/system.h"

#include "kc/prefs/prefs.h"

#include "ui/gtk/cmd.h"
#include "ui/gtk/color.h"

#include "libdbg/dbg.h"

class CMD_color_window_toggle : public CMD
{
private:
  ColorWindow *_w;

public:
  CMD_color_window_toggle(ColorWindow *w) : CMD("ui-color-window-toggle")
    {
      _w = w;
      register_cmd("ui-color-window-toggle");
    }

  void execute(CMD_Args *args, CMD_Context context)
    {
      _w->toggle();
    }
};

void
ColorWindow::sf_adjustment_changed(GtkAdjustment *adj, double *data)
{
  *data = adj->value;
  CMD_EXEC("ui-update-colortable");
}

ColorWindow::ColorWindow(const char *ui_xml_file) : UI_Gtk_Window(ui_xml_file)
{
  _brightness_fg = Preferences::instance()->get_int_value("color_brightness_fg", 50) / 100.0;
  _contrast_fg   = Preferences::instance()->get_int_value("color_contrast_fg", 70) / 100.0;
  _brightness_bg = Preferences::instance()->get_int_value("color_brightness_bg", 35) / 100.0;
  _contrast_bg   = Preferences::instance()->get_int_value("color_contrast_bg", 50) / 100.0;

  _cmd = new CMD_color_window_toggle(this);
}

ColorWindow::~ColorWindow(void)
{
  delete _cmd;
}

GtkAdjustment *
ColorWindow::init_adjustment(GtkRange* range, double *val_ptr)
{
  GtkAdjustment *adj = gtk_range_get_adjustment(range);
  gtk_range_set_value(range, *val_ptr);
  gtk_object_set_data(GTK_OBJECT(adj), "self", this);
  gtk_signal_connect(GTK_OBJECT(adj), "value_changed",
		     GTK_SIGNAL_FUNC(sf_adjustment_changed), val_ptr);
  return adj;
}

void
ColorWindow::init(void)
{
  _window = get_widget("color_window");
  gtk_window_position(GTK_WINDOW(_window), GTK_WIN_POS_MOUSE);
  gtk_signal_connect(GTK_OBJECT(_window), "delete_event",
		     GTK_SIGNAL_FUNC(cmd_exec_sft),
		     (char *)"ui-color-window-toggle"); // FIXME:

  /*
   *  foreground contrast
   */
  _w.c_fg_vscale = get_widget("foreground_contrast_vscale");
  _w.s_fg_adj = init_adjustment(GTK_RANGE(_w.c_fg_vscale), &_contrast_fg);

  /*
   *  foreground brightness
   */
  _w.b_fg_vscale = get_widget("foreground_brightness_vscale");
  _w.v_fg_adj = init_adjustment(GTK_RANGE(_w.b_fg_vscale), &_brightness_fg);

  /*
   *  background contrast
   */
  _w.c_bg_vscale = get_widget("background_contrast_vscale");
  _w.s_bg_adj = init_adjustment(GTK_RANGE(_w.c_bg_vscale), &_contrast_bg);
  
  /*
   *  background brightness
   */
  _w.b_bg_vscale = get_widget("background_brightness_vscale");
  _w.v_bg_adj = init_adjustment(GTK_RANGE(_w.b_bg_vscale), &_brightness_bg);

  init_dialog("ui-color-window-toggle", "window-colors");
}
