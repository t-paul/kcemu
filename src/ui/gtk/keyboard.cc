/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: keyboard.cc,v 1.1 2002/10/31 01:38:12 torsten_paul Exp $
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

#include <stdio.h>
#include <string.h>

#include <stdio.h>
#include <string.h>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/keys.h"
#include "kc/keyboard.h"

#include "cmd/cmd.h"

#include "ui/gtk/cmd.h"
#include "ui/gtk/ui_gtk.h"
#include "ui/gtk/keyboard.h"

#include "libdbg/dbg.h"

// #define DEBUG_REGIONS 1

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

KeyboardWindow::KeyboardWindow(void)
{
  _key_active = FALSE;
  _key_pressed = NULL;

  _keys[0].key = NULL;
  _pixbuf_normal = NULL;
  _pixbuf_pressed = NULL;

  _cmd = new CMD_ui_keyboard_window_toggle(this);
}

KeyboardWindow::~KeyboardWindow(void)
{
  delete _cmd;
}

gboolean
KeyboardWindow::sf_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  int w, h;
  KeyboardWindow *self = (KeyboardWindow *)data;

  gdk_window_set_back_pixmap(self->_w.canvas->window, NULL, FALSE);

  w = self->_w.canvas->allocation.width;
  h = self->_w.canvas->allocation.height;
  gdk_draw_rectangle(self->_w.canvas->window,
		     self->_w.canvas->style->black_gc,
		     FALSE, 0, 0, w - 1, h - 1);
  gdk_draw_pixbuf(self->_w.canvas->window,
		  self->_w.canvas->style->fg_gc[GTK_STATE_NORMAL],
		  self->_pixbuf_normal,
		  0, 0,
		  1, 1,
		  w - 2, h - 2,
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
#ifdef DEBUG_REGIONS
      self->debug_regions(event);
#endif /* DEBUG_REGIONS */
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
KeyboardWindow::debug_regions(GdkEventMotion *event)
{
#ifdef DEBUG_REGIONS
  static GdkColor     red;
  static GdkColor     blue;
  static GdkGC       *gc;
  static GdkColormap *colormap = NULL;

  int a, b;
  int n_rectangles;
  GdkRectangle *rectangles;

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
#endif /* DEBUG_REGIONS */
}

void
KeyboardWindow::init_key_regions(void)
{
  int a;
  FILE *f;
  int state;
  char *ptr;
  char buf[1024];
  GdkRectangle r;
  const char *filename = NULL;

  switch (get_kc_type())
    {
    case KC_TYPE_85_2:
    case KC_TYPE_85_3:
    case KC_TYPE_85_4:
    case KC_TYPE_85_5:
      filename = "kc854.key";
      break;
    case KC_TYPE_85_1:
    case KC_TYPE_87:
      filename = "kc851.key";
      break;
    case KC_TYPE_LC80:
      filename = "lc80.key";
      break;
    case KC_TYPE_POLY880:
      filename = "poly880.key";
      break;
    case KC_TYPE_Z1013:
    case KC_TYPE_A5105:
    case KC_TYPE_KRAMERMC:
    case KC_TYPE_MUGLERPC:
      break;

      /*
       *  We don't use a default so we get a warning if the enum
       *  is extended; the following values can't be returned by
       *  get_kc_type() though.
       */
    case KC_TYPE_NONE:
    case KC_TYPE_85_1_CLASS:
    case KC_TYPE_85_2_CLASS:
    case KC_TYPE_ALL:
      break;
    }


  if (filename == NULL)
    return;

  ptr = new char[strlen(kcemu_datadir) + strlen(filename) + 2];
  strcpy(ptr, kcemu_datadir);
  strcat(ptr, "/");
  strcat(ptr, filename);

  f = fopen(ptr, "rb");
  delete[] ptr;
  if (f == NULL)
    return;

  if (fgets(buf, 1024, f) == NULL)
    {
      fclose(f);
      return;
    }

  ptr = strchr(buf, '\n');
  if (ptr != NULL)
    *ptr = '\0';

  ptr = new char[strlen(kcemu_datadir) + strlen(buf) + 2];
  strcpy(ptr, kcemu_datadir);
  strcat(ptr, "/");
  strcat(ptr, buf);

  _pixbuf_normal = gdk_pixbuf_new_from_file(ptr, NULL);
  if (_pixbuf_normal == NULL)
    {
      printf("can't load keyboard pixmap '%s'\n", ptr);
      fclose(f);
      delete[] ptr;
      return;
    }
  delete[] ptr;

  if (fgets(buf, 1024, f) == NULL)
    {
      fclose(f);
      return;
    }

  ptr = strchr(buf, '\n');
  if (ptr != NULL)
    *ptr = '\0';

  ptr = new char[strlen(kcemu_datadir) + strlen(buf) + 2];
  strcpy(ptr, kcemu_datadir);
  strcat(ptr, "/");
  strcat(ptr, buf);

  _pixbuf_pressed = gdk_pixbuf_new_from_file(ptr, NULL);
  if (_pixbuf_pressed == NULL)
    {
      printf("can't load keyboard pixmap '%s'\n", ptr);
      fclose(f);
      delete[] ptr;
      return;
    }
  delete[] ptr;

  a = -1;
  state = 0;
  while (state >= 0)
    {
      if (fgets(buf, 1024, f) == NULL)
	break;

      if (buf[0] == '#')
	continue;

      ptr = strchr(buf, '\n');
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

#ifdef DEBUG_REGIONS
  printf("%d key definitions loaded.\n", a);
#endif /* DEBUG_REGIONS */

  fclose(f);
}

void
KeyboardWindow::init(void)
{
  init_key_regions();

  _window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name(_window, "KeyboardWindow");
  gtk_window_set_title(GTK_WINDOW(_window), _("KCemu: Keyboard"));
  gtk_window_set_resizable(GTK_WINDOW(_window), FALSE);
  gtk_signal_connect(GTK_OBJECT(_window), "delete_event",
		     GTK_SIGNAL_FUNC(cmd_exec_sft),
		     (char *)"ui-keyboard-window-toggle"); // FIXME:
  gtk_signal_connect(GTK_OBJECT(_window), "key_press_event",
		     GTK_SIGNAL_FUNC(sf_key_press), this);
  gtk_signal_connect(GTK_OBJECT(_window), "key_release_event",
		     GTK_SIGNAL_FUNC(sf_key_release), this);

  /*
   *  vbox
   */
  _w.vbox = gtk_vbox_new(FALSE, 2);
  gtk_container_border_width(GTK_CONTAINER(_w.vbox), 6);
  gtk_container_add(GTK_CONTAINER(_window), _w.vbox);
  gtk_widget_show(_w.vbox);

  _w.label = NULL;
  _w.canvas = NULL;
  _w.eventbox = NULL;
  if ((_pixbuf_normal != NULL) && (_pixbuf_pressed != NULL))
    {
      /*
       *  eventbox
       */
      _w.eventbox = gtk_event_box_new();
      gtk_signal_connect(GTK_OBJECT(_w.eventbox), "motion_notify_event",
			 GTK_SIGNAL_FUNC(sf_motion_notify), this);
      gtk_signal_connect(GTK_OBJECT(_w.eventbox), "button_press_event",
			 GTK_SIGNAL_FUNC(sf_button_press), this);
      gtk_signal_connect(GTK_OBJECT(_w.eventbox), "button_release_event",
			 GTK_SIGNAL_FUNC(sf_button_release), this);
      gtk_widget_set_events(_w.eventbox, (GDK_POINTER_MOTION_MASK |
					  GDK_BUTTON_PRESS_MASK |
					  GDK_BUTTON_RELEASE_MASK));
      gtk_box_pack_start(GTK_BOX(_w.vbox), _w.eventbox, FALSE, FALSE, 5);
      gtk_widget_show(_w.eventbox);

      /*
       *  canvas
       */
      _w.canvas = gtk_drawing_area_new();
      gtk_widget_set_events(_w.canvas, GDK_EXPOSURE_MASK);
      gtk_signal_connect(GTK_OBJECT(_w.canvas), "expose_event",
			 GTK_SIGNAL_FUNC(sf_expose), this);
      gtk_container_add(GTK_CONTAINER(_w.eventbox), _w.canvas);
      gtk_widget_set_usize(_w.canvas,
			   gdk_pixbuf_get_width(_pixbuf_normal),
			   gdk_pixbuf_get_height(_pixbuf_normal));
      gtk_widget_show(_w.canvas);
    }
  else
    {
      _w.label = gtk_label_new(_("\n  Sorry, keyboard display not configured.  \n"));
      gtk_box_pack_start(GTK_BOX(_w.vbox), _w.label, FALSE, FALSE, 5);
      gtk_widget_show(_w.label);
    }

  /*
   *  separator
   */
  _w.separator = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.separator,
		     FALSE, FALSE, 5);
  gtk_widget_show(_w.separator);

  /*
   *  close button
   */
  _w.close = gtk_button_new_with_label(_("Close"));
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.close, FALSE, FALSE, 5);
  gtk_signal_connect(GTK_OBJECT(_w.close), "clicked",
		     GTK_SIGNAL_FUNC(cmd_exec_sf),
		     (gpointer)"ui-keyboard-window-toggle");
  GTK_WIDGET_SET_FLAGS(_w.close, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(_w.close);
  gtk_widget_show(_w.close);
}
