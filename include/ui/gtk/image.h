/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-1998 Torsten Paul
 *
 *  $Id: image.h,v 1.1 2000/05/21 17:38:45 tp Exp $
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

#ifndef __ui_gtk_image_h
#define __ui_gtk_image_h

#include <gtk/gtk.h>

#include "kc/config.h"
#include "kc/system.h"

#include "cmd/cmd.h"

class CMD_ui_image_props : public CMD
{
 private:
  CMD_Args *_args;
  struct
  {
    GtkWidget *dialog;
    GtkWidget *load_addr;
    GtkWidget *start_addr;
  } _w;
  bool _autostart;
  
 protected:
  static void ok(GtkWidget */*widget*/, gpointer data);
  static void cancel(GtkWidget */*widget*/, gpointer data);
  static int delete_event(GtkWidget */*widget*/, GdkEvent *event, gpointer data);
  static void toggle_autostart(GtkWidget *checkbutton, gpointer data);

  virtual bool check_values(CMD_Args *args);
  
 public:
  CMD_ui_image_props(void);
  virtual ~CMD_ui_image_props(void);
  
  virtual void execute(CMD_Args *args, CMD_Context context);
};

#endif /* __ui_gtk_image_h */
