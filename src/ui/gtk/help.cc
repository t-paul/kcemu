/*
 *  KCemu -- The emulator for the KC85 homecomputer series and much more.
 *  Copyright (C) 1997-2010 Torsten Paul
 *
 *  Handling of hyperlinks in GtkTextView taken from the gtk demos source
 *  code provided as example for programming with GTK - The GIMP Toolkit.
 *
 *  Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 *  Context help handling taken from "The GIMP"
 *
 *  Copyright (C) ???
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

#include <stdlib.h>

#include <string>

#include <gdk/gdkkeysyms.h>

#include "kc/system.h"

#include "kc/kc.h"

#include "ui/gtk/cmd.h"
#include "ui/gtk/help.h"

#include "libdbg/dbg.h"

using namespace std;

class CMD_help_window_toggle : public CMD
{
private:
  HelpWindow *_w;

public:
  CMD_help_window_toggle(HelpWindow *w) : CMD("ui-help-window-toggle")
    {
      _w = w;
      register_cmd("ui-help-window-toggle");
      register_cmd("ui-help-window-toggle-home", 1);
      register_cmd("ui-help-window-toggle-index", 2);
      register_cmd("ui-help-window-show", 3);
      register_cmd("ui-help-window-context-help", 4);
    }

  void execute(CMD_Args *args, CMD_Context context)
    {
      const char *topic = NULL;

      switch (context)
	{
	case 0:
	  _w->toggle();
	  break;
	case 1:
	  _w->toggle();
	  _w->scroll_to("home", TRUE);
	  break;
	case 2:
	  _w->toggle();
	  _w->scroll_to("index", TRUE);
	  break;
	case 3:
	  _w->show();
	  if (args != NULL)
	    topic = args->get_string_arg("help-topic");
	  _w->scroll_to(topic, TRUE);
	  break;
	case 4:
	  g_idle_add(_w->context_help_idle_start, _w);
	  break;
	}
    }
};

void
HelpWindow::sf_button_back(GtkButton *button, gpointer data)
{
  HelpWindow *w = (HelpWindow *)data;
  w->history_back();
}

void
HelpWindow::sf_button_forward(GtkButton *button, gpointer data)
{
  HelpWindow *w = (HelpWindow *)data;
  w->history_next();
}

void
HelpWindow::sf_button_home(GtkButton *button, gpointer data)
{
  HelpWindow *w = (HelpWindow *)data;
  w->scroll_to("home", TRUE);
}

void
HelpWindow::sf_button_index(GtkButton *button, gpointer data)
{
  HelpWindow *w = (HelpWindow *)data;
  w->scroll_to("index", TRUE);
}

void
HelpWindow::sf_button_refresh(GtkButton *button, gpointer data)
{
  HelpWindow *w = (HelpWindow *)data;
  w->load_text();
}

/*
 *  Links can be activated by pressing Enter.
 */
gboolean
HelpWindow::sf_key_press_event(GtkWidget *text_view, GdkEventKey *event, gpointer data)
{
  GtkTextIter iter;
  GtkTextBuffer *buffer;
  HelpWindow *w = (HelpWindow *)data;
  
  switch (event->keyval)
    {
    case GDK_Return: 
    case GDK_KP_Enter:
      buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
      gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
      w->follow_if_link(text_view, &iter);
      break;
      
    default:
      break;
    }
  
  return FALSE;
}

/*
 *  Also update the cursor image if the window becomes visible
 *  (e.g. when a window covering it got iconified).
 */
gboolean
HelpWindow::sf_visibility_notify_event(GtkWidget *text_view, GdkEventVisibility *event, gpointer data)
{
  gint wx, wy, bx, by;
  HelpWindow *w = (HelpWindow *)data;
  
  gdk_window_get_pointer (text_view->window, &wx, &wy, NULL);
  
  gtk_text_view_window_to_buffer_coords(GTK_TEXT_VIEW(text_view), 
					GTK_TEXT_WINDOW_WIDGET,
					wx, wy, &bx, &by);

  w->set_cursor_if_appropriate(GTK_TEXT_VIEW(text_view), bx, by);

  return FALSE;
}

/*
 *  Update the cursor image if the pointer moved. 
 */
