/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: color.h,v 1.3 2002/06/09 14:24:32 torsten_paul Exp $
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

#ifndef __ui_gtk_color_h
#define __ui_gtk_color_h

#include <gtk/gtk.h>

#include "kc/system.h"

#include "ui/gtk/window.h"

class ColorWindow : public UI_Gtk_Window
{
 private:
  struct {
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *fg_frame;
    GtkWidget *fg_hbox;
    GtkWidget *bg_frame;
    GtkWidget *bg_hbox;
    GtkWidget *s_fg_frame;
    GtkWidget *s_fg_vscale;
    GtkWidget *s_bg_frame;
    GtkWidget *s_bg_vscale;
    GtkWidget *v_fg_frame;
    GtkWidget *v_fg_vscale;
    GtkWidget *v_bg_frame;
    GtkWidget *v_bg_vscale;
    GtkWidget *b_frame;
    GtkWidget *b_vscale;
    GtkWidget *w_frame;
    GtkWidget *w_vscale;
    GtkWidget *separator;
    GtkWidget *close;

    GtkObject *s_fg_adj;
    GtkObject *s_bg_adj;
    GtkObject *v_fg_adj;
    GtkObject *v_bg_adj;
    GtkObject *b_adj;
    GtkObject *w_adj;
  } _w;

  double _saturation_fg;
  double _saturation_bg;
  double _brightness_fg;
  double _brightness_bg;
  double _black_level;
  double _white_level;

 protected:
  static void sf_adjustment_changed(GtkAdjustment *adj, double *data);

  void init(void);

 public:
  ColorWindow(void) { init(); }
  virtual ~ColorWindow(void) {}

  virtual double get_saturation_fg() { return _saturation_fg; }
  virtual double get_saturation_bg() { return _saturation_bg; }
  virtual double get_brightness_fg() { return _brightness_fg; }
  virtual double get_brightness_bg() { return _brightness_bg; }
  virtual double get_black_level()   { return _black_level; }
  virtual double get_white_level()   { return _white_level; }
};

#endif /* __ui_gtk_color_h */


