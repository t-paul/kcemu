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

#ifndef __ui_gtk_savemem_h
#define __ui_gtk_savemem_h

#include <gtk/gtk.h>

#include "kc/system.h"

#include "ui/gtk/window.h"

class SaveMemoryWindow : public UI_Gtk_Window
{
 private:
  enum {
    NR_OF_LINES = 4096,
  };
            
  struct {
    GtkButton       *button_save;
    GtkButton       *button_refresh;
    GtkToggleButton *toggle_button_jump_to_start;
    GtkToggleButton *toggle_button_jump_to_end;
      
    GtkSpinButton   *spin_button_start_hex;
    GtkSpinButton   *spin_button_start_dec;
    GtkSpinButton   *spin_button_end_hex;
    GtkSpinButton   *spin_button_end_dec;
    GtkSpinButton   *spin_button_length_hex;
    GtkSpinButton   *spin_button_length_dec;
      
    GtkAdjustment   *adjustment_start;
    GtkAdjustment   *adjustment_end;
    GtkAdjustment   *adjustment_length;
      
    GtkTextView     *text_view;
    GtkTextBuffer   *text_buffer;
    GtkTextTag      *text_tag_bold;
    GtkTextTag      *text_tag_italic;
    GtkTextTag      *text_tag_monospace;
    
    gint on_adjustment_start_value_changed_id;
    gint on_adjustment_end_value_changed_id;
    gint on_adjustment_length_value_changed_id;
  } _w;

  bool line_selected[NR_OF_LINES];
  CMD *_cmd_window_toggle;

 protected:
  void init(void);
            
  void show(void);
  void refresh(void);
  void jump_to(int line, gdouble within_margin, gboolean use_align, gdouble yalign);
  void load_memory(int start_addr, int end_addr);
  void apply_selection(int start_addr, int end_addr);
  void remove_selection(int start_line, int end_line);
  void apply_line_selection(int line, int bytes, int line_offset, int line_offset2, int separator_offset, int chars_per_byte, bool add);
  void set_length_adjustment(void);
  void save(const char *filename, int start_addr, int end_addr);
  void set_adjustment_value_with_blocked_handler(GtkAdjustment *adjustment, gdouble value, gint handler_id);

  static void on_save(GtkButton *button, gpointer user_data);
  static void on_refresh(GtkButton *button, gpointer user_data);
  static void on_jump_to_start(GtkToggleButton *button, gpointer user_data);
  static void on_jump_to_end(GtkToggleButton *button, gpointer user_data);
  static void on_populate_popup(GtkTextView *textview, GtkMenu *menu, gpointer user_data);

  static gboolean on_output(GtkSpinButton *spin_button, gpointer user_data);
  static gboolean on_input(GtkSpinButton *spin_button, gdouble *new_value, gpointer user_data);

  static void on_adjustment_start_value_changed(GtkAdjustment *adjustment, gpointer user_data);
  static void on_adjustment_end_value_changed(GtkAdjustment *adjustment, gpointer user_data);
  static void on_adjustment_length_value_changed(GtkAdjustment *adjustment, gpointer user_data);

 public:
  SaveMemoryWindow(const char *ui_xml_file);
  virtual ~SaveMemoryWindow(void);
};

#endif /* __ui_gtk_savemem_h */