gboolean
HelpWindow::sf_motion_notify_event(GtkWidget *text_view, GdkEventMotion *event, gpointer data)
{
  gint x, y;
  HelpWindow *w = (HelpWindow *)data;

  gtk_text_view_window_to_buffer_coords(GTK_TEXT_VIEW(text_view), 
					GTK_TEXT_WINDOW_WIDGET,
					(gint)event->x, (gint)event->y, &x, &y);

  w->set_cursor_if_appropriate(GTK_TEXT_VIEW(text_view), x, y);

  gdk_window_get_pointer(text_view->window, NULL, NULL, NULL);

  return FALSE;
}

/*
 *  Links can also be activated by clicking.
 */
gboolean
HelpWindow::sf_event_after(GtkWidget *text_view, GdkEvent *ev, gpointer data)
{
  gint x, y;
  GtkTextBuffer *buffer;
  GdkEventButton *event;
  GtkTextIter start, end, iter;
  HelpWindow *w = (HelpWindow *)data;

  if (ev->type != GDK_BUTTON_RELEASE)
    return FALSE;

  event = (GdkEventButton *)ev;

  if (event->button != 1)
    return FALSE;

  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW (text_view));

  /* we shouldn't follow a link if the user has selected something */
  gtk_text_buffer_get_selection_bounds(buffer, &start, &end);
  if (gtk_text_iter_get_offset(&start) != gtk_text_iter_get_offset(&end))
    return FALSE;

  gtk_text_view_window_to_buffer_coords(GTK_TEXT_VIEW(text_view), 
					GTK_TEXT_WINDOW_WIDGET,
					(gint)event->x, (gint)event->y, &x, &y);

  gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(text_view), &iter, x, y);

  w->follow_if_link(text_view, &iter);

  return FALSE;
}

const gchar *
HelpWindow::get_attribute(const gchar *name, const gchar **attribute_names, const gchar **attribute_values)
{
  for (int i = 0;attribute_names[i] != NULL;i++)
    {
      if (strcmp(attribute_names[i], name) == 0)
	{
	  return attribute_values[i];
	}
    }

  return NULL;
}

void
HelpWindow::sf_parser_start_element_handler(GMarkupParseContext *context, const gchar *element_name,
					    const gchar **attribute_names, const gchar **attribute_values,
					    gpointer user_data, GError **error)
{
  HelpWindow *w = (HelpWindow *)user_data;

  if (strcmp(element_name, "ref") == 0)
    {
      const gchar *val = get_attribute("name", attribute_names, attribute_values);
      if (val != NULL)
	{
	  w->insert_ref(val);
	}
    }
  else if (strcmp(element_name, "size") == 0)
    {
      const gchar *val = get_attribute("value", attribute_names, attribute_values);
      if (val != NULL)
	{
	  int value = atoi(val);
	  if (value < -3)
	    value = -3;
	  if (value > 3)
	    value = 3;
	  w->text_size = value;
	}
    }
  else if (strcmp(element_name, "link") == 0)
    {
      const gchar *val = get_attribute("ref", attribute_names, attribute_values);
      if (val != NULL)
	{
	  w->link_name = strdup(val);
	}
      w->parse_state = PARSE_STATE_LINK;
    }
  else if (strcmp(element_name, "img") == 0)
    {
      const gchar *val = get_attribute("src", attribute_names, attribute_values);
      if (val != NULL)
	{
	  w->insert_image(val);
	}
    }
  else if (strcmp(element_name, "b") == 0)
    {
      w->parse_flags |= PARSE_FLAGS_BOLD;
    }
  else if (strcmp(element_name, "i") == 0)
    {
      w->parse_flags |= PARSE_FLAGS_ITALIC;
    }
  else if (strcmp(element_name, "u") == 0)
    {
      w->parse_flags |= PARSE_FLAGS_UNDERLINE;
    }
  else if (strcmp(element_name, "c") == 0)
    {
      w->parse_flags |= PARSE_FLAGS_CENTER;
    }
  else if (strcmp(element_name, "m") == 0)
    {
      w->parse_flags |= PARSE_FLAGS_MONOSPACE;
    }
  else if (strcmp(element_name, "p") == 0)
    {
      w->parse_state = PARSE_STATE_TEXT;
    }
}

