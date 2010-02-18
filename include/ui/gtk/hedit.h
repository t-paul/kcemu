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

#ifndef __ui_gtk_hedit_h
#define __ui_gtk_hedit_h

#include <gtk/gtk.h>

#include "kc/system.h"

#include "cmd/cmd.h"

#include "ui/gtk/window.h"

class EditHeaderWindow : public CMD
{
 private:
  CMD_Args *_args;
  struct
  {
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *table;
    GtkWidget *tapename;
    GtkWidget *kcname;
    GtkWidget *load_addr;
    GtkWidget *start_addr;
    GtkWidget *separator;
    GtkWidget *bbox;
    GtkWidget *b_ok;
    GtkWidget *b_close;
  } _w;
  const char    *_tape_filename;
  const char    *_kc_filename;
  unsigned long  _load_address;
  unsigned long  _start_address;
  bool           _autostart;

 protected:
  void init(const char *tapename, const char *kcname);
  bool check_values(CMD_Args *args);
  static void ok(GtkWidget */*widget*/, gpointer data);
  static void cancel(GtkWidget */*widget*/, gpointer data);
  static int delete_event(GtkWidget */*widget*/,
                          GdkEvent *event,
                          gpointer data);
  static void sf_toggle_autostart(GtkWidget *checkbutton, gpointer data);

 public:
  EditHeaderWindow(void);
  virtual ~EditHeaderWindow(void);
  
  void execute(CMD_Args *args, CMD_Context context);
  const char * get_tape_filename(void)
    {
      return gtk_entry_get_text(GTK_ENTRY(_w.tapename));
    }
  const char * get_kc_filename(void)
    {
      return gtk_entry_get_text(GTK_ENTRY(_w.kcname));
    }
  const char * get_load_address(void)
    {
      return gtk_entry_get_text(GTK_ENTRY(_w.load_addr));
    }
  const char * get_start_address(void)
    {
      return gtk_entry_get_text(GTK_ENTRY(_w.start_addr));
    }
};

#endif /* __ui_gtk_hedit_h */
