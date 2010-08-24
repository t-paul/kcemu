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

#include <string.h>

#include "kc/system.h"
#include "kc/prefs/types.h"
#include "kc/prefs/prefs.h"

#include "kc/keys.h"
#include "kc/keyboard.h"

#include "cmd/cmd.h"

#include "ui/gtk/cmd.h"
#include "ui/gtk/ui_gtk.h"
#include "ui/gtk/keyboard.h"

#include "libdbg/dbg.h"

using namespace std;

static struct {
  const char *name;
  int key_val;
} _key_names[] = {
  { "KC_KEY_ENTER", KC_KEY_ENTER },
  { "KC_KEY_SPACE", KC_KEY_SPACE },
  { "KC_KEY_ESC", KC_KEY_ESC },
  { "KC_KEY_F1", KC_KEY_F1 },
  { "KC_KEY_F2", KC_KEY_F2 },
  { "KC_KEY_F3", KC_KEY_F3 },
  { "KC_KEY_F4", KC_KEY_F4 },
  { "KC_KEY_F5", KC_KEY_F5 },
  { "KC_KEY_F6", KC_KEY_F6 },
  { "KC_KEY_F7", KC_KEY_F7 },
  { "KC_KEY_F8", KC_KEY_F8 },
  { "KC_KEY_F9", KC_KEY_F9 },
  { "KC_KEY_F10", KC_KEY_F10 },
  { "KC_KEY_F11", KC_KEY_F11 },
  { "KC_KEY_F12", KC_KEY_F12 },
  { "KC_KEY_F13", KC_KEY_F13 },
  { "KC_KEY_F14", KC_KEY_F14 },
  { "KC_KEY_F15", KC_KEY_F15 },
  { "KC_KEY_SHIFT", KC_KEY_SHIFT },
  { "KC_KEY_CONTROL", KC_KEY_CONTROL },
  { "KC_KEY_ALT", KC_KEY_ALT },
  { "KC_KEY_ALT_GR", KC_KEY_ALT_GR },
  { "KC_KEY_LEFT", KC_KEY_LEFT },
  { "KC_KEY_RIGHT", KC_KEY_RIGHT },
  { "KC_KEY_DOWN", KC_KEY_DOWN },
  { "KC_KEY_UP", KC_KEY_UP },
  { "KC_KEY_HOME", KC_KEY_HOME },
  { "KC_KEY_END", KC_KEY_END },
  { "KC_KEY_DEL", KC_KEY_DEL },
  { "KC_KEY_INSERT", KC_KEY_INSERT },
  { "KC_KEY_PAGE_UP", KC_KEY_PAGE_UP },
  { "KC_KEY_PAGE_DOWN", KC_KEY_PAGE_DOWN },
  { "KC_KEY_PAUSE", KC_KEY_PAUSE },
  { "KC_KEY_PRINT", KC_KEY_PRINT },
  { "KC_KEY_COLOR", KC_KEY_COLOR },
  { "KC_KEY_GRAPHIC", KC_KEY_GRAPHIC },
  { "KC_KEY_LIST", KC_KEY_LIST },
  { "KC_KEY_RUN", KC_KEY_RUN },
  { "KC_KEY_STOP", KC_KEY_STOP },
  { "KC_KEY_SHIFT_LOCK", KC_KEY_SHIFT_LOCK },
  { "KC_KEY_RESET", KC_KEY_RESET },
  { "KC_KEY_NMI", KC_KEY_NMI },
  { "KC_KEY_BREAK", KC_KEY_BREAK },
  { "KC_KEY_CLEAR", KC_KEY_CLEAR },
  { NULL, -1 },
};

class CMD_ui_keyboard_window_toggle : public CMD
{
private:
  KeyboardWindow *_w;

public:
  CMD_ui_keyboard_window_toggle(KeyboardWindow *w) : CMD("ui-keyboard-window-toggle")
    {
      _w = w;
      register_cmd("ui-keyboard-window-toggle");
    }

  void execute(CMD_Args *args, CMD_Context context)
    {
      _w->toggle();
    }
};

KeyboardWindow::KeyboardWindow(const char *ui_xml_file) : UI_Gtk_Window(ui_xml_file)
{
  _key_active = FALSE;
  _key_pressed = NULL;

  _keys[0].key = NULL;
  _pixbuf_normal = NULL;
  _pixbuf_pressed = NULL;
    
  _delay = 0;
  _has_info = false;

  _cmd = new CMD_ui_keyboard_window_toggle(this);
}

KeyboardWindow::~KeyboardWindow(void)
{
  delete _cmd;
}

