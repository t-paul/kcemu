/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: tapeadd.cc,v 1.2 2001/04/14 15:17:03 tp Exp $
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

#include "cmd/cmd.h"

#include "ui/status.h"
#include "ui/gtk/cmd.h"
#include "ui/gtk/tapeadd.h"

class CMD_tape_add_window_toggle : public CMD
{
private:
  TapeAddWindow *_w;
  
public:
  CMD_tape_add_window_toggle(TapeAddWindow *w) : CMD("ui-tape-add-window-toggle")
    {
      _w = w;
      register_cmd("ui-tape-add-window-toggle");
    }
  
  void execute(CMD_Args *args, CMD_Context context)
    {
      _w->toggle();
    }
};

void
TapeAddWindow::init(void)
{
  GtkWidget *w;
  
  /*
   *  window
   */
  _window = gtk_window_new(GTK_WINDOW_DIALOG);
  gtk_window_set_title(GTK_WINDOW(_window), _("KCemu: Add File"));
  gtk_window_position(GTK_WINDOW(_window), GTK_WIN_POS_MOUSE);
  gtk_signal_connect(GTK_OBJECT(_window), "destroy",
                     GTK_SIGNAL_FUNC(gtk_widget_destroyed),
                     &_window);

  /*
   *  vbox
   */
  _tape_add.vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_border_width(GTK_CONTAINER(_tape_add.vbox), 6);
  gtk_container_add(GTK_CONTAINER(_window), _tape_add.vbox);
  gtk_widget_show(_tape_add.vbox);

  /*
   *  table
   */
  _tape_add.table = gtk_table_new(5, 3, FALSE);
  gtk_box_pack_start(GTK_BOX(_tape_add.vbox), _tape_add.table, FALSE, TRUE, 0);
  gtk_widget_show(_tape_add.table);

  /*
   *  filename
   */
  w = gtk_label_new(_("Filename:"));
  gtk_table_attach_defaults(GTK_TABLE(_tape_add.table), w,
                            0, 1, 0, 1);
  gtk_widget_show(w);  
  _tape_add.filename = gtk_entry_new();
  gtk_table_attach_defaults(GTK_TABLE(_tape_add.table), _tape_add.filename,
                            1, 2, 0, 1);
  gtk_widget_show(_tape_add.filename);
  w = gtk_button_new_with_label(_("Browse"));
  gtk_table_attach_defaults(GTK_TABLE(_tape_add.table), w,
                            2, 3, 0, 1);
  /*
  gtk_signal_connect(GTK_OBJECT(w), "clicked",
                     GTK_SIGNAL_FUNC(cmd _ execute),
                     new CMD_Browse(GTK_ENTRY(_tape_add.filename)));*/
  gtk_widget_show(w);

  /*
   *  tapename
   */
  w = gtk_label_new(_("Tapename:"));
  gtk_table_attach_defaults(GTK_TABLE(_tape_add.table), w,
                            0, 1, 1, 2);
  gtk_widget_show(w);
  _tape_add.tapename = gtk_entry_new();
  gtk_table_attach_defaults(GTK_TABLE(_tape_add.table), _tape_add.tapename,
                            1, 3, 1, 2);
  gtk_widget_show(_tape_add.tapename);

  /*
   *  kc name
   */
  w = gtk_label_new(_("KC name:"));
  gtk_table_attach_defaults(GTK_TABLE(_tape_add.table), w,
                            0, 1, 2, 3);
  gtk_widget_show(w);
  _tape_add.kcname = gtk_entry_new();
  gtk_table_attach_defaults(GTK_TABLE(_tape_add.table), _tape_add.kcname,
                            1, 3, 2, 3);
  gtk_widget_show(_tape_add.kcname);

  /*
   *  load addr
   */
  w = gtk_label_new(_("load addr:"));
  gtk_table_attach_defaults(GTK_TABLE(_tape_add.table), w,
                            0, 1, 3, 4);
  gtk_widget_show(w);
  _tape_add.load_addr = gtk_entry_new();
  gtk_table_attach_defaults(GTK_TABLE(_tape_add.table), _tape_add.load_addr,
                            1, 3, 3, 4);
  gtk_widget_show(_tape_add.load_addr);

  /*
   *  start addr
   */
  w = gtk_label_new(_("start addr:"));
  gtk_table_attach_defaults(GTK_TABLE(_tape_add.table), w,
                            0, 1, 4, 5);
  gtk_widget_show(w);
  _tape_add.start_addr = gtk_entry_new();
  gtk_table_attach_defaults(GTK_TABLE(_tape_add.table), _tape_add.start_addr,
                            1, 3, 4, 5);
  gtk_widget_show(_tape_add.start_addr);

  /*
   *  separator
   */
  _tape_add.separator = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(_tape_add.vbox), _tape_add.separator,
                     FALSE, FALSE, 5);
  gtk_widget_show(_tape_add.separator);

  /*
   *  button box
   */
  _tape_add.bbox = gtk_hbutton_box_new();
  //gtk_button_box_set_spacing(GTK_BUTTON_BOX(_tape_add.bbox), 0);
  gtk_button_box_set_layout(GTK_BUTTON_BOX(_tape_add.bbox), GTK_BUTTONBOX_SPREAD);
  //gtk_button_box_set_child_size(GTK_BUTTON_BOX(_tape_add.bbox), 0, 0);
  gtk_box_pack_start(GTK_BOX(_tape_add.vbox), _tape_add.bbox, FALSE, TRUE, 0);
  gtk_widget_show(_tape_add.bbox);
  
  /*
   *  ok button
   */
  _tape_add.b_ok = gtk_button_new_with_label(_("Ok"));
  gtk_container_add(GTK_CONTAINER(_tape_add.bbox), _tape_add.b_ok);
  /*
    gtk_signal_connect_object(GTK_OBJECT(_tape_add.b_ok), "clicked",
    GTK_SIGNAL_FUNC(gtk_widget_destroy),
    GTK_OBJECT(_window));
  */
  gtk_object_set_user_data(GTK_OBJECT(_tape_add.b_ok), this);
  /*
  gtk_signal_connect(GTK_OBJECT(_tape_add.b_ok), "clicked",
  GTK_SIGNAL_FUNC(cmd _ execute_object), _cmd);*/

  GTK_WIDGET_SET_FLAGS(_tape_add.b_ok, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(_tape_add.b_ok);
  gtk_widget_show(_tape_add.b_ok);

  /*
   *  close button
   */
  _tape_add.b_close = gtk_button_new_with_label(_("Close"));
  gtk_container_add(GTK_CONTAINER(_tape_add.bbox), _tape_add.b_close);
  gtk_signal_connect_object(GTK_OBJECT(_tape_add.b_close), "clicked",
                            GTK_SIGNAL_FUNC(gtk_widget_destroy),
                            GTK_OBJECT(_window));
  GTK_WIDGET_SET_FLAGS(_tape_add.b_close, GTK_CAN_DEFAULT);
  gtk_widget_show(_tape_add.b_close);

  CMD_tape_add_window_toggle *cmd = new CMD_tape_add_window_toggle(this);
}

