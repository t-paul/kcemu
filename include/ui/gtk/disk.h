/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: disk.h,v 1.2 2001/04/14 15:15:24 tp Exp $
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

#ifndef __ui_gtk_disk_h
#define __ui_gtk_disk_h

#include <gtk/gtk.h>

#include "kc/config.h"
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
    GtkWidget *bbox;
    GtkWidget *b_ok;
    GtkWidget *b_apply;
    GtkWidget *b_close;
  } _w;

 protected:
  void init(void);

 public:
  DiskWindow(void) { init(); }
  virtual ~DiskWindow(void) {}

  virtual void set_name(int idx, const char *name);
};

#endif /* __ui_gtk_disk_h */
