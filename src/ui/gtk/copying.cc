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

#include "kc/kc.h"

#include "ui/gtk/cmd.h"
#include "ui/gtk/copying.h"

using namespace std;

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

CopyingWindow::CopyingWindow(const char *ui_xml_file) : UI_Gtk_Window(ui_xml_file)
{
  _font_desc = pango_font_description_new();
  pango_font_description_set_family(_font_desc, "Courier");
  pango_font_description_set_style(_font_desc, PANGO_STYLE_NORMAL);
  pango_font_description_set_variant(_font_desc, PANGO_VARIANT_NORMAL);
  pango_font_description_set_weight(_font_desc, PANGO_WEIGHT_NORMAL);
  pango_font_description_set_stretch(_font_desc, PANGO_STRETCH_NORMAL);
  pango_font_description_set_size(_font_desc, 10 * PANGO_SCALE);

  _cmd = new CMD_copying_window_toggle(this);
}

CopyingWindow::~CopyingWindow(void)
{
  delete _cmd;
  pango_font_description_free(_font_desc);
}

void
CopyingWindow::init(void)
{
  /*
   *  copying window
   */
  _window = get_widget("copying_window");
  gtk_signal_connect(GTK_OBJECT(_window), "delete_event",
		     GTK_SIGNAL_FUNC(cmd_exec_sft),
		     (char *)"ui-copying-window-toggle"); // FIXME:

  /*
   *  scrolled window
   */
  _w.scrolled_window = get_widget("main_scrolledwindow");

  /*
   *  label copying
   */
  _w.label_license = get_widget("main_label_license");
  gtk_label_set_text(GTK_LABEL(_w.label_license), kc_get_license());
  gtk_widget_modify_font(_w.label_license, _font_desc);

  /*
   *  label warranty
   */
  string warranty_text = kc_get_warranty();
  string text = warranty_text + kc_get_license_trailer();
  _w.label_warranty = get_widget("main_label_warranty");
  gtk_label_set_text(GTK_LABEL(_w.label_warranty), text.c_str());
  gtk_widget_modify_font(_w.label_warranty, _font_desc);

  init_dialog("ui-copying-window-toggle", NULL);

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

  adj->value = _w.label_license->requisition.height;
  gtk_signal_emit_by_name(GTK_OBJECT(adj), "value_changed");
}

