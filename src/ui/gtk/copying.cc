/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: copying.cc,v 1.8 2002/06/09 14:24:34 torsten_paul Exp $
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

#include "ui/copying.h"
#include "ui/gtk/cmd.h"
#include "ui/gtk/copying.h"

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
  _font_desc = pango_font_description_new();
  pango_font_description_set_family(_font_desc, "Courier");
  pango_font_description_set_style(_font_desc, PANGO_STYLE_NORMAL);
  pango_font_description_set_variant(_font_desc, PANGO_VARIANT_NORMAL);
  pango_font_description_set_weight(_font_desc, PANGO_WEIGHT_NORMAL);
  pango_font_description_set_stretch(_font_desc, PANGO_STRETCH_NORMAL);
  pango_font_description_set_size(_font_desc, 8 * PANGO_SCALE);

  /*
   *  copying window
   */
  _window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
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
   *  label vbox
   */
  _w.label_vbox = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(_w.label_vbox);

  /*
   *  label copying
   */
  _w.label_copying = gtk_label_new(gnu_general_public_licence);
  gtk_box_pack_start(GTK_BOX(_w.label_vbox), _w.label_copying, FALSE, FALSE, 0);
  gtk_widget_modify_font(_w.label_copying, _font_desc);
  gtk_widget_show(_w.label_copying);

  /*
   *  label warranty
   */
  _w.label_warranty = gtk_label_new(gnu_general_public_licence_warranty);
  gtk_box_pack_start(GTK_BOX(_w.label_vbox), _w.label_warranty, FALSE, FALSE, 0);
  gtk_widget_modify_font(_w.label_warranty, _font_desc);
  gtk_widget_show(_w.label_warranty);

  /*
   *  scrolled window
   */
  _w.scrolled_window = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(_w.scrolled_window), _w.label_vbox);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(_w.scrolled_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.scrolled_window, TRUE, TRUE, 6);
  gtk_widget_set_usize(_w.scrolled_window, -1, 450);
  gtk_widget_show(_w.scrolled_window);

  /*
   *  separator
   */
  _w.separator = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.separator, FALSE, FALSE, 5);
  gtk_widget_show(_w.separator);
  
  /*
   *  close button
   */
  _w.close = gtk_button_new_with_label(_("Close"));
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.close, FALSE, FALSE, 5);
  gtk_signal_connect(GTK_OBJECT(_w.close), "clicked",
                     GTK_SIGNAL_FUNC(cmd_exec_sf),
                     (char *)"ui-copying-window-toggle"); // FIXME:
  GTK_WIDGET_SET_FLAGS(_w.close, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(_w.close);
  gtk_widget_show(_w.close);
  
  /*
   *  force allocation calculation for this window; so we get
   *  the values for the label width
   */
  gtk_widget_realize(_window);
}

void
CopyingWindow::scroll_to_copying(void)
{
  GtkAdjustment* adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(_w.scrolled_window));
  if (adj == NULL)
    return;

  adj->value = 0.0;
  gtk_signal_emit_by_name(GTK_OBJECT(adj), "value_changed");
}

void
CopyingWindow::scroll_to_warranty(void)
{
  GtkAdjustment* adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(_w.scrolled_window));
  if (adj == NULL)
    return;

  adj->value = _w.label_copying->requisition.height;
  gtk_signal_emit_by_name(GTK_OBJECT(adj), "value_changed");
}

