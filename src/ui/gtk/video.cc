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

#include "kc/kc.h"
#include "kc/system.h"

#include "cmd/cmd.h"

#include "ui/gtk/ui_gtk.h"

#include "ui/gtk/cmd.h"
#include "ui/gtk/video.h"

#include "ui/gtk/vnc.h"
#include "ui/gtk/gif.h"
#include "ui/gtk/xvid.h"
#include "ui/gtk/dirac.h"
#include "ui/gtk/schro.h"
#include "ui/gtk/ffmpeg.h"
#include "ui/gtk/theora.h"

#include "libgtkex/libgtkex.h"

class CMD_ui_video_window_toggle : public CMD
{
private:
  VideoWindow *_w;

public:

  CMD_ui_video_window_toggle(VideoWindow *w) : CMD("ui-video-window-toggle")
  {
    _w = w;
    register_cmd("ui-video-window-toggle");
  }

  void
  execute(CMD_Args *args, CMD_Context context)
  {
    _w->toggle();
  }
};

class CMD_ui_video_record : public CMD
{
private:
  VideoWindow *_w;

public:

  CMD_ui_video_record(VideoWindow *w) : CMD("ui-video-record")
  {
    _w = w;
    register_cmd("ui-video-reset", -1);
    register_cmd("ui-video-record", VideoEncoder::VIDEO_ENCODER_STATE_RECORD);
    register_cmd("ui-video-pause", VideoEncoder::VIDEO_ENCODER_STATE_PAUSE);
    register_cmd("ui-video-stop", VideoEncoder::VIDEO_ENCODER_STATE_STOP);
  }

  void
  execute(CMD_Args *args, CMD_Context context)
  {
    _w->ui_set_state(context);
  }
};

VideoWindow::VideoWindow(const char *ui_xml_file, UI_Gtk *ui) : UI_Gtk_Window(ui_xml_file)
{
  _ui = ui;
  _cmd_video_toggle = new CMD_ui_video_window_toggle(this);
  _cmd_video_record = new CMD_ui_video_record(this);
}

VideoWindow::~VideoWindow(void)
{
  delete _cmd_video_toggle;
  delete _cmd_video_record;
}

void
VideoWindow::set_state(int state)
{
  _ui->set_video_encoder_state(state);
}

void
VideoWindow::ui_set_state(int state)
{
  if (_window == 0) {
    return;
  }
  switch (state)
    {
    case VideoEncoder::VIDEO_ENCODER_STATE_RECORD:
      gtk_widget_set_sensitive(_w.encoder_combobox, false);
      gtk_widget_set_sensitive(_w.record_button, false);
      gtk_widget_set_sensitive(_w.stop_button, true);
      gtk_widget_set_sensitive(_w.pause_button, true);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(_w.record_button), TRUE);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(_w.pause_button), FALSE);
      set_config_widget_sensitivity(false);
      break;
    case VideoEncoder::VIDEO_ENCODER_STATE_STOP:
      gtk_widget_set_sensitive(_w.encoder_combobox, true);
      gtk_widget_set_sensitive(_w.record_button, true);
      gtk_widget_set_sensitive(_w.stop_button, false);
      gtk_widget_set_sensitive(_w.pause_button, false);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(_w.record_button), FALSE);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(_w.pause_button), FALSE);
      set_config_widget_sensitivity(true);
      break;
    case VideoEncoder::VIDEO_ENCODER_STATE_PAUSE:
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(_w.pause_button), TRUE);
      break;
    default:
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(_w.start_on_reset_checkbutton), FALSE);
      break;
    }
}

void
VideoWindow::set_config_widget_sensitivity(bool enable)
{
  gboolean enable_encoder = FALSE, enable_filebutton = FALSE, enable_config = FALSE;

  GtkTreeIter iter;
  if (enable && gtk_combo_box_get_active_iter(GTK_COMBO_BOX(_w.encoder_combobox), &iter))
    {
      enable_encoder = TRUE;
      gtk_tree_model_get(GTK_TREE_MODEL(gtk_combo_box_get_model(GTK_COMBO_BOX(_w.encoder_combobox))), &iter, 2, &enable_filebutton, 3, &enable_config, -1);
    }

  gtk_widget_set_sensitive(_w.filebutton, enable_filebutton);
  gtk_widget_set_sensitive(_w.fps_combobox, enable_config);
  gtk_widget_set_sensitive(_w.quality_spinbutton, enable_config);
  gtk_widget_set_sensitive(_w.start_on_reset_checkbutton, enable_config);
}

