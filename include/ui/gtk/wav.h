/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2006 Torsten Paul
 *
 *  $Id: wav.h,v 1.1 2002/10/31 01:38:07 torsten_paul Exp $
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

#ifndef __ui_gtk_wav_h
#define __ui_gtk_wav_h

#include <gtk/gtk.h>

#include "kc/system.h"

#include "ui/gtk/window.h"

class WavWindow : public UI_Gtk_Window
{
 private:
  enum {
    NUM_POINTS = 8000,
    WINDOW_WIDTH = 400,
    WINDOW_HEIGHT = 400,
  };

  struct {
    GtkWidget   *canvas;
    GtkWidget   *comboboxentry;
    GtkWidget   *open;
    GtkWidget   *close;
    GtkWidget   *play;
    GtkWidget   *stop;
    GtkWidget   *record;
    GdkGC       *gc;
    GtkTooltips *tooltips;
  } _w;

  GdkImage *_image;
  int       _image_y;

  CMD *_cmd_wav_info;
  CMD *_cmd_wav_toggle;

 protected:
  static void sf_expose(GtkWidget *widget, GdkEvent *event, gpointer *data);

  void init(void);
  void expose(void);

 public:
  WavWindow(const char *ui_xml_file);
  virtual ~WavWindow(void);

  void update(int gap);
  void selected(const char *filename);
};

#endif /* __ui_gtk_wav_h */
