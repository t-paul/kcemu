/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2002 Torsten Paul
 *
 *  $Id: wav.cc,v 1.1 2002/10/31 01:38:12 torsten_paul Exp $
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

#include "cmd/cmd.h"

#include "ui/gtk/cmd.h"
#include "ui/gtk/wav.h"

#include "ui/gtk/play.xpm"
#include "ui/gtk/stop.xpm"
#include "ui/gtk/open.xpm"
//#include "ui/gtk/pause.xpm"

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
      register_cmd("ui-wav-open");
      register_cmd("ui-wav-info", 2);
      register_cmd("ui-wav-file-selected", 3);
    }

  void execute(CMD_Args *args, CMD_Context context)
    {
      switch (context)
	{
	case 0:
	  CMD_EXEC("kc-wav-open");
	  break;
	case 2:
	  if (args == NULL)
	    return;
	  if (_w->is_visible())
	    _w->update(args->get_int_arg("gap"));
	  break;
	case 3:
	  if (args == NULL)
	    return;
	  _w->selected(args->get_string_arg("filename"));
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

WavWindow::WavWindow(void)
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
  _window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name(_window, "WavWindow");
  gtk_window_set_title(GTK_WINDOW(_window), _("KCemu: Audio Player"));
  gtk_window_position(GTK_WINDOW(_window), GTK_WIN_POS_MOUSE);
  gtk_signal_connect(GTK_OBJECT(_window), "delete_event",
		     GTK_SIGNAL_FUNC(cmd_exec_sft),
		     (char *)"ui-wav-window-toggle"); // FIXME:

  _w.tooltips = gtk_tooltips_new();

  /*
   *  vbox
   */
  _w.vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(_window), _w.vbox);
  gtk_container_set_border_width(GTK_CONTAINER(_window), 4);
  gtk_widget_show(_w.vbox);

  /*
   *  frame
   */
  _w.frame = gtk_frame_new(NULL);
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.frame, TRUE, TRUE, 0);
  gtk_widget_show(_w.frame);

  /*
   *  canvas
   */
  _w.canvas = gtk_drawing_area_new();
  gtk_drawing_area_size(GTK_DRAWING_AREA(_w.canvas), WINDOW_WIDTH, WINDOW_HEIGHT);
  gtk_container_add(GTK_CONTAINER(_w.frame), _w.canvas);
  gtk_widget_show(_w.canvas);
  gtk_widget_set_events(_w.canvas, GDK_EXPOSURE_MASK);
  gtk_signal_connect(GTK_OBJECT(_w.canvas), "expose_event",
		     GTK_SIGNAL_FUNC(sf_expose), this);

  /*
   *  realize window _after_ gtk_widget_set_events()!
   */
  gtk_widget_realize(_window);

  /*
   *  separator
   */
  _w.separator1 = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.separator1, FALSE, FALSE, 5);
  gtk_widget_show(_w.separator1);

  /*
   *  hbox
   */
  _w.hbox2 = gtk_hbox_new(FALSE, 4);
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.hbox2, FALSE, FALSE, 5);
  gtk_widget_show(_w.hbox2);

  /*
   *  entry
   */
  _w.entry = gtk_entry_new();
  gtk_box_pack_start(GTK_BOX(_w.hbox2), _w.entry, TRUE, TRUE, 2);
  gtk_entry_set_editable(GTK_ENTRY(_w.entry), FALSE);
  gtk_widget_show(_w.entry);

  /*
   *  hbox
   */
  /*
  _w.hbox1 = gtk_hbox_new(FALSE, 4);
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.hbox1, FALSE, FALSE, 5);
  gtk_widget_show(_w.hbox1);
  */

  /*
   *  open button
   */
  //_w.open = gtk_button_new_with_label(_("Open"));
  _w.open = create_button_with_pixmap(_window, __xpm_open);
  gtk_signal_connect(GTK_OBJECT(_w.open), "clicked",
		     GTK_SIGNAL_FUNC(cmd_exec_sf),
		     (gpointer)"ui-wav-open");
  gtk_box_pack_start(GTK_BOX(_w.hbox2), _w.open, FALSE, FALSE, 2);
  gtk_widget_show(_w.open);

  gtk_tooltips_set_tip(_w.tooltips, _w.open, _("Open"), NULL);

  /*
   *  stop button
   */
  //_w.stop = gtk_button_new_with_label(_("Stop"));
  _w.stop = create_button_with_pixmap(_window, __xpm_stop);
  gtk_signal_connect(GTK_OBJECT(_w.stop), "clicked",
		     GTK_SIGNAL_FUNC(cmd_exec_sf),
		     (gpointer)"kc-wav-stop");
  gtk_box_pack_start(GTK_BOX(_w.hbox2), _w.stop, FALSE, FALSE, 2);
  gtk_widget_show(_w.stop);

  gtk_tooltips_set_tip(_w.tooltips, _w.stop, _("Stop"), NULL);

  /*
   *  pause button
   */
  //_w.pause = gtk_button_new_with_label(_("Pause"));
  /*
  _w.pause = create_button_with_pixmap(_window, __xpm_pause);
  gtk_signal_connect(GTK_OBJECT(_w.pause), "clicked",
                     GTK_SIGNAL_FUNC(cmd_exec_sf),
                     (gpointer)"kc-wav-pause");
  gtk_box_pack_start(GTK_BOX(_w.hbox2), _w.pause, FALSE, FALSE, 2);
  gtk_widget_show(_w.pause);

  gtk_tooltips_set_tip(_w.tooltips, _w.pause, _("Pause"), NULL);
  */

  /*
   *  play button
   */
  //_w.play = gtk_button_new_with_label(_("Play"));
  _w.play = create_button_with_pixmap(_window, __xpm_play);
  gtk_signal_connect(GTK_OBJECT(_w.play), "clicked",
		     GTK_SIGNAL_FUNC(cmd_exec_sf),
		     (gpointer)"kc-wav-play");
  gtk_box_pack_start(GTK_BOX(_w.hbox2), _w.play, FALSE, FALSE, 2);
  gtk_widget_show(_w.play);

  gtk_tooltips_set_tip(_w.tooltips, _w.play, _("Play"), NULL);

  /*
   *  separator
   */
  _w.separator2 = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.separator2,
		     FALSE, FALSE, 5);
  gtk_widget_show(_w.separator2);

  /*
   *  close button
   */
  _w.close = gtk_button_new_with_label(_("Close"));
  gtk_signal_connect(GTK_OBJECT(_w.close), "clicked",
		     GTK_SIGNAL_FUNC(cmd_exec_sf),
		     (gpointer)"ui-wav-window-toggle");
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.close, FALSE, FALSE, 5);
  GTK_WIDGET_SET_FLAGS(_w.close, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(_w.close);
  gtk_widget_show(_w.close);

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
  if (filename == NULL)
    filename = "";

  if (_w.entry != NULL)
    gtk_entry_set_text(GTK_ENTRY(_w.entry), filename);
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