gboolean
KeyboardWindow::sf_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  KeyboardWindow *self = (KeyboardWindow *)data;

  gdk_window_set_back_pixmap(self->_w.canvas->window, NULL, FALSE);

  int w = self->_w.canvas->allocation.width;
  int h = self->_w.canvas->allocation.height;
  gdk_draw_rectangle(self->_w.canvas->window,
		     self->_w.canvas->style->black_gc,
		     FALSE, 0, 0, w - 1, h - 1);

  int width = gdk_pixbuf_get_width(self->_pixbuf_normal);
  int height = gdk_pixbuf_get_height(self->_pixbuf_normal);
  gdk_draw_pixbuf(self->_w.canvas->window,
		  self->_w.canvas->style->fg_gc[GTK_STATE_NORMAL],
		  self->_pixbuf_normal,
		  0, 0,
		  1, 1,
		  width, height,
		  GDK_RGB_DITHER_NONE,
		  0, 0);

  return FALSE;
}

void
KeyboardWindow::draw_key_normal(struct _key_struct *key)
{
  gdk_draw_pixbuf(_w.canvas->window,
		  _w.canvas->style->fg_gc[GTK_STATE_NORMAL],
		  _pixbuf_normal,
		  key->rect.x,
		  key->rect.y,
		  key->rect.x + 1,
		  key->rect.y + 1,
		  key->rect.width,
		  key->rect.height,
		  GDK_RGB_DITHER_NONE,
		  0, 0);
}

void
KeyboardWindow::draw_key_pressed(struct _key_struct *key)
{
  gdk_draw_pixbuf(_w.canvas->window,
		  _w.canvas->style->fg_gc[GTK_STATE_NORMAL],
		  _pixbuf_pressed,
		  key->rect.x,
		  key->rect.y,
		  key->rect.x + 1,
		  key->rect.y + 1,
		  key->rect.width,
		  key->rect.height,
		  GDK_RGB_DITHER_NONE,
		  0, 0);
}

gboolean
KeyboardWindow::sf_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  int a;
  KeyboardWindow *self = (KeyboardWindow *)data;

  for (a = 0;self->_keys[a].key != NULL;a++)
    {
      if (self->_keys[a].key_val < 0)
	continue;

      if (gdk_region_point_in(self->_keys[a].region, (int)event->x - 1, (int)event->y - 1))
	{
	  self->_key_active = TRUE;
	  self->_key_pressed = &self->_keys[a];
	  self->draw_key_pressed(self->_key_pressed);

	  switch (event->button)
	    {
	    case 2:
	      keyboard->keyPressed(KC_KEY_CONTROL, KC_KEY_CONTROL);
	      break;
	    case 3:
	      keyboard->keyPressed(KC_KEY_SHIFT, KC_KEY_SHIFT);
	      break;
	    }

	  keyboard->keyPressed(self->_key_pressed->key_val, self->_key_pressed->key_val);
	}
    }

  return TRUE;
}

gboolean
KeyboardWindow::sf_button_release(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  KeyboardWindow *self = (KeyboardWindow *)data;

  if (!self->_key_pressed)
    return TRUE;

  if (self->_key_active)
    self->_key_active = FALSE;

  self->draw_key_normal(self->_key_pressed);

  keyboard->keyReleased(self->_key_pressed->key_val, self->_key_pressed->key_val);
  switch (event->button)
    {
    case 2:
      keyboard->keyReleased(KC_KEY_CONTROL, KC_KEY_CONTROL);
      break;
    case 3:
      keyboard->keyReleased(KC_KEY_SHIFT, KC_KEY_SHIFT);
      break;
    }

  if (self->_key_pressed->key_val == KC_KEY_RESET)
    {
      CMD_EXEC("emu-reset");
    }
  else if (self->_key_pressed->key_val == KC_KEY_NMI)
    {
      CMD_EXEC("emu-nmi");
    }

  self->_key_pressed = NULL;

  return TRUE;
}

gboolean
KeyboardWindow::sf_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
  return UI_Gtk::sf_key_press(widget, event);
}

gboolean
KeyboardWindow::sf_key_release(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
  return UI_Gtk::sf_key_release(widget, event);
}

gboolean
KeyboardWindow::sf_motion_notify(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
  KeyboardWindow *self = (KeyboardWindow *)data;

  if (!self->_key_pressed)
    {
      self->check_regions(event);
      return TRUE;
    }

  if (gdk_region_point_in(self->_key_pressed->region,
			  (int)event->x - 1,
			  (int)event->y - 1))
    {
      if (self->_key_active)
	return TRUE;

      self->_key_active = TRUE;
      self->draw_key_pressed(self->_key_pressed);
    }
  else
    {
      if (!self->_key_active)
	return TRUE;

      self->_key_active = FALSE;
      self->draw_key_normal(self->_key_pressed);
    }

  return TRUE;
}