void
VideoWindow::on_record_clicked(GtkWidget *widget, gpointer user_data)
{
  VideoWindow *self = (VideoWindow *) user_data;
  GtkToggleButton *button = GTK_TOGGLE_BUTTON(widget);
  if (button->active)
    {
      self->set_state(VideoEncoder::VIDEO_ENCODER_STATE_RECORD);
    }
}

void
VideoWindow::on_stop_clicked(GtkWidget *widget, gpointer user_data)
{
  VideoWindow *self = (VideoWindow *) user_data;
  self->set_state(VideoEncoder::VIDEO_ENCODER_STATE_STOP);
}

void
VideoWindow::on_pause_clicked(GtkWidget *widget, gpointer user_data)
{
  VideoWindow *self = (VideoWindow *) user_data;
  GtkToggleButton *button = GTK_TOGGLE_BUTTON(widget);
  int state = button->active ? VideoEncoder::VIDEO_ENCODER_STATE_PAUSE : VideoEncoder::VIDEO_ENCODER_STATE_RECORD;
  self->set_state(state);
}

void
VideoWindow::on_config_changed(GtkWidget *widget, gpointer user_data)
{
  VideoWindow *self = (VideoWindow *) user_data;
  gchar *filename = gtk_filebutton_get_filename(GTK_FILEBUTTON(self->_w.filebutton));
  double quality = gtk_spin_button_get_value(GTK_SPIN_BUTTON(self->_w.quality_spinbutton));
  int video_skip = get_active_value_as_int(GTK_COMBO_BOX(self->_w.fps_combobox), 1);
  bool start_on_reset = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->_w.start_on_reset_checkbutton));
  self->_ui->set_video_encoder_config(filename, quality / 100.0, video_skip, start_on_reset);
}

void
VideoWindow::on_encoder_changed(GtkComboBox *combobox, gpointer user_data)
{
  VideoWindow *self = (VideoWindow *) user_data;

  GtkTreeIter iter;
  if (!gtk_combo_box_get_active_iter(combobox, &iter))
    return;

  gpointer encoder;
  gtk_tree_model_get(GTK_TREE_MODEL(gtk_combo_box_get_model(combobox)), &iter, 1, &encoder, -1);
  self->_ui->set_video_encoder((VideoEncoder *) encoder);

  self->set_config_widget_sensitivity(true);

  on_config_changed(GTK_WIDGET(combobox), user_data);
}

gint
VideoWindow::get_active_value_as_int(GtkComboBox *combobox, int column)
{
  GtkTreeIter iter;
  if (!gtk_combo_box_get_active_iter(combobox, &iter))
    return 0;

  guint value;
  gtk_tree_model_get(gtk_combo_box_get_model(combobox), &iter, 1, &value, -1);
  return value;
}

void
VideoWindow::init_fps(GtkComboBox *combobox)
{
  GtkTreeIter iter;
  GtkListStore *store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

  gtk_list_store_append(store, &iter);
  gtk_list_store_set(store, &iter, 0, "50", 1, 1, -1);
  gtk_list_store_append(store, &iter);
  gtk_list_store_set(store, &iter, 0, "25", 1, 2, -1);
  gtk_list_store_append(store, &iter);
  gtk_list_store_set(store, &iter, 0, "16", 1, 3, -1);
  gtk_list_store_append(store, &iter);
  gtk_list_store_set(store, &iter, 0, "12", 1, 4, -1);
  gtk_list_store_append(store, &iter);
  gtk_list_store_set(store, &iter, 0, "10", 1, 5, -1);
  gtk_list_store_append(store, &iter);
  gtk_list_store_set(store, &iter, 0, "5", 1, 10, -1);
  gtk_list_store_append(store, &iter);
  gtk_list_store_set(store, &iter, 0, "2", 1, 25, -1);
  gtk_list_store_append(store, &iter);
  gtk_list_store_set(store, &iter, 0, "1", 1, 50, -1);

  gtk_combo_box_set_model(combobox, GTK_TREE_MODEL(store));
  GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
  gtk_cell_layout_pack_end(GTK_CELL_LAYOUT(combobox), renderer, TRUE);
  gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combobox), renderer, "text", 0, NULL);
  gtk_combo_box_set_active(combobox, 0);
  g_object_unref(store);
}

void
VideoWindow::init_encoder(GtkComboBox *combobox)
{
  GtkTreeIter iter;
  GtkListStore *store = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN);

  gtk_list_store_append(store, &iter);
  gtk_list_store_set(store, &iter, 0, _("<none>"), 1, new DummyVideoEncoder(), 2, FALSE, 3, FALSE, -1);

