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

#include "kc/rc.h"

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


ColorWindow::ColorWindow(void)
{
  init();
}

ColorWindow::~ColorWindow(void)
{
  delete _cmd;
}

void
ColorWindow::init(void)
{
  _saturation_fg = RC::instance()->get_int("Foreground Saturation", 40) / 100.0;
  _brightness_fg = RC::instance()->get_int("Foreground Brightness", 80) / 100.0;
  _saturation_bg = RC::instance()->get_int("Background Saturation", 50) / 100.0;
  _brightness_bg = RC::instance()->get_int("Background Brightness", 60) / 100.0;
  _black_level   = RC::instance()->get_int("Black Value", 10) / 100.0;
  _white_level   = RC::instance()->get_int("White Value", 90) / 100.0;

  _window = gtk_window_new(GTK_WINDOW_DIALOG);
  gtk_widget_set_name(_window, "ColorWindow");
  gtk_window_set_title(GTK_WINDOW(_window), _("KCemu: Color Configuration"));
  gtk_window_position(GTK_WINDOW(_window), GTK_WIN_POS_MOUSE);
  gtk_signal_connect(GTK_OBJECT(_window), "delete_event",
                     GTK_SIGNAL_FUNC(cmd_exec_sft),
                     (char *)"ui-color-window-toggle"); // FIXME:

  /*
   *  vbox
   */
  _w.vbox = gtk_vbox_new(FALSE, 2);
  gtk_container_border_width(GTK_CONTAINER(_w.vbox), 6);
  gtk_container_add(GTK_CONTAINER(_window), _w.vbox);
  gtk_widget_show(_w.vbox);

  /*
   *  hbox
   */
  _w.hbox = gtk_hbox_new(FALSE, 2);
  gtk_widget_set_usize (_w.hbox, -2, 160);
  gtk_widget_show(_w.hbox);
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.hbox, TRUE, TRUE, 5);

  /*
   *  foreground
   */
  _w.fg_frame = gtk_frame_new(_("Foreground"));
  gtk_box_pack_start(GTK_BOX(_w.hbox), _w.fg_frame, TRUE, TRUE, 0);
  gtk_widget_show(_w.fg_frame);

  _w.fg_hbox = gtk_hbox_new(FALSE, 2);
  gtk_container_border_width(GTK_CONTAINER(_w.fg_hbox), 6);
  gtk_container_add(GTK_CONTAINER(_w.fg_frame), _w.fg_hbox);
  gtk_widget_show(_w.fg_hbox);

  /*
   *  background
   */
  _w.bg_frame = gtk_frame_new(_("Background"));
  gtk_box_pack_start(GTK_BOX(_w.hbox), _w.bg_frame, TRUE, TRUE, 0);
  gtk_widget_show(_w.bg_frame);

  _w.bg_hbox = gtk_hbox_new(FALSE, 2);
  gtk_container_border_width(GTK_CONTAINER(_w.bg_hbox), 6);
  gtk_container_add(GTK_CONTAINER(_w.bg_frame), _w.bg_hbox);
  gtk_widget_show(_w.bg_hbox);

  /*
   *  foreground saturation
   */
  _w.s_fg_frame = gtk_frame_new(_("Saturation"));
  gtk_box_pack_start(GTK_BOX(_w.fg_hbox), _w.s_fg_frame, TRUE, TRUE, 0);
  gtk_widget_show(_w.s_fg_frame);

  _w.s_fg_adj = gtk_adjustment_new(_saturation_fg, 0, 1.1, 0.01, 0.1, 0.1);
  gtk_object_set_data(GTK_OBJECT(_w.s_fg_adj), "self", this);
  gtk_signal_connect(GTK_OBJECT(_w.s_fg_adj), "value_changed",
		     GTK_SIGNAL_FUNC(sf_adjustment_changed),
		     &_saturation_fg);

  _w.s_fg_vscale = gtk_vscale_new(GTK_ADJUSTMENT(_w.s_fg_adj));
  gtk_scale_set_digits(GTK_SCALE(_w.s_fg_vscale), 2);
  gtk_container_add(GTK_CONTAINER(_w.s_fg_frame), _w.s_fg_vscale);
  gtk_widget_show(_w.s_fg_vscale);

  /*
   *  foreground brightness (= value of HSV)
   */
  _w.v_fg_frame = gtk_frame_new(_("Brightness"));
  gtk_box_pack_start(GTK_BOX(_w.fg_hbox), _w.v_fg_frame, TRUE, TRUE, 0);
  gtk_widget_show(_w.v_fg_frame);

  _w.v_fg_adj = gtk_adjustment_new(_brightness_fg, 0, 1.1, 0.01, 0.1, 0.1);
  gtk_object_set_data(GTK_OBJECT(_w.v_fg_adj), "self", this);
  gtk_signal_connect(GTK_OBJECT(_w.v_fg_adj), "value_changed",
		     GTK_SIGNAL_FUNC(sf_adjustment_changed),
		     &_brightness_fg);

  _w.v_fg_vscale = gtk_vscale_new(GTK_ADJUSTMENT(_w.v_fg_adj));
  gtk_scale_set_digits(GTK_SCALE(_w.v_fg_vscale), 2);
  gtk_container_add(GTK_CONTAINER(_w.v_fg_frame), _w.v_fg_vscale);
  gtk_widget_show (_w.v_fg_vscale);

  /*
   *  background saturation
   */
  _w.s_bg_frame = gtk_frame_new(_("Saturation"));
  gtk_box_pack_start(GTK_BOX(_w.bg_hbox), _w.s_bg_frame, TRUE, TRUE, 0);
  gtk_widget_show(_w.s_bg_frame);

  _w.s_bg_adj = gtk_adjustment_new(_saturation_bg, 0, 1.1, 0.01, 0.1, 0.1);
  gtk_object_set_data(GTK_OBJECT(_w.s_bg_adj), "self", this);
  gtk_signal_connect(GTK_OBJECT(_w.s_bg_adj), "value_changed",
		     GTK_SIGNAL_FUNC(sf_adjustment_changed),
		     &_saturation_bg);

  _w.s_bg_vscale = gtk_vscale_new(GTK_ADJUSTMENT(_w.s_bg_adj));
  gtk_scale_set_digits(GTK_SCALE(_w.s_bg_vscale), 2);
  gtk_container_add(GTK_CONTAINER(_w.s_bg_frame), _w.s_bg_vscale);
  gtk_widget_show(_w.s_bg_vscale);

  /*
   *  background brightness (= value of HSV)
   */
  _w.v_bg_frame = gtk_frame_new(_("Brightness"));
  gtk_box_pack_start(GTK_BOX(_w.bg_hbox), _w.v_bg_frame, TRUE, TRUE, 0);
  gtk_widget_show(_w.v_bg_frame);

  _w.v_bg_adj = gtk_adjustment_new(_brightness_bg, 0, 1.1, 0.01, 0.1, 0.1);
  gtk_object_set_data(GTK_OBJECT(_w.v_bg_adj), "self", this);
  gtk_signal_connect(GTK_OBJECT(_w.v_bg_adj), "value_changed",
		     GTK_SIGNAL_FUNC(sf_adjustment_changed),
		     &_brightness_bg);

  _w.v_bg_vscale = gtk_vscale_new(GTK_ADJUSTMENT(_w.v_bg_adj));
  gtk_scale_set_digits(GTK_SCALE(_w.v_bg_vscale), 2);
  gtk_container_add(GTK_CONTAINER(_w.v_bg_frame), _w.v_bg_vscale);
  gtk_widget_show (_w.v_bg_vscale);

  /*
   *  black level (= value of HSV for black pixels)
   */
  _w.b_frame = gtk_frame_new(_("Black Level"));
  gtk_box_pack_start(GTK_BOX(_w.hbox), _w.b_frame, TRUE, TRUE, 0);
  gtk_widget_show(_w.b_frame);

  _w.b_adj = gtk_adjustment_new(_black_level, 0, 1.1, 0.01, 0.1, 0.1);
  gtk_object_set_data(GTK_OBJECT(_w.b_adj), "self", this);
  gtk_signal_connect(GTK_OBJECT(_w.b_adj), "value_changed",
		     GTK_SIGNAL_FUNC(sf_adjustment_changed),
		     &_black_level);

  _w.b_vscale = gtk_vscale_new(GTK_ADJUSTMENT(_w.b_adj));
  gtk_scale_set_digits(GTK_SCALE(_w.b_vscale), 2);
  gtk_container_add(GTK_CONTAINER(_w.b_frame), _w.b_vscale);
  gtk_widget_show (_w.b_vscale);

  /*
   *  white level (= value of HSV for white pixels)
   */
  _w.w_frame = gtk_frame_new(_("White Level"));
  gtk_box_pack_start(GTK_BOX(_w.hbox), _w.w_frame, TRUE, TRUE, 0);
  gtk_widget_show(_w.w_frame);

  _w.w_adj = gtk_adjustment_new(_white_level, 0, 1.1, 0.01, 0.1, 0.1);
  gtk_object_set_data(GTK_OBJECT(_w.w_adj), "self", this);
  gtk_signal_connect(GTK_OBJECT(_w.w_adj), "value_changed",
		     GTK_SIGNAL_FUNC(sf_adjustment_changed),
		     &_white_level);

  _w.w_vscale = gtk_vscale_new(GTK_ADJUSTMENT(_w.w_adj));
  gtk_scale_set_digits(GTK_SCALE(_w.w_vscale), 2);
  gtk_container_add(GTK_CONTAINER(_w.w_frame), _w.w_vscale);
  gtk_widget_show (_w.w_vscale);

  /*
   *  separator
   */
  _w.separator = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.separator,
                     FALSE, FALSE, 5);
  gtk_widget_show(_w.separator);
  
  /*
   *  close button
   */
  _w.close = gtk_button_new_with_label(_("Close"));
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.close,
                     FALSE, FALSE, 5);
  gtk_signal_connect(GTK_OBJECT(_w.close), "clicked",
                     GTK_SIGNAL_FUNC(cmd_exec_sf),
                     (char *)"ui-color-window-toggle"); // FIXME:
  GTK_WIDGET_SET_FLAGS(_w.close, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(_w.close);
  gtk_widget_show(_w.close);

  _cmd = new CMD_color_window_toggle(this);
}
