/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-1998 Torsten Paul
 *
 *  $Id: tapeadd.h,v 1.1 2000/05/21 17:38:46 tp Exp $
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

#ifndef __ui_gtk_tapeadd_h
#define __ui_gtk_tapeadd_h

#include <gtk/gtk.h>

#include "kc/config.h"
#include "kc/system.h"

#include "ui/gtk/window.h"

class TapeAddWindow : public UI_Gtk_Window
{
 private:
  struct {
    GtkWidget *vbox;
    GtkWidget *table;
    GtkWidget *filename;
    GtkWidget *tapename;
    GtkWidget *kcname;
    GtkWidget *load_addr;
    GtkWidget *start_addr;
    GtkWidget *separator;
    GtkWidget *bbox;
    GtkWidget *b_ok;
    GtkWidget *b_close;
  } _tape_add;

 public:
  TapeAddWindow(void)
    {
      init();
    }
  virtual ~TapeAddWindow(void)
    {
      gtk_widget_destroy(_window);
    }

  void init(void);
  const char * getFileName(void)
    {
      return gtk_entry_get_text(GTK_ENTRY(_tape_add.filename));
    } 
  const char * getTapeName(void)
    {
      return gtk_entry_get_text(GTK_ENTRY(_tape_add.tapename));
    }
  const char * getKCName(void)
    {
      return gtk_entry_get_text(GTK_ENTRY(_tape_add.kcname));
    }
  const char * getLoadAddr(void)
    {
      return gtk_entry_get_text(GTK_ENTRY(_tape_add.load_addr));
    }
  const char * getStartAddr(void)
    {
      return gtk_entry_get_text(GTK_ENTRY(_tape_add.start_addr));
    }
};

#endif /* __ui_gtk_tapeadd_h */
