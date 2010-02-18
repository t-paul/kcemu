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

#include "kc/system.h"

#include "cmd/cmd.h"

#include "ui/gtk/cmd.h"
#include "ui/gtk/wav.h"

class CMD_ui_wav_window_toggle : public CMD
{
private:
  WavWindow *_w;

public:
  CMD_ui_wav_window_toggle(WavWindow *w) : CMD("ui-wav-window-toggle")
    {
      _w = w;
      register_cmd("ui-wav-window-toggle");
    }

  void execute(CMD_Args *args, CMD_Context context)
    {
      _w->toggle();
    }
};

class CMD_ui_wav_info : public CMD
{
private:
  WavWindow *_w;

public:
  CMD_ui_wav_info(WavWindow *w) : CMD("ui-wav-info")
    {
      _w = w;
      register_cmd("ui-wav-info", 0);
      register_cmd("ui-wav-file-selected", 1);
      register_cmd("ui-wav-file-closed", 2);
    }

  void execute(CMD_Args *args, CMD_Context context)
    {
      switch (context)
	{
	case 0:
	  if (args == NULL)
	    return;
	  if (_w->is_visible())
	    _w->update(args->get_long_arg("gap"));
	  break;
	case 1:
	  if (args == NULL)
	    return;
	  _w->selected(args->get_string_arg("filename"));
	  break;
	case 2:
	  _w->selected(NULL);
	  break;
	}
    }
};

void
WavWindow::sf_expose(GtkWidget *widget, GdkEvent *event, gpointer *data)
{
  WavWindow *self = (WavWindow *)data;
  self->expose();
}

WavWindow::WavWindow(const char *ui_xml_file) : UI_Gtk_Window(ui_xml_file)
{
  _cmd_wav_info = new CMD_ui_wav_info(this);
  _cmd_wav_toggle = new CMD_ui_wav_window_toggle(this);
}

WavWindow::~WavWindow(void)
{
  delete _cmd_wav_info;
  delete _cmd_wav_toggle;
}

void
WavWindow::init(void)
{
  /*
   *  wav window
   */
  _window = get_widget("audio_window");
  gtk_signal_connect(GTK_OBJECT(_window), "delete_event",
		     GTK_SIGNAL_FUNC(cmd_exec_sft),
		     (char *)"ui-wav-window-toggle"); // FIXME:
  
  _w.tooltips = gtk_tooltips_new();

  /*
   *  canvas, needs GDK_EXPOSURE_MASK to be set!
   */
  _w.canvas = get_widget("main_drawingarea");
  gtk_drawing_area_size(GTK_DRAWING_AREA(_w.canvas), WINDOW_WIDTH, WINDOW_HEIGHT);
  gtk_signal_connect(GTK_OBJECT(_w.canvas), "expose_event",
		     GTK_SIGNAL_FUNC(sf_expose), this);

  /*
   *  comboboxentry
   */
  _w.comboboxentry = get_widget("file_comboboxentry");

  /*
   *  open button
   */
  _w.open = get_widget("file_button_open");
  gtk_signal_connect(GTK_OBJECT(_w.open), "clicked",
		     GTK_SIGNAL_FUNC(cmd_exec_sf),
		     (gpointer)"kc-wav-open");

  gtk_tooltips_set_tip(_w.tooltips, _w.open, _("Open"), NULL);

  /*
   *  close button
   */
  _w.close = get_widget("file_button_close");
  gtk_signal_connect(GTK_OBJECT(_w.close), "clicked",
		     GTK_SIGNAL_FUNC(cmd_exec_sf),
		     (gpointer)"kc-wav-close");

  gtk_tooltips_set_tip(_w.tooltips, _w.close, _("Close"), NULL);

  /*
   *  play button
   */
  _w.play = get_widget("control_togglebutton_play");
  gtk_signal_connect(GTK_OBJECT(_w.play), "clicked",
		     GTK_SIGNAL_FUNC(cmd_exec_sf),
		     (gpointer)"kc-wav-play");

  gtk_tooltips_set_tip(_w.tooltips, _w.play, _("Play"), NULL);

  /*
   *  stop button
   */
  _w.stop = get_widget("control_button_stop");
  gtk_signal_connect(GTK_OBJECT(_w.stop), "clicked",
		     GTK_SIGNAL_FUNC(cmd_exec_sf),
		     (gpointer)"kc-wav-stop");

  gtk_tooltips_set_tip(_w.tooltips, _w.stop, _("Stop"), NULL);

  /*
   *  record button
   */
  _w.record = get_widget("control_togglebutton_record");
  gtk_signal_connect(GTK_OBJECT(_w.record), "clicked",
		     GTK_SIGNAL_FUNC(cmd_exec_sf),
		     (gpointer)"kc-wav-record");

  gtk_tooltips_set_tip(_w.tooltips, _w.record, _("Record"), NULL);

  init_dialog("ui-wav-window-toggle", "window-audio");

  _w.gc = NULL;
  _image_y = 0;
  _image  = gdk_image_new(GDK_IMAGE_FASTEST,
			  gdk_visual_get_system(),
			  WINDOW_WIDTH,
			  2 * WINDOW_HEIGHT);
}