#ifdef HAVE_LIBTHEORA
  gtk_list_store_append(store, &iter);
  gtk_list_store_set(store, &iter, 0, _("Theora/Ogg"), 1, new TheoraVideoEncoder(), 2, TRUE, 3, TRUE, -1);
#endif
#ifdef HAVE_LIBAVFORMAT
  gtk_list_store_append(store, &iter);
  gtk_list_store_set(store, &iter, 0, _("AVI"), 1, new FfmpegVideoEncoder(), 2, TRUE, 3, TRUE, -1);
#endif
#ifdef HAVE_LIBXVIDCORE
  gtk_list_store_append(store, &iter);
  gtk_list_store_set(store, &iter, 0, _("MPEG 4"), 1, new XvidVideoEncoder(), 2, TRUE, 3, TRUE, -1);
#endif
#ifdef HAVE_LIBDIRAC
  gtk_list_store_append(store, &iter);
  gtk_list_store_set(store, &iter, 0, _("Dirac"), 1, new DiracVideoEncoder(), 2, TRUE, 3, TRUE, -1);
#endif
#ifdef HAVE_LIBSCHROEDINGER
  gtk_list_store_append(store, &iter);
  gtk_list_store_set(store, &iter, 0, _("Schroedinger"), 1, new SchroedingerVideoEncoder(), 2, TRUE, 3, TRUE, -1);
#endif
#ifdef HAVE_LIBGIF
  gtk_list_store_append(store, &iter);
  gtk_list_store_set(store, &iter, 0, _("Animated GIF"), 1, new GifVideoEncoder(), 2, TRUE, 3, TRUE, -1);
#endif
#ifdef HAVE_LIBVNCSERVER
  gtk_list_store_append(store, &iter);
  gtk_list_store_set(store, &iter, 0, _("VNC"), 1, new VncVideoEncoder(), 2, FALSE, 3, TRUE, -1);
#endif

  gtk_combo_box_set_model(combobox, GTK_TREE_MODEL(store));
  GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
  gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combobox), renderer, TRUE);
  gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combobox), renderer, "text", 0, NULL);
  gtk_combo_box_set_active(combobox, 0);
  g_object_unref(store);
}

void
VideoWindow::init(void)
{
  /*
   *  video window
   */
  _window = get_widget("video_window");
  g_signal_connect(_window, "delete_event", G_CALLBACK(cmd_exec_sft), (char *) "ui-video-window-toggle");// FIXME:

  _w.start_on_reset_checkbutton = get_widget("start_on_reset_checkbutton");
  g_signal_connect(_w.start_on_reset_checkbutton, "toggled", G_CALLBACK(on_config_changed), this);

  _w.quality_spinbutton = get_widget("quality_spinbutton");
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(_w.quality_spinbutton), 80);
  g_signal_connect(_w.quality_spinbutton, "value-changed", G_CALLBACK(on_config_changed), this);

  _w.fps_combobox = get_widget("fps_combobox");
  init_fps(GTK_COMBO_BOX(_w.fps_combobox));
  g_signal_connect(_w.fps_combobox, "changed", G_CALLBACK(on_config_changed), this);

  _w.encoder_combobox = get_widget("encoder_combobox");
  init_encoder(GTK_COMBO_BOX(_w.encoder_combobox));
  g_signal_connect(_w.encoder_combobox, "changed", G_CALLBACK(on_encoder_changed), this);

  _w.filebutton = gtk_filebutton_new("Video");
  GtkWidget *c = get_widget("video_file_container");
  gtk_container_add(GTK_CONTAINER(c), _w.filebutton);
  gtk_widget_set_sensitive(_w.filebutton, FALSE);
  gtk_widget_show(_w.filebutton);
  g_signal_connect(_w.filebutton, "changed", G_CALLBACK(on_config_changed), this);

  _w.record_button = get_widget("record_button");
  g_signal_connect(_w.record_button, "clicked", G_CALLBACK(on_record_clicked), this);

  _w.stop_button = get_widget("stop_button");
  g_signal_connect(_w.stop_button, "clicked", G_CALLBACK(on_stop_clicked), this);

  _w.pause_button = get_widget("pause_button");
  g_signal_connect(_w.pause_button, "clicked", G_CALLBACK(on_pause_clicked), this);

  init_dialog("ui-video-window-toggle", "window-video");
}