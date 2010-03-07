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

#ifndef __ui_gtk_tape_h
#define __ui_gtk_tape_h

#include <gtk/gtk.h>

#include "kc/system.h"

#include "ui/tape.h"
#include "ui/gtk/window.h"

class TapeWindow : public UI_Gtk_Window, public TapeInterface
{
 private:
  enum {
    TREEVIEW_COLUMN_NAME,
    TREEVIEW_COLUMN_TYPE,
    TREEVIEW_COLUMN_LOAD,
    TREEVIEW_COLUMN_START,
    TREEVIEW_COLUMN_SIZE,
    TREEVIEW_N_COLUMNS
  };

  struct {
    GtkWidget *treeview;
    GtkWidget *b_play;
    GtkWidget *b_stop;
    GtkWidget *b_record;
    GtkWidget *b_next;
    GtkWidget *b_prev;
    GtkWidget *b_attach;
    GtkWidget *b_detach;

    GtkListStore *list_store;
    GtkAdjustment *scale_adj;

    GtkWidget *b_close;
    GtkWidget *vbox;
    GtkWidget *combo_hbox;
    GtkWidget *combo_label;
    GtkWidget *combo;
    GtkWidget *sw;
    GtkWidget *hbox;
    GtkWidget *led_power;
    GtkWidget *scale;
    GtkWidget *bbox;
    GtkWidget *m_run;
    GtkWidget *m_load;
    GtkWidget *m_edit;
    GtkWidget *m_delete;
    GtkWidget *m_rename;
    GtkWidget *m_export;
    GtkWidget *m_wav;
    GtkWidget *menu;
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
  static void sf_tape_file_select(GtkTreeSelection *selection, gpointer data);
  static void sf_tape_archive_select(GtkWidget *widget, gpointer data);
    
  void init(void);
  void allocate_colors(void);
  void set_selected_index(GtkTreeSelection *selection);
  virtual void clear_list(void);

 public:
  TapeWindow(const char *ui_xml_file);
  virtual ~TapeWindow(void);

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