void
HelpWindow::sf_parser_end_element_handler(GMarkupParseContext *context, const gchar *element_name,
       			    gpointer user_data, GError **error)
{
  HelpWindow *w = (HelpWindow *)user_data;
  if (strcmp(element_name, "size") == 0)
    {
      w->text_size = 0;
    }
  if (strcmp(element_name, "b") == 0)
    {
      w->parse_flags &= ~PARSE_FLAGS_BOLD;
    }
  if (strcmp(element_name, "i") == 0)
    {
      w->parse_flags &= ~PARSE_FLAGS_ITALIC;
    }
  if (strcmp(element_name, "u") == 0)
    {
      w->parse_flags &= ~PARSE_FLAGS_UNDERLINE;
    }
  if (strcmp(element_name, "c") == 0)
    {
      w->parse_flags &= ~PARSE_FLAGS_CENTER;
    }
  if (strcmp(element_name, "m") == 0)
    {
      w->parse_flags &= ~PARSE_FLAGS_MONOSPACE;
    }
  if (strcmp(element_name, "p") == 0)
    {
      w->parse_state = PARSE_STATE_NONE;
    }
  else if (strcmp(element_name, "link") == 0)
    {
      w->parse_state = PARSE_STATE_TEXT;
    }
}

void
HelpWindow::sf_parser_text_handler(GMarkupParseContext *context, const gchar *text, gsize text_len,
       		     gpointer user_data, GError **error)
{
  HelpWindow *w = (HelpWindow *)user_data;

  switch (w->parse_state)
    {
    case PARSE_STATE_LINK:
      w->insert_link(text, w->link_name);
      free(w->link_name);
      w->link_name = NULL;
      break;
    case PARSE_STATE_TEXT:
      w->insert_text(text, NULL);
      break;
    default:
      break;
    }
}

void
HelpWindow::sf_parser_passthrough_handler(GMarkupParseContext *context, const gchar *passthrough_text, gsize text_len,
       			    gpointer user_data, GError **error)
{
  DBG(1, form("KCemu/Help/parser", "passthrough text (%ld chars): '%.*s'\n", text_len, text_len, passthrough_text));
}

void
HelpWindow::sf_parser_error_handler(GMarkupParseContext *context, GError *error, gpointer user_data)
{
  DBG(1, form("KCemu/Help/parser", "error: %s\n", error->message));
}

HelpWindow::HelpWindow(const char *ui_xml_file) : UI_Gtk_Window(ui_xml_file)
{
  _history = NULL;
  _history_ptr = NULL;

  _hovering_over_link = FALSE;
  _hand_cursor = gdk_cursor_new(GDK_HAND2);
  _regular_cursor = gdk_cursor_new(GDK_XTERM);

  parser.start_element = sf_parser_start_element_handler;
  parser.end_element = sf_parser_end_element_handler;
  parser.text = sf_parser_text_handler;
  parser.passthrough = sf_parser_passthrough_handler;
  parser.error = sf_parser_error_handler;

  _cmd = new CMD_help_window_toggle(this);

  init2();
}

HelpWindow::~HelpWindow(void)
{
  delete _cmd;
}

void
HelpWindow::init(void)
{
}