gboolean
KeyboardWindow::timeout_callback(gpointer data)
{
  KeyboardWindow *self = (KeyboardWindow *)data;

  if (self->_delay == 0)
    return TRUE;

  self->_delay--;
  if (self->_delay != 0)
    return TRUE;

  gtk_label_set_text(GTK_LABEL(self->_w.label_info), "");
  return TRUE;
}

int
KeyboardWindow::get_key_val(const char *key)
{
  int a;

  for (a = 0;_key_names[a].name != NULL;a++)
    if (strcmp(_key_names[a].name, key) == 0)
      return _key_names[a].key_val;

  if (strlen(key) == 1)
    return key[0];

  return -1;
}

void
KeyboardWindow::check_regions(GdkEventMotion *event)
{
  static GdkColor     red;
  static GdkColor     blue;
  static GdkGC       *gc;
  static GdkColormap *colormap = NULL;

  int a, b;
  int n_rectangles;
  GdkRectangle *rectangles;

  bool debug = DBG_check("KCemu/KeyboardWindow/debug_regions");

  if (colormap == NULL)
    {
      gc = gdk_gc_new(_w.canvas->window);
      colormap = gdk_colormap_get_system();
      gdk_color_parse("#ff0000", &red);
      gdk_color_parse("#0000ff", &blue);
      gdk_color_alloc(colormap, &red);
      gdk_color_alloc(colormap, &blue);
    }

  for (a = 0;_keys[a].key != NULL;a++)
    {
      if (gdk_region_point_in(_keys[a].region, (int)event->x - 1, (int)event->y - 1))
	{
          _delay = 5;
          gtk_label_set_text(GTK_LABEL(_w.label_info), _keys[a].info == NULL ? "" : gettext(_keys[a].info));

          if (!debug)
            continue;

	  sf_expose(_w.canvas, NULL, this);
	  gdk_gc_set_foreground(gc, &red);
	  gdk_draw_rectangle(_w.canvas->window, gc, FALSE,
			     _keys[a].rect.x + 1,
			     _keys[a].rect.y + 1,
			     _keys[a].rect.width - 1,
			     _keys[a].rect.height - 1);
	  gdk_draw_rectangle(_w.canvas->window, gc, FALSE,
			     _keys[a].rect.x,
			     _keys[a].rect.y,
			     _keys[a].rect.width + 1,
			     _keys[a].rect.height + 1);
	  gdk_region_get_rectangles(_keys[a].region, &rectangles, &n_rectangles);
	  gdk_gc_set_foreground(gc, &blue);
	  for (b = 0;b < n_rectangles;b++)
	    {
	      gdk_draw_rectangle(_w.canvas->window, gc, FALSE,
				 rectangles[b].x + 1,
				 rectangles[b].y + 1,
				 rectangles[b].width - 1,
				 rectangles[b].height - 1);
	    }
	}
    }
}

void
KeyboardWindow::init_key_regions(void)
{
  int a;
  FILE *f;
  int state;
  char buf[1024];
  GdkRectangle r;

  const EmulationType &emulation_type = Preferences::instance()->get_system_type()->get_emulation_type();
  const char *filename = emulation_type.get_keyboard_filename();
  if (filename == NULL)
    return;

  string datadir(kcemu_datadir);
  string keymap_dir = datadir + "/keymaps/";
  string keymap_filename = keymap_dir + filename;

  f = fopen(keymap_filename.c_str(), "rb");
  if (f == NULL)
    return;

  if (fgets(buf, 1024, f) == NULL)
    {
      fclose(f);
      return;
    }

  _pixbuf_normal = load_pixmap(keymap_dir.c_str(), buf);
  if (_pixbuf_normal == NULL)
    {
      fclose(f);
      return;
    }

  if (fgets(buf, 1024, f) == NULL)
    {
      fclose(f);
      return;
    }

  _pixbuf_pressed = load_pixmap(keymap_dir.c_str(), buf);
  if (_pixbuf_pressed == NULL)
    {
      fclose(f);
      return;
    }

  a = -1;
  state = 0;
  while (state >= 0)
    {
      if (fgets(buf, 1024, f) == NULL)
	break;

      if (buf[0] == '#')
	continue;

      char *ptr = strchr(buf, '\n');
      if (ptr != NULL)
	*ptr = '\0';

      switch (state)
	{
	case 2:
	  if (buf[0] == '=')
	    {
	      if (sscanf(buf, "=%d,%d,%d,%d", &r.x, &r.y, &r.width, &r.height) != 4)
		{
		  state = -1;
		  break;
		}
	      _keys[a].region = gdk_region_rectangle(&r);
	      break;
	    }
          else if (buf[0] == '?')
            {
              _has_info = true;
              _keys[a].info = strdup(buf + 1);
              break;
            }
	  else if (buf[0] != '+')
	    {
	      state = -1;
	      break;
	    }
	  /*
	   *  else reset state and fall through; this is the case when we
	   *  just read the first line for a new key definition
	   */
	  state = 0;
	case 0:
	  if (buf[0] != '+')
	    {
	      state = -1;
	      break;
	    }
	  a++;
	  _keys[a].key = strdup(buf + 1);
	  _keys[a].key_val = get_key_val(_keys[a].key);
          _keys[a].info = NULL;
	  if (_keys[a].key_val < 0)
	    {
	      DBG(0, form("KCemu/warning",
			  "Unknown key '%s' in file '%s'\n",
			  _keys[a].key, filename));
	    }
	  state++;
	  break;
	case 1:
	  if (buf[0] != ':')
	    {
	      state = -1;
	      break;
	    }
	  if (sscanf(buf, ":%d,%d,%d,%d",
		     &_keys[a].rect.x,
		     &_keys[a].rect.y,
		     &_keys[a].rect.width,
		     &_keys[a].rect.height) != 4)
	    {
	      state = -1;
	      break;
	    }
	  state++;
	  break;
	}
    }

  a++;
  _keys[a].key = NULL;

  DBG(0, form("KCemu/KeyboardWindow/debug_regions",
	      "%d key definitions loaded.\n", a));

  fclose(f);
}