#if 0
class CMD_AddRawFile : public Command
{
private:
  bool is_empty(const char *str)
    {
      int a;

      for (;*str != '\0';str++)
        if (!isspace(*str)) return false;
      return true;
    }
  bool string_to_ushort(const char *str, unsigned short *val)
    {
      unsigned long ret;
      char *endptr;

      *val = 0;
      if (str == NULL) return false;
      ret = strtoul(str, &endptr, 0);
      if (ret > 0xffff) return false;
      if (endptr[0] == '\0')
        {
          *val = ret;
          return true;
        }
      return false;
    }
public:
  CMD_AddRawFile(void) {}
  virtual ~CMD_AddRawFile(void) {}
  void execute(void *arg)
    {
      if (arg == NULL)
        new TapeAddWindow(this);
      else
        {
          const char *str;
          const char *filename;
          TapeAddWindow *w;
          unsigned short load;
          unsigned short start;

          w = (TapeAddWindow *)gtk_object_get_user_data(GTK_OBJECT(arg));
          filename = w->getFileName();
          if (is_empty(filename))
            {
              Error::instance()->info(_("no filename given!"));
              return;
            }
          str = w->getLoadAddr();
          if (is_empty(str))
            {
              Error::instance()->info(_("need load address!"));
              return;
            }
          if (!string_to_ushort(str, &load))
            {
              Error::instance()->info(_("invalid value for load address!"));
              return;
            }
          str = w->getStartAddr();
          if (is_empty(str))
            start = 0xffff;
          else
            if (!string_to_ushort(w->getStartAddr(), &start))
              {
                Error::instance()->info(_("invalid value for start address!"));
                return;
              }
          tape->add_raw(filename, load, start);
          delete w;
        }
    }
};
#endif
