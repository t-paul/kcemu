/*
 *  KCemu -- The emulator for the KC85 homecomputer series and much more.
 *  Copyright (C) 1997-2010 Torsten Paul
 *
 *  Parts of this file are taken from the gtk demos source code provided
 *  as example for programming with GTK - The GIMP Toolkit.
 *
 *  Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
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

#ifndef __ui_gtk_help_h
#define __ui_gtk_help_h

#include <gtk/gtk.h>

#include "kc/system.h"

#include "ui/gtk/window.h"

class HelpWindow : public UI_Gtk_Window
{
 private:
  struct {
    GtkWidget     *text_view;
    GtkWidget     *button_back;
    GtkWidget     *button_next;

    GtkTextTag    *tag_bold;
    GtkTextTag    *tag_italic;
    GtkTextTag    *tag_underline;
    GtkTextTag    *tag_center;
    GtkTextTag    *tag_monospace;
    GtkTextTag    *tag_size[7];
    GtkTextBuffer *text_buffer;
  } _w;

  GList *_history;
  GList *_history_ptr;

  enum {
    PARSE_FLAGS_DEFAULT    = 0,
    PARSE_FLAGS_BOLD       = (1 << 0),
    PARSE_FLAGS_ITALIC     = (1 << 1),
    PARSE_FLAGS_UNDERLINE  = (1 << 2),
    PARSE_FLAGS_CENTER     = (1 << 3),
    PARSE_FLAGS_MONOSPACE  = (1 << 4),
  };

  typedef enum {
    PARSE_STATE_NONE = 0,
    PARSE_STATE_TEXT = 1,
    PARSE_STATE_LINK = 2,
  } parse_state_t;

  GtkTextIter iter;
  GMarkupParser parser;
  int parse_flags;
  int text_size;
  char *link_name;
  parse_state_t parse_state;

  GdkCursor *_hand_cursor;
  GdkCursor *_regular_cursor;
  gboolean   _hovering_over_link;

  const char *_delayed_help_topic;
  gboolean    _realized;

  CMD *_cmd;

 protected:
  void init(void);
  void init2(void);
  void load_text(void);
  void follow_if_link(GtkWidget *text_view, GtkTextIter *iter);
  void set_cursor_if_appropriate (GtkTextView *text_view, gint x, gint y);

  void insert_ref(const char *name);
  void insert_image(const char *name);
  void insert_link(const char *text, const char *ref);
  void insert_text(const char *text, GtkTextTag *tag);

  void history_next(void);
  void history_back(void);
  
  static gboolean context_help_button_press(GtkWidget *widget, GdkEventButton *bevent, gpointer data);

  static void sf_button_back(GtkButton *button, gpointer data);
  static void sf_button_forward(GtkButton *button, gpointer data);
  static void sf_button_home(GtkButton *button, gpointer data);
  static void sf_button_index(GtkButton *button, gpointer data);
  static void sf_button_refresh(GtkButton *button, gpointer data);

  static gboolean sf_key_press_event(GtkWidget *text_view, GdkEventKey *event, gpointer data);
  static gboolean sf_visibility_notify_event(GtkWidget *text_view, GdkEventVisibility *event, gpointer data);
  static gboolean sf_motion_notify_event(GtkWidget *text_view, GdkEventMotion *event, gpointer data);
  static gboolean sf_event_after(GtkWidget *text_view, GdkEvent *ev, gpointer data);

  /*
   *  parser callback functions
   */
  static void sf_parser_start_element_handler(GMarkupParseContext *context, const gchar *element_name,
					      const gchar **attribute_names, const gchar **attribute_values,
					      gpointer user_data, GError **error);
  static void sf_parser_end_element_handler(GMarkupParseContext *context, const gchar *element_name,
					    gpointer user_data, GError **error);
  static void sf_parser_text_handler(GMarkupParseContext *context, const gchar *text, gsize text_len,
				     gpointer user_data, GError **error);
  static void sf_parser_passthrough_handler(GMarkupParseContext *context, const gchar *passthrough_text, gsize text_len,
					    gpointer user_data, GError **error);
  static void sf_parser_error_handler(GMarkupParseContext *context,GError *error, gpointer user_data);

  static const gchar * get_attribute(const gchar *name, const gchar **attribute_names, const gchar **attribute_values);
  
 public:
  HelpWindow(const char *ui_xml_file);
  virtual ~HelpWindow(void);

  gboolean scroll_to(const char *topic, gboolean update_history);

  static gboolean context_help_idle_start(gpointer widget);
};

#endif /* __ui_gtk_help_h */
