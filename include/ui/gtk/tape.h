/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: tape.h,v 1.11 2001/04/14 15:15:31 tp Exp $
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

#ifndef __ui_gtk_tape_h
#define __ui_gtk_tape_h

#include <gtk/gtk.h>

#include "kc/config.h"
#include "kc/system.h"

#include "ui/tape.h"
#include "ui/gtk/window.h"

class TapeWindow : public UI_Gtk_Window, public TapeInterface
{
 public:
  enum {
    TAPE_REWIND  = 1,
    TAPE_FF      = 2,
    TAPE_RECORD  = 3,
    TAPE_PLAY    = 4,
    TAPE_DETACH  = 5,
    TAPE_ATTACH  = 6,
    TAPE_STOP    = 7,
    TAPE_CHANGED = 8,
  };

 private:
  struct {
    GtkWidget *vbox;
    GtkWidget *combo_hbox;
    GtkWidget *combo_label;
    GtkWidget *combo;
    GtkWidget *sw;
    GtkWidget *clist;
    GtkWidget *hbox;
    GtkWidget *led_power;
    GtkWidget *scale;
    GtkWidget *bbox;
    GtkWidget *b_close;
    GtkWidget *b_detach;
    GtkWidget *b_attach;
    GtkWidget *b_stop;
    GtkWidget *b_record;
    GtkWidget *b_play;
    GtkWidget *m_run;
    GtkWidget *m_load;
    GtkWidget *m_edit;
    GtkWidget *m_delete;
    GtkWidget *m_export;
    GtkWidget *menu;
    GtkObject *scale_adj;
  } _w;
  
  int    _nr_of_files;
  int    _selected;
  int    _play;
  int    _record;
  int    _power;
  double _scale_value;

  GdkColormap *_colormap;
  GdkColor     _col[4];

 protected:
  static void sf_power_expose(TapeWindow *self);
  static void sf_tape_scale_changed(GtkAdjustment *adjustment, gpointer data);
  static void sf_tape_button(GtkWidget *widget, int x);
  static int sf_tape_button_press(GtkWidget *widget, GdkEventButton *event,
                                   gpointer data);
  static void sf_tape_file_select(GtkWidget *widget, gint row, gint column, 
                                  GdkEventButton * bevent, gpointer data);
  static void sf_tape_archive_select(GtkWidget *widget, gpointer data);
    
  void init(void);
  void allocate_colors(void);
  virtual void clear_list(void);

 public:
  TapeWindow(void) { init(); }
  virtual ~TapeWindow(void) {}

  virtual void stop(void);
  virtual void attached(const char *name);
  
  /*
   *  TapeInterface
   */
  virtual void tapeAttach(const char *name);
  virtual void tapeDetach(void);
  virtual void tapePower(bool power);
  virtual void tapeProgress(int val);
  virtual void tapeNext(void);
  virtual void tapeAddFile(const char *name, long load,
                           long start, long size,
			   unsigned char type);
  virtual void tapeRemoveFile(int idx);
  virtual int  tapeGetSelected(void);
  virtual const char * tapeGetName(int idx);
  virtual void tapeAddArchive(const char *name);
};

#endif /* __ui_gtk_tape_h */
