/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
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

#include "ui/gtk/kcemu.xpm"

char AboutWindow::APP_NAME[]      = ("KCemu v" KCEMU_VERSION);
char AboutWindow::APP_COPYRIGHT[] = ("(c) 1997-2003 Torsten Paul\n"
				     "<Torsten.Paul@gmx.de>");

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
  init();
}

AboutWindow::~AboutWindow(void)
{
  delete _cmd;
}

void
AboutWindow::init(void)
{
  _window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name(_window, "AboutWindow");
  gtk_window_set_title(GTK_WINDOW(_window), _("About KCemu"));
  gtk_window_set_resizable(GTK_WINDOW(_window), false);
  gtk_window_position(GTK_WINDOW(_window), GTK_WIN_POS_MOUSE);
  gtk_signal_connect(GTK_OBJECT(_window), "delete_event",
                     GTK_SIGNAL_FUNC(cmd_exec_sft),
                     (char *)"ui-about-window-toggle"); // FIXME:
      
  /*
   *  vbox
   */
  _w.vbox = gtk_vbox_new(FALSE, 2);
  gtk_container_border_width(GTK_CONTAINER(_w.vbox), 6);
  gtk_container_add(GTK_CONTAINER(_window), _w.vbox);
  gtk_widget_show(_w.vbox);

  /*
   *  realize window for pixmap creation
   */
  gtk_widget_realize(_window);

  /*
   *  pixmap
   */
  _w.pixmap = create_pixmap_widget(_window, __xpm_kcemu);
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.pixmap, FALSE, FALSE, 2);
  gtk_widget_show(_w.pixmap);
  
  /*
   *  labels
   */
  _w.name = gtk_label_new(APP_NAME);
  _w.copyright = gtk_label_new(APP_COPYRIGHT);
  _w.info = gtk_label_new("build: " __DATE__ " / " __TIME__);
  gtk_label_set_justify(GTK_LABEL(_w.copyright), GTK_JUSTIFY_CENTER);
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.name,
                     FALSE, FALSE, 5);
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.copyright,
                     FALSE, FALSE, 5);
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.info,
                     FALSE, FALSE, 5);
  gtk_widget_show(_w.name);
  gtk_widget_show(_w.copyright);
  gtk_widget_show(_w.info);
  
  /*
   *  separator
   */
  _w.separator = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.separator,
                     FALSE, FALSE, 5);
  gtk_widget_show(_w.separator);
  
  /*
   *  close button
   */
  _w.close = gtk_button_new_with_label(_("Close"));
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.close,
                     FALSE, FALSE, 5);
  gtk_signal_connect(GTK_OBJECT(_w.close), "clicked",
                     GTK_SIGNAL_FUNC(cmd_exec_sf),
                     (char *)"ui-about-window-toggle"); // FIXME:
  GTK_WIDGET_SET_FLAGS(_w.close, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(_w.close);
  gtk_widget_show(_w.close);

  _cmd = new CMD_about_window_toggle(this);
}
