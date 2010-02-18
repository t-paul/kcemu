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

#ifndef __ui_gtk_fbrowse_h
#define __ui_gtk_fbrowse_h

#include <gtk/gtk.h>

#include "kc/system.h"

#include "cmd/cmd.h"

class FileBrowser : public CMD
{
 private:
  CMD_Args *_args;

 public:
  FileBrowser(void);
  virtual ~FileBrowser(void);

 protected:
  void init(const char *title, const char *path, bool dironly);
  static void sf_response(GtkDialog *dialog, gint response_id, gpointer user_data);

public:
  void execute(CMD_Args *args, CMD_Context context);
};


#endif /* __ui_gtk_fbrowse_h */
