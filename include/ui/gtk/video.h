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

#ifndef __ui_gtk_video_h
#define __ui_gtk_video_h

#include "kc/system.h"

#include "ui/gtk/window.h"

class VideoWindow : public UI_Gtk_Window
{
private:

  UI_Gtk *_ui;
  
  struct
  {
    GtkWidget *encoder_combobox;
    GtkWidget *filebutton;
    GtkWidget *fps_combobox;
    GtkWidget *quality_spinbutton;
    GtkWidget *start_on_reset_checkbutton;
    GtkWidget *record_button;
    GtkWidget *stop_button;
    GtkWidget *pause_button;
  } _w;

  CMD *_cmd_video_toggle;
  CMD *_cmd_video_record;

protected:
  void init(void);
  void init_fps(GtkComboBox *combobox);
  void init_encoder(GtkComboBox *combobox);

  void set_state(int state);
  void set_config_widget_sensitivity(bool enable);

  static void on_record_clicked(GtkWidget *widget, gpointer user_data);
  static void on_stop_clicked(GtkWidget *widget, gpointer user_data);
  static void on_pause_clicked(GtkWidget *widget, gpointer user_data);
  static void on_config_changed(GtkWidget *widget, gpointer user_data);
  static void on_encoder_changed(GtkComboBox *combobox, gpointer user_data);

  static gint get_active_value_as_int(GtkComboBox *combobox, int column);
  
public:
  VideoWindow(const char *ui_xml_file, UI_Gtk *ui);
  virtual ~VideoWindow(void);
  
  void ui_set_state(int state);
};

#endif /* __ui_gtk_video_h */
