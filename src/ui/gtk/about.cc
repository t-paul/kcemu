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

#include <string>

#include "kc/system.h"

#include "ui/gtk/cmd.h"
#include "ui/gtk/about.h"

using namespace std;

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

AboutWindow::AboutWindow(const char *ui_xml_file) : UI_Gtk_Window(ui_xml_file)
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
  string version_text(string("<big><b>") + APP_NAME + "</b></big>");
  
  _window = get_widget("about_window");
  gtk_signal_connect(GTK_OBJECT(_window), "delete_event",
		     GTK_SIGNAL_FUNC(cmd_exec_sft),
		     (char *)"ui-about-window-toggle"); // FIXME:

  GtkWidget *version = get_widget("main_label_version");
  gtk_label_set_text(GTK_LABEL(version), version_text.c_str());
  gtk_label_set_use_markup(GTK_LABEL(version), TRUE);

  GtkWidget *build = get_widget("main_label_build");
  gtk_label_set_text(GTK_LABEL(build), "build: " __DATE__ " / " __TIME__);

  init_dialog("ui-about-window-toggle", NULL);
}