void
WavWindow::selected(const char *filename)
{
  gboolean sensitive = TRUE;

  if (filename == NULL)
    {
      filename = "";
      sensitive = FALSE;
    }

  gtk_widget_set_sensitive(_w.close, sensitive);
  gtk_widget_set_sensitive(_w.play, sensitive);
  gtk_widget_set_sensitive(_w.stop, sensitive);

  gtk_widget_set_sensitive(_w.record, FALSE);

  GtkEntry *entry = GTK_ENTRY(GTK_BIN(_w.comboboxentry)->child);
  gtk_entry_set_text(GTK_ENTRY(entry), filename);
}

void
WavWindow::expose(void)
{
  if (_w.gc == NULL)
    {
      _w.gc = gdk_gc_new(GTK_WIDGET(_w.canvas)->window);

      /*
       * clear image not in initialization to decrease startup time
       * instead we do it when receiving the first expose event
       */
      for (int y = 0;y < 2 * WINDOW_HEIGHT;y++)
	for (int x = 0;x < WINDOW_WIDTH;x++)
	  gdk_image_put_pixel(_image, x, y, 0xffffffff);
    }

  gdk_draw_image(GTK_WIDGET(_w.canvas)->window,
		 _w.gc, _image,
		 0, _image_y,
		 0, 0,
		 WINDOW_WIDTH, WINDOW_HEIGHT);
}

void
WavWindow::update(int gap)
{
  int a, x;
  static int cnt = 80;

  if (_w.gc == NULL)
    return;

  x = gap / 5;
  if (x >= WINDOW_WIDTH)
    return;

  gdk_image_put_pixel(_image, x, _image_y, 0);
  gdk_image_put_pixel(_image, x, _image_y + WINDOW_HEIGHT, 0);

  if (--cnt >= 0)
    return;

  cnt = 80;

  _image_y++;

  for (a = 0;a < WINDOW_WIDTH;a++)
    {
      gdk_image_put_pixel(_image, a, _image_y, 0xffffffff);
      gdk_image_put_pixel(_image, a, _image_y + WINDOW_HEIGHT, 0xffffffff);
    }
  gdk_image_put_pixel(_image,  550 / 5, _image_y, 0xff0000);
  gdk_image_put_pixel(_image,  550 / 5, _image_y + WINDOW_HEIGHT, 0xff0000);
  gdk_image_put_pixel(_image, 1000 / 5, _image_y, 0x00ff00);
  gdk_image_put_pixel(_image, 1000 / 5, _image_y + WINDOW_HEIGHT, 0x00ff00);

  expose();

  if (_image_y >= WINDOW_HEIGHT)
    _image_y = 0;
}
