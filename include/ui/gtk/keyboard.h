/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: keyboard.h,v 1.1 2002/10/31 01:38:07 torsten_paul Exp $
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

#ifndef __ui_gtk_keyboard_h
#define __ui_gtk_keyboard_h

#include <gtk/gtk.h>

#include "kc/system.h"

#include "ui/gtk/window.h"

struct _key_struct {
  const char   *key;
  int           key_val;
  GdkRectangle  rect;
  GdkRegion    *region;
};

class KeyboardWindow : public UI_Gtk_Window
{
 private:
  struct {
    GtkWidget *vbox;
    GtkWidget *eventbox;
    GtkWidget *canvas;
    GtkWidget *label;
    GtkWidget *separator;
    GtkWidget *close;
  } _w;
  
  GdkPixbuf   *_pixbuf_normal;
  GdkPixbuf   *_pixbuf_pressed;
  gboolean     _key_active;

  struct _key_struct _keys[256];
  struct _key_struct *_key_pressed;

 protected:
  void init(void);
  void init_key_regions(void);
  int get_key_val(const char *key);
  void draw_key_normal(struct _key_struct *key);
  void draw_key_pressed(struct _key_struct *key);

  void debug_regions(GdkEventMotion *event);

  static gboolean sf_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data);
  static gboolean sf_motion_notify(GtkWidget *widget, GdkEventMotion *event, gpointer data);
  static gboolean sf_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data);
  static gboolean sf_button_release(GtkWidget *widget, GdkEventButton *event, gpointer data);
  static gboolean sf_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data);
  static gboolean sf_key_release(GtkWidget *widget, GdkEventKey *event, gpointer data);

 public:
  KeyboardWindow(void);
  virtual ~KeyboardWindow(void);
};

#endif /* __ui_gtk_keyboard_h */
