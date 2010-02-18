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

#ifndef __ui_gtk_disk_h
#define __ui_gtk_disk_h

#include <gtk/gtk.h>

#include "kc/system.h"

#include "ui/gtk/window.h"

class DiskWindow : public UI_Gtk_Window
{
  enum {
    NR_OF_DISKS = 4
  };
    
 private:
  struct {
    GtkWidget *vbox;
    GtkWidget *table;
    GtkWidget *label[NR_OF_DISKS];
    GtkWidget *combo[NR_OF_DISKS];
    GtkWidget *browse[NR_OF_DISKS];
    GtkWidget *eject[NR_OF_DISKS];
    GtkWidget *separator;
    GtkWidget *close;
    guint combo_signal_id[NR_OF_DISKS];
  } _w;

  CMD *_cmd_attach;
  CMD *_cmd_update;
  CMD *_cmd_window_toggle;

 protected:
  void init(void);

  static void sf_disk_attach(GtkWidget *widget, gpointer data);

 public:
  DiskWindow(const char *ui_xml_file);
  virtual ~DiskWindow(void);

  virtual void set_name(int idx, const char *name);
};

#endif /* __ui_gtk_disk_h */