GdkPixbuf *
KeyboardWindow::load_pixmap(const char *keymap_dir, char *filename_buffer)
{
  char *ptr = strchr(filename_buffer, '\n');
  if (ptr != NULL)
    *ptr = '\0';

  string dir(keymap_dir);
  string filename = dir + filename_buffer;
  GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(filename.c_str(), NULL);

  if (pixbuf == NULL)
    {
      DBG(0, form("KCemu/warning",
                  "KeyboardWindow::load_pixmap((): can't load keyboard pixmap '%s'\n",
                  filename.c_str()));
    }

  return pixbuf;
}

void
KeyboardWindow::init(void)
{
  init_key_regions();

  _window = get_widget("keyboard_window");
  gtk_window_set_title(GTK_WINDOW(_window), _("KCemu: Keyboard"));
  gtk_signal_connect(GTK_OBJECT(_window), "delete_event",
		     GTK_SIGNAL_FUNC(cmd_exec_sft),
		     (char *)"ui-keyboard-window-toggle"); // FIXME:
  gtk_signal_connect(GTK_OBJECT(_window), "key_press_event",
		     GTK_SIGNAL_FUNC(sf_key_press), this);
  gtk_signal_connect(GTK_OBJECT(_window), "key_release_event",
		     GTK_SIGNAL_FUNC(sf_key_release), this);

  _w.canvas = NULL;
  _w.eventbox = NULL;
  _w.notebook = get_widget("notebook");
  _w.not_configured_label = get_widget("not_configured_label");
  
  _w.label_info = get_widget("label_info");
  if (_has_info)
    {
      gtk_widget_show(_w.label_info);
      gtk_widget_show(get_widget("hseparator_info"));
    }
  
  if ((_pixbuf_normal != NULL) && (_pixbuf_pressed != NULL))
    {
      gtk_notebook_set_current_page(GTK_NOTEBOOK(_w.notebook), 0);
      
      /*
       *  eventbox
       */
      _w.eventbox = get_widget("eventbox");
      gtk_signal_connect(GTK_OBJECT(_w.eventbox), "motion_notify_event",
			 GTK_SIGNAL_FUNC(sf_motion_notify), this);
      gtk_signal_connect(GTK_OBJECT(_w.eventbox), "button_press_event",
			 GTK_SIGNAL_FUNC(sf_button_press), this);
      gtk_signal_connect(GTK_OBJECT(_w.eventbox), "button_release_event",
			 GTK_SIGNAL_FUNC(sf_button_release), this);

      /*
       *  canvas
       */
      _w.canvas = get_widget("drawingarea");
      gtk_signal_connect(GTK_OBJECT(_w.canvas), "expose_event",
			 GTK_SIGNAL_FUNC(sf_expose), this);
      gtk_widget_set_usize(_w.canvas,
			   gdk_pixbuf_get_width(_pixbuf_normal),
			   gdk_pixbuf_get_height(_pixbuf_normal));
    }
  else
    {
      gtk_label_set_text(GTK_LABEL(_w.not_configured_label), _("Sorry, keyboard display not configured."));
      gtk_misc_set_padding(GTK_MISC(_w.not_configured_label), 100, 50);
      gtk_notebook_set_current_page(GTK_NOTEBOOK(_w.notebook), 1);
    }

#if GLIB_CHECK_VERSION(2,14,0)
  g_timeout_add_seconds(1, timeout_callback, this);
#else
  g_timeout_add(1000, timeout_callback, this);
#endif

  init_dialog("ui-keyboard-window-toggle", "window-keyboard");
}