void
HelpWindow::init2(void)
{
  _window = get_widget("help_window");
  g_signal_connect(G_OBJECT(_window), "delete_event",
		   G_CALLBACK(cmd_exec_sft),
		   (char *)"ui-help-window-toggle"); // FIXME:

  /*
   *  textview / textbuffer
   */
  _w.text_view = get_widget("main_textview");
  _w.text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(_w.text_view));
  
  _w.tag_bold = gtk_text_buffer_create_tag(_w.text_buffer, "bold", "weight", PANGO_WEIGHT_BOLD, NULL);
  _w.tag_italic = gtk_text_buffer_create_tag(_w.text_buffer, "italic", "style", PANGO_STYLE_ITALIC, NULL);
  _w.tag_underline = gtk_text_buffer_create_tag(_w.text_buffer, "underline", "underline", PANGO_UNDERLINE_DOUBLE, NULL);
  _w.tag_center = gtk_text_buffer_create_tag(_w.text_buffer, "center", "justification", GTK_JUSTIFY_CENTER, NULL);
  _w.tag_monospace = gtk_text_buffer_create_tag(_w.text_buffer, "monospace", "family", "Monospace", NULL);
  
  _w.tag_size[0] = gtk_text_buffer_create_tag(_w.text_buffer, "size -3", "scale", PANGO_SCALE_XX_SMALL, NULL);
  _w.tag_size[1] = gtk_text_buffer_create_tag(_w.text_buffer, "size -2", "scale", PANGO_SCALE_X_SMALL, NULL);
  _w.tag_size[2] = gtk_text_buffer_create_tag(_w.text_buffer, "size -1", "scale", PANGO_SCALE_SMALL, NULL);
  _w.tag_size[3] = NULL;
  _w.tag_size[4] = gtk_text_buffer_create_tag(_w.text_buffer, "size +1", "scale", PANGO_SCALE_LARGE, NULL);
  _w.tag_size[5] = gtk_text_buffer_create_tag(_w.text_buffer, "size +2", "scale", PANGO_SCALE_X_LARGE, NULL);
  _w.tag_size[6] = gtk_text_buffer_create_tag(_w.text_buffer, "size +3", "scale", PANGO_SCALE_XX_LARGE, NULL);
  
  g_signal_connect(_w.text_view, "key-press-event", G_CALLBACK(sf_key_press_event), this);
  g_signal_connect(_w.text_view, "event-after", G_CALLBACK(sf_event_after), this);
  g_signal_connect(_w.text_view, "motion-notify-event", G_CALLBACK(sf_motion_notify_event), this);
  g_signal_connect(_w.text_view, "visibility-notify-event", G_CALLBACK(sf_visibility_notify_event), this);

  /*
   *  Buttons
   */ 
  _w.button_back = get_widget("navigation_button_back");
  _w.button_next = get_widget("navigation_button_forward");

  g_signal_connect(_w.button_back, "clicked", G_CALLBACK(sf_button_back), this);
  g_signal_connect(_w.button_next, "clicked", G_CALLBACK(sf_button_forward), this);
  g_signal_connect(get_widget("navigation_button_home"), "clicked", G_CALLBACK(sf_button_home), this);
  g_signal_connect(get_widget("navigation_button_index"), "clicked", G_CALLBACK(sf_button_index), this);
  g_signal_connect(get_widget("navigation_button_refresh"), "clicked", G_CALLBACK(sf_button_refresh), this);

  init_dialog("ui-help-window-toggle", NULL);
  
  /*
   *  force initialization directly on gui creation, otherwise the first
   *  call to show the context help will go wrong as it does not wait
   *  for the expose of the window :-(
   */
  
  load_text();

  gtk_widget_realize(_window);
}

void
HelpWindow::load_text(void)
{
  GtkTextBuffer *buf = _w.text_buffer;

  string datadir(kcemu_datadir);
  string help_doc = datadir + "/doc/kcemu-help.xml";

  gchar *text;
  gsize length;
  GError *error = NULL;
  if (!g_file_get_contents(help_doc.c_str(), &text, &length, &error))
    {
      fprintf (stderr, "%s\n", error->message);
      g_error_free (error);
      return;
    }

  text_size = 0;
  parse_flags = 0;
  link_name = NULL;
  parse_state = PARSE_STATE_NONE;
  gtk_text_buffer_set_text(buf, "", 0);
  gtk_text_buffer_get_start_iter(buf, &iter);
  gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(_w.text_view), &iter, 0.0, TRUE, 0.0, 0.0);

  GMarkupParseContext *context = g_markup_parse_context_new(&parser, (GMarkupParseFlags)0, this, NULL);

  if (!g_markup_parse_context_parse(context, text, length, NULL))
    {
      g_markup_parse_context_free(context);
      return;
    }

  if (!g_markup_parse_context_end_parse(context, NULL))
    {
      g_markup_parse_context_free(context);
      return;
    }

  g_markup_parse_context_free(context);
}

void
HelpWindow::history_next(void)
{
  if (_history_ptr != g_list_first(_history))
    {
      _history_ptr = g_list_previous(_history_ptr);

      if (scroll_to((const char *)_history_ptr->data, FALSE))
	{
	  gtk_widget_set_sensitive(_w.button_back, TRUE);
	}
    }
  gtk_widget_set_sensitive(_w.button_next, _history_ptr != g_list_first(_history));
}

