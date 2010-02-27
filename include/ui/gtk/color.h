/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: color.h,v 1.4 2002/10/31 01:38:07 torsten_paul Exp $
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
    GtkWidget *b_fg_vscale;
    GtkWidget *b_bg_vscale;
    GtkWidget *c_fg_vscale;
    GtkWidget *c_bg_vscale;

    GtkAdjustment *s_fg_adj;
    GtkAdjustment *s_bg_adj;
    GtkAdjustment *v_fg_adj;
    GtkAdjustment *v_bg_adj;
  } _w;

  double _brightness_fg;
  double _brightness_bg;
  double _contrast_fg;
  double _contrast_bg;

  CMD *_cmd;

 protected:
  static void sf_adjustment_changed(GtkAdjustment *adj, double *data);

  void init(void);
  GtkAdjustment * init_adjustment(GtkRange* range, double *val_ptr);

 public:
  ColorWindow(const char *ui_xml_file);
  virtual ~ColorWindow(void);

  virtual double get_brightness_fg() { return _brightness_fg; }
  virtual double get_brightness_bg() { return _brightness_bg; }
  virtual double get_contrast_fg() { return _contrast_fg; }
  virtual double get_contrast_bg() { return _contrast_bg; }
};

#endif /* __ui_gtk_color_h */
