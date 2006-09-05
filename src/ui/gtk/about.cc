/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2006 Torsten Paul
 *
 *  $Id: about.cc,v 1.11 2002/10/31 01:38:12 torsten_paul Exp $
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

#include "kc/system.h"

#include "ui/gtk/cmd.h"
#include "ui/gtk/about.h"
#include "ui/gtk/glade/interface.h"

char AboutWindow::APP_NAME[]      = ("KCemu v" KCEMU_VERSION);

class CMD_about_window_toggle : public CMD
{
private:
  AboutWindow *_w;

public:
  CMD_about_window_toggle(AboutWindow *w) : CMD("ui-about-window-toggle")
    {
      _w = w;
      register_cmd("ui-about-window-toggle");
    }

  void execute(CMD_Args *args, CMD_Context context)
    {
      _w->toggle();
    }
};

AboutWindow::AboutWindow(void)
{
  _cmd = new CMD_about_window_toggle(this);
}

AboutWindow::~AboutWindow(void)
{
  delete _cmd;
}

void
AboutWindow::init(void)
{
  _window = create_about_window();
  gtk_signal_connect(GTK_OBJECT(_window), "delete_event",
		     GTK_SIGNAL_FUNC(cmd_exec_sft),
		     (char *)"ui-about-window-toggle"); // FIXME:

  GtkWidget *version = get_widget("main_label_version");
  gtk_label_set_text(GTK_LABEL(version), APP_NAME);

  GtkWidget *build = get_widget("main_label_build");
  gtk_label_set_text(GTK_LABEL(build), "build: " __DATE__ " / " __TIME__);

  init_dialog("ui-about-window-toggle", NULL);
}