void
HelpWindow::history_back(void)
{
  if (_history_ptr != g_list_last(_history))
    {
      _history_ptr = g_list_next(_history_ptr);

      if (scroll_to((const char *)_history_ptr->data, FALSE))
	{
	  gtk_widget_set_sensitive(_w.button_next, TRUE);
	}
    }
  gtk_widget_set_sensitive(_w.button_back, _history_ptr != g_list_last(_history));
}

void
HelpWindow::insert_ref(const char *name)
{
  gtk_text_buffer_create_mark(_w.text_buffer, name, &iter, TRUE);
}

void
HelpWindow::insert_text(const char *text, GtkTextTag *tag)
{
  GtkTextTag *tags[7];

  tags[0] = tags[1] = tags[2] = tags[3] = tags[4] = tags[5] = tags[6] = NULL;

  int i = 0;

  if (parse_flags & PARSE_FLAGS_BOLD)
    tags[i++] = _w.tag_bold;
  if (parse_flags & PARSE_FLAGS_ITALIC)
    tags[i++] = _w.tag_italic;
  if (parse_flags & PARSE_FLAGS_UNDERLINE)
    tags[i++] = _w.tag_underline;
  if (parse_flags & PARSE_FLAGS_CENTER)
    tags[i++] = _w.tag_center;
  if (parse_flags & PARSE_FLAGS_MONOSPACE)
    tags[i++] = _w.tag_monospace;
  if (_w.tag_size[text_size + 3] != NULL)
    tags[i++] = _w.tag_size[text_size + 3];
  if (tag != NULL)
    tags[i++] = tag;

  gtk_text_buffer_insert_with_tags(_w.text_buffer, &iter, text, -1, tags[0], tags[1], tags[2], tags[3], tags[4], tags[5], tags[6], NULL);
}

void
HelpWindow::insert_image(const char *name)
{
  string datadir(kcemu_datadir);
  string docdir = datadir + "/doc/";
  string filename = docdir + name;

  GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(filename.c_str(), NULL);
  if (pixbuf == NULL)
    return;

  gtk_text_buffer_insert_pixbuf(_w.text_buffer, &iter, pixbuf);
}

void
HelpWindow::insert_link(const char *text, const char *ref)
{
  GtkTextTag *tag = gtk_text_buffer_create_tag(_w.text_buffer, NULL,
					       "foreground", "blue",
					       "underline", PANGO_UNDERLINE_SINGLE,
					       NULL);
  g_object_set_data(G_OBJECT(tag), "ref", (gpointer)strdup(ref));
  insert_text(text, tag);
}

gboolean
HelpWindow::scroll_to(const char *topic, gboolean update_history)
{
  GtkTextMark *mark = gtk_text_buffer_get_mark(_w.text_buffer, topic);
  if (mark == NULL)
    {
      const char *fallback_topic = "home";
      DBG(1, form("KCemu/Help/help", "help-topic not found: '%s' (reverting to topic '%s')\n", topic, fallback_topic));
      mark = gtk_text_buffer_get_mark(_w.text_buffer, fallback_topic);
      if (mark == NULL)
	{
	  DBG(1, form("KCemu/Help/help", "help-topic not found: '%s'\n", fallback_topic));
	  return FALSE;
	}
    }

  if (update_history)
    {
      while (_history_ptr != _history)
	{
	  g_free(_history->data);
	  _history = g_list_delete_link(_history, _history);
	}
      
      if ((_history == NULL) || (strcmp((const char *)_history->data, topic) != 0))
	_history = g_list_prepend(_history, g_strdup(topic));

      _history_ptr = _history;

      gtk_widget_set_sensitive(_w.button_back, _history_ptr != g_list_last(_history));
      gtk_widget_set_sensitive(_w.button_next, FALSE);
    }

  DBG(1, form("KCemu/Help/history", "history: ---top-\n"));
  for (GList *ptr = _history;ptr != NULL;ptr = g_list_next(ptr))
    {
      DBG(1, form("KCemu/Help/history", "history: %s%s\n", ptr == _history_ptr ? "->" : "  ", ptr->data));
    }

  gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(_w.text_view), mark, 0, true, 0.0, 0.0);
  return TRUE;
}

