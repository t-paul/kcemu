/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: tapeadd.h,v 1.3 2002/06/09 14:24:32 torsten_paul Exp $
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

#include "kc/system.h"

#include "cmd/cmd.h"

#include "ui/gtk/window.h"

class TapeAddWindow : public UI_Gtk_Window, public CMD
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
    GtkWidget *b_cancel;
  } _tape_add;

  CMD_Args *_args;

 protected:
  static void ok(GtkWidget *widget, gpointer *data);
  static void cancel(GtkWidget *widget, gpointer *data);

 public:
  TapeAddWindow(void);
  ~TapeAddWindow(void);

  void init(const char *tapename, const char *text);
  void execute(CMD_Args *args, CMD_Context context);
};

#endif /* __ui_gtk_tapeadd_h */
