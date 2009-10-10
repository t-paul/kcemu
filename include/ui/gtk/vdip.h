/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2009 Torsten Paul
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

#ifndef __ui_gtk_vdip_h
#define __ui_gtk_vdip_h

#include <gtk/gtk.h>

#include "kc/system.h"

#include "ui/gtk/window.h"

class VDIPWindow : public UI_Gtk_Window
{
 private:
  struct {
    GtkWidget *vbox;
    GtkWidget *table;
    GtkWidget *label[2];
    GtkWidget *combo[2];
    GtkWidget *browse[2];
    GtkWidget *eject[2];
    GtkWidget *separator;
    GtkWidget *close;
    guint combo_signal_id[2];
  } _w;

  CMD *_cmd_attach;
  CMD *_cmd_update;
  CMD *_cmd_window_toggle;

 protected:
  void init(void);

  static void sf_vdip_attach(GtkWidget *widget, gpointer data);

 public:
  VDIPWindow(const char *ui_xml_file);
  virtual ~VDIPWindow(void);

  virtual void set_name(int idx, const char *name);
};

#endif /* __ui_gtk_vdip_h */
