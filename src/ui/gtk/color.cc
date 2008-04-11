/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: color.cc,v 1.4 2002/10/31 01:38:12 torsten_paul Exp $
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

ColorWindow::ColorWindow(const char *glade_xml_file) : UI_Gtk_Window(glade_xml_file)
{
  _saturation_fg = Preferences::instance()->get_int_value("color_fg_saturation", 80) / 100.0;
  _brightness_fg = Preferences::instance()->get_int_value("color_fg_brightness", 95) / 100.0;
  _saturation_bg = Preferences::instance()->get_int_value("color_bg_saturation", 65) / 100.0;
  _brightness_bg = Preferences::instance()->get_int_value("color_bg_brightness", 50) / 100.0;
  _black_level   = Preferences::instance()->get_int_value("color_black_level", 10) / 100.0;
  _white_level   = Preferences::instance()->get_int_value("color_white_level", 90) / 100.0;

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
   *  foreground saturation
   */
  _w.s_fg_vscale = get_widget("foreground_saturation_vscale");
  _w.s_fg_adj = init_adjustment(GTK_RANGE(_w.s_fg_vscale), &_saturation_fg);

  /*
   *  foreground brightness (= value of HSV)
   */
  _w.v_fg_vscale = get_widget("foreground_brightness_vscale");
  _w.v_fg_adj = init_adjustment(GTK_RANGE(_w.v_fg_vscale), &_brightness_fg);

  /*
   *  background saturation
   */
  _w.s_bg_vscale = get_widget("background_saturation_vscale");
  _w.s_bg_adj = init_adjustment(GTK_RANGE(_w.s_bg_vscale), &_saturation_bg);
  
  /*
   *  background brightness (= value of HSV)
   */
  _w.v_bg_vscale = get_widget("background_brightness_vscale");
  _w.v_bg_adj = init_adjustment(GTK_RANGE(_w.v_bg_vscale), &_brightness_bg);

  /*
   *  black level (= value of HSV for black pixels)
   */
  _w.b_vscale = get_widget("black_level_vscale");
  _w.b_adj = init_adjustment(GTK_RANGE(_w.b_vscale), &_black_level);

  /*
   *  white level (= value of HSV for white pixels)
   */
  _w.w_vscale = get_widget("white_level_vscale");
  _w.w_adj = init_adjustment(GTK_RANGE(_w.w_vscale), &_white_level);

  init_dialog("ui-color-window-toggle", NULL);
}