/*
 *  Looks at all tags covering the position of iter in the text view, 
 *  and if one of them is a link, follow it by showing the page identified
 *  by the data attached to it.
 */
void
HelpWindow::follow_if_link(GtkWidget *text_view, GtkTextIter *iter)
{
  GSList *tags = NULL, *tagp = NULL;

  tags = gtk_text_iter_get_tags (iter);
  for (tagp = tags;tagp != NULL;tagp = tagp->next)
    {
      GtkTextTag *tag = (GtkTextTag *)tagp->data;
      const char *ref = (const char *)g_object_get_data(G_OBJECT(tag), "ref");
      
      if (ref != NULL)
	{
	  DBG(1, form("KCemu/Help/help", "follow link to ref: '%s'\n", ref));
	  scroll_to(ref, TRUE);
	}
    }
  
  if (tags) 
    g_slist_free (tags);
}

/*
 *  Looks at all tags covering the position (x, y) in the text view, 
 *  and if one of them is a link, change the cursor to the "hands" cursor
 *  typically used by web browsers.
 */
void
HelpWindow::set_cursor_if_appropriate(GtkTextView *text_view, gint x, gint y)
{
  GSList *tags = NULL, *tagp = NULL;
  GtkTextBuffer *buffer;
  GtkTextIter iter;
  gboolean hovering = FALSE;

  buffer = gtk_text_view_get_buffer(text_view);

  gtk_text_view_get_iter_at_location(text_view, &iter, x, y);
  
  tags = gtk_text_iter_get_tags(&iter);
  for (tagp = tags;tagp != NULL;tagp = tagp->next)
    {
      GtkTextTag *tag = (GtkTextTag *)tagp->data;
      const char *ref = (const char *)g_object_get_data(G_OBJECT(tag), "ref");

      if (ref != NULL)
        {
          hovering = TRUE;
          break;
        }
    }

  if (hovering != _hovering_over_link)
    {
      _hovering_over_link = hovering;

      if (_hovering_over_link)
        gdk_window_set_cursor(gtk_text_view_get_window(text_view, GTK_TEXT_WINDOW_TEXT), _hand_cursor);
      else
        gdk_window_set_cursor(gtk_text_view_get_window(text_view, GTK_TEXT_WINDOW_TEXT), _regular_cursor);
    }

  if (tags) 
    g_slist_free (tags);
}

gboolean
HelpWindow::context_help_idle_start(gpointer data)
{
  UI_Gtk_Window *w = (UI_Gtk_Window *)data;

  if (!gtk_grab_get_current ())
    {
      GtkWidget *invisible = gtk_invisible_new_for_screen(gtk_widget_get_screen(w->get_window()));
      gtk_widget_show(invisible);

      GdkCursor *cursor = gdk_cursor_new_for_display(gtk_widget_get_display(invisible), GDK_QUESTION_ARROW);

      GdkGrabStatus status = gdk_pointer_grab(invisible->window, TRUE,
					      (GdkEventMask)(GDK_BUTTON_PRESS_MASK   |
							     GDK_BUTTON_RELEASE_MASK |
							     GDK_ENTER_NOTIFY_MASK   |
							     GDK_LEAVE_NOTIFY_MASK),
					      NULL, cursor,
					      GDK_CURRENT_TIME);
      
      gdk_cursor_unref(cursor);
      
      if (status != GDK_GRAB_SUCCESS)
        {
          gtk_widget_destroy (invisible);
          return FALSE;
        }

      gtk_grab_add(invisible);

      g_signal_connect(invisible, "button_press_event", G_CALLBACK(context_help_button_press), w);
    }

  return FALSE;
}

gboolean
HelpWindow::context_help_button_press(GtkWidget *widget, GdkEventButton *bevent, gpointer data)
{
  UI_Gtk_Window *w = (UI_Gtk_Window *)data;

  GtkWidget *help_widget = gtk_get_event_widget((GdkEvent *)bevent);
  
  if (help_widget && (bevent->button == 1) && (bevent->type == GDK_BUTTON_PRESS))
    {
      gtk_grab_remove(widget);
      gdk_display_pointer_ungrab(gtk_widget_get_display(widget), bevent->time);
      gtk_widget_destroy(widget);

      if (help_widget != widget)
	sf_help_recursive(help_widget, w);
    }

  return TRUE;
}
