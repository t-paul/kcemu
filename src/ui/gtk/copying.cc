/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: copying.cc,v 1.7 2001/04/14 15:16:51 tp Exp $
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

#include "kc/config.h"
#include "kc/system.h"

#include "ui/copying.h"
#include "ui/gtk/cmd.h"
#include "ui/gtk/copying.h"

const char CopyingWindow::COPYING_FONT[] = "fixed";

class CMD_copying_window_toggle : public CMD
{
private:
  CopyingWindow *_w;

public:
  CMD_copying_window_toggle(CopyingWindow *w) : CMD("ui-copying-window-toggle")
    {
      _w = w;
      register_cmd("ui-copying-window-toggle", 0);
      register_cmd("ui-warranty-window-toggle", 1);
    }
  
  void execute(CMD_Args *args, CMD_Context context)
    {
      _w->toggle();
      if (context == 0)
        _w->scroll_to_copying();
      else
        _w->scroll_to_warranty();
    }
};

CopyingWindow::CopyingWindow(void)
{
  init();
  _cmd = new CMD_copying_window_toggle(this);
}

CopyingWindow::~CopyingWindow(void)
{
  delete _cmd;
}

void
CopyingWindow::init(void)
{
  char **ptr;
  GdkFont *font;
  GtkStyle *style;
  int width, height;

  /*
   *  copying window
   */
  _window = gtk_window_new(GTK_WINDOW_DIALOG);
  gtk_widget_set_name(_window, "CopyingWindow");
  gtk_window_set_title(GTK_WINDOW(_window), _("KCemu: Copying"));
  gtk_window_position(GTK_WINDOW(_window), GTK_WIN_POS_MOUSE);
  gtk_signal_connect(GTK_OBJECT(_window), "delete_event",
                     GTK_SIGNAL_FUNC(cmd_exec_sft),
                     (char *)"ui-copying-window-toggle"); // FIXME:
  
  /*
   *  vbox
   */
  _w.vbox = gtk_vbox_new(FALSE, 2);
  gtk_container_border_width(GTK_CONTAINER(_w.vbox), 6);
  gtk_container_add(GTK_CONTAINER(_window), _w.vbox);
  gtk_widget_show(_w.vbox);
  
  /*
   *  hbox
   */
  _w.hbox = gtk_hbox_new(FALSE, 0);
  gtk_container_border_width(GTK_CONTAINER(_w.hbox), 0);
  gtk_container_add(GTK_CONTAINER(_w.vbox), _w.hbox);
  gtk_widget_show(_w.hbox);
  
  /*
   *  text
   */
  _w.text = gtk_text_new(NULL, NULL);
  gtk_text_set_editable(GTK_TEXT(_w.text), FALSE);
  gtk_text_set_line_wrap(GTK_TEXT(_w.text), TRUE);
  gtk_text_set_word_wrap(GTK_TEXT(_w.text), TRUE);
  gtk_widget_set_name(GTK_WIDGET(_w.text), "copying");
  gtk_box_pack_start(GTK_BOX(_w.hbox), _w.text,
                     TRUE, TRUE, 0);
  
  font = gdk_font_load(COPYING_FONT);
  style = gtk_widget_get_style(_w.text);
  style->font = font;
  gtk_widget_set_style(_w.text, style);
  width = 0;
  _lines = 0;
  ptr = gnu_general_public_licence;
  while (*ptr != NULL)
    {
      int l = gdk_string_width(font, *ptr++);
      if (l > width) width = l;
      _lines++;
    }
  width += 2 * gdk_char_width(font, 'W');
  height = 30 * (font->ascent + font->descent);
  _warranty_pos = (gnu_warranty_position *
                            (font->ascent + font->descent));
  gtk_widget_show(_w.text);
  gtk_widget_realize(_w.text);
  gtk_widget_set_usize(_w.text, width, height);

  /*
   *  scrollbar
   */
  _w.scrollbar = gtk_vscrollbar_new(GTK_TEXT(_w.text)->vadj);
  gtk_box_pack_start(GTK_BOX(_w.hbox), _w.scrollbar,
                     FALSE, FALSE, 0);
  gtk_widget_show(_w.scrollbar);
  
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
                     (char *)"ui-copying-window-toggle"); // FIXME:
  GTK_WIDGET_SET_FLAGS(_w.close, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(_w.close);
  gtk_widget_show(_w.close);
  
  ptr = gnu_general_public_licence;
  gtk_text_freeze(GTK_TEXT(_w.text));
  while (*ptr != NULL)
    gtk_text_insert(GTK_TEXT(_w.text), NULL, NULL, NULL, *ptr++, -1);
  gtk_text_thaw(GTK_TEXT(_w.text));

  gtk_widget_realize(_w.text);
  GTK_TEXT(_w.text)->vadj->value = GTK_TEXT(_w.text)->vadj->upper;
  gtk_signal_emit_by_name(GTK_OBJECT(GTK_TEXT(_w.text)->vadj), "value_changed");
}

void
CopyingWindow::scroll_to_copying(void)
{
  GtkAdjustment *vadj;

  vadj = GTK_TEXT(_w.text)->vadj;
  vadj->value = vadj->lower;
  gtk_signal_emit_by_name(GTK_OBJECT(vadj), "value_changed");
}

void
CopyingWindow::scroll_to_warranty(void)
{
  GtkAdjustment *vadj;

  vadj = GTK_TEXT(_w.text)->vadj;
  _text_pos = (((vadj->upper -  vadj->lower) / ((double)_lines + 1.0))
               * (double)gnu_warranty_position) + vadj->lower;
  vadj->value = _text_pos;
  gtk_signal_emit_by_name(GTK_OBJECT(vadj), "value_changed");
}

