/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: ui_gtk.cc,v 1.25 2002/10/31 01:38:12 torsten_paul Exp $
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

#include <iostream>
#include <iomanip>

#include <signal.h> /* FIXME: only for testing */
#include <string.h>
#include <unistd.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "kc/system.h"

#include "kc/rc.h"
#include "kc/z80.h"
#include "kc/keys.h"
#include "kc/memory.h" // text_update()
#include "kc/keyboard.h"

#include "cmd/cmd.h"

/*  hmm, Xlib.h defines Status as int :-( */
#undef Status
#include "ui/error.h"
#include "ui/status.h"
#include "ui/hsv2rgb.h"
#include "ui/gtk/cmd.h"

#include "ui/gtk/debug.h"
#include "ui/gtk/ui_gtk.h"

#include "libdbg/dbg.h"

using namespace std;

static UI_Gtk *self;

class CMD_ui_toggle : public CMD
{
private:
  UI_Gtk *_ui;
  
public:
  CMD_ui_toggle(UI_Gtk *ui) : CMD("ui-toggle")
    {
      _ui = ui;
      register_cmd("ui-status-bar-toggle", 0);
      register_cmd("ui-menu-bar-toggle", 1);
      register_cmd("ui-speed-limit-toggle", 2);
      register_cmd("ui-zoom-1", 3);
      register_cmd("ui-zoom-2", 4);
      register_cmd("ui-zoom-3", 5);
    }
  
  void execute(CMD_Args *args, CMD_Context context)
    {
      switch (context)
	{
	case 0:
	  _ui->status_bar_toggle();
	  break;
	case 1:
	  _ui->menu_bar_toggle();
	  break;
	case 2:
	  _ui->speed_limit_toggle();
	  break;
	case 3:
	case 4:
	case 5:
	  _ui->gtk_zoom(context - 2);
	  break;
	}
    }
};

class CMD_update_colortable : public CMD
{
private:
  UI_Gtk *_ui;
  ColorWindow *_colwin;

public:
  CMD_update_colortable(UI_Gtk *ui, ColorWindow *colwin) : CMD("ui-update-colortable")
    {
      _ui = ui;
      _colwin = colwin;
      register_cmd("ui-update-colortable");
    }

  void execute(CMD_Args *args, CMD_Context context)
    {
      _ui->allocate_colors(_colwin->get_saturation_fg(),
			   _colwin->get_saturation_bg(),
			   _colwin->get_brightness_fg(),
			   _colwin->get_brightness_bg(),
			   _colwin->get_black_level(),
			   _colwin->get_white_level());
      _ui->update(true, true);
    }
};

void
UI_Gtk::attach_remote_listener(void)
{
  GdkAtom atom;

  atom = gdk_atom_intern("_KCEMU_REMOTE_COMMAND", FALSE);
  gdk_property_change(_main.window->window,
                      atom, GDK_TARGET_STRING, 8, GDK_PROP_MODE_REPLACE,
                      (unsigned char *)"", 1);
  gdk_flush();
}

gboolean
UI_Gtk::property_change(GtkWidget *widget,
                        GdkEventProperty *event,
                        gpointer data)
{
  gboolean ret;
  guchar *prop_data;
  char *ptr, *val, *atom;
  GdkAtom actual_property_type;
  gint    actual_format, actual_length;
  CMD_Args *args;

  UI_Gtk *self = (UI_Gtk *)data;

  if (event == NULL)
    return TRUE;

  atom = gdk_atom_name(event->atom);
  if (atom == NULL)
    return TRUE;
  
  if (strcmp(atom, "_KCEMU_REMOTE_COMMAND") == 0)
    {
      DBG(1, form("KCemu/UI/remote",
                  "property_change: %s\n",
                  atom));
      
      prop_data = NULL;
      ret = gdk_property_get(self->_main.window->window,
			     event->atom, GDK_TARGET_STRING,
			     0, (65536 / sizeof(long)), FALSE,
			     &actual_property_type,
			     &actual_format, &actual_length,
			     &prop_data);
      
      if (!ret || (*prop_data == '\0'))
	{
	  DBG(1, form("KCemu/UI/remote",
		      "empty or invalid property!\n"));
	}
      else
	{
	  ptr = (char *)prop_data;
	  DBG(1, form("KCemu/UI/remote",
		      "command: %s'\n",
		      ptr));
	  args = new CMD_Args();
	  while (242)
	    {
	      ptr += strlen(ptr) + 1;
	      if ((ptr - (char *)prop_data) >= actual_length)
		break;
	      val = strchr(ptr, '=');
	      if (!val)
		continue;
	      *val++ = '\0';
	      DBG(1, form("KCemu/UI/remote",
			  " arg: %s -> '%s'\n",
			  ptr, val));
	      args->set_string_arg(ptr, val);
	    }
	  
	  CMD_EXEC_ARGS((const char *)prop_data, args);
	}
      
      if (prop_data != NULL)
	g_free(prop_data);
      
    }
  
  g_free(atom);

  return TRUE;
}

void
UI_Gtk::idle(void)
{
  gtk_main_quit();
}

void
UI_Gtk::sf_selection_received(GtkWidget *widget,
                              GtkSelectionData *sel_data,
                              gpointer *data)
{
  GdkAtom atom;
  UI_Gtk *self;
  CMD_Args *args;

  self = (UI_Gtk *)data;
  if (sel_data->length < 0)
    return;

  atom = gdk_atom_intern("TEXT", FALSE);
  if (atom == GDK_NONE)
    return;

  /*
   *  may check sel_data->type here...
   */

  // printf("selection (%d bytes) = %s\n", sel_data->length, sel_data->data);
  args = new CMD_Args();
  args->set_string_arg("text", (const char *)sel_data->data);
  CMD_EXEC_ARGS("keyboard-replay", args);
}

void
UI_Gtk::sf_button_press(GtkWidget */*widget*/, GdkEventButton *event)
{
  GdkAtom atom;

  switch (event->button)
    {
    case 2:
      /* hmm, is TEXT the correct selection type ??? */
      atom = gdk_atom_intern("TEXT", FALSE);
      if (atom == GDK_NONE)
        break;
      gtk_selection_convert(self->_main.window,
                            GDK_SELECTION_PRIMARY,
                            atom,
                            GDK_CURRENT_TIME);

      break;
    case 3:
      gtk_menu_popup(GTK_MENU(self->_main.menu), NULL, NULL, NULL, NULL,
                     3, event->time);
      break;
    }
}

void
UI_Gtk::sf_expose(void)
{
  static int x = 1;

  if (ui) ui->update(1);
  if (x)
    {
      x = 0;
      self->attach_remote_listener();
    }      
}

void
UI_Gtk::key_press_release(GdkEventKey *event, bool press)
{
  int c = 0;
  int key_code;

  key_code = event->hardware_keycode;

  switch (event->keyval)
    {
    case GDK_Alt_L:
    case GDK_Alt_R:
    case GDK_Meta_L:
    case GDK_Meta_R:
    case GDK_Super_L:
    case GDK_Super_R:
    case GDK_Hyper_L:
    case GDK_Hyper_R:          c = KC_KEY_ALT;     break;
    case GDK_Mode_switch:      c = KC_KEY_ALT_GR;  break;
    case GDK_Shift_L:
    case GDK_Shift_R:          c = KC_KEY_SHIFT;   break;
    case GDK_Control_L:
    case GDK_Control_R:        c = KC_KEY_CONTROL; break;
    case GDK_Left:
    case GDK_KP_Left:          c = KC_KEY_LEFT;    break;
    case GDK_Right:
    case GDK_KP_Right:         c = KC_KEY_RIGHT;   break;
    case GDK_Up:
    case GDK_KP_Up:            c = KC_KEY_UP;      break;
    case GDK_Down:
    case GDK_KP_Down:          c = KC_KEY_DOWN;    break;
    case GDK_Escape:           c = KC_KEY_ESC;     break;
    case GDK_Home:             c = KC_KEY_HOME;    break;
    case GDK_End:              c = KC_KEY_END;     break;
    case GDK_Pause:            c = KC_KEY_PAUSE;   break;
    case GDK_Print:            c = KC_KEY_PRINT;   break;
    case GDK_Delete:
    case GDK_KP_Delete:        c = KC_KEY_DEL;     break;
    case GDK_Insert:
    case GDK_KP_Insert:        c = KC_KEY_INSERT;  break;
    case GDK_Page_Up:
    case GDK_KP_Page_Up:       c = KC_KEY_PAGE_UP; break;
    case GDK_Page_Down:
    case GDK_KP_Page_Down:     c = KC_KEY_PAGE_DOWN; break;
    case GDK_F1:
    case GDK_KP_F1:            c = KC_KEY_F1;      break;
    case GDK_F2:
    case GDK_KP_F2:            c = KC_KEY_F2;      break;
    case GDK_F3:
    case GDK_KP_F3:            c = KC_KEY_F3;      break;
    case GDK_F4:
    case GDK_KP_F4:            c = KC_KEY_F4;      break;
    case GDK_F5:               c = KC_KEY_F5;      break;
    case GDK_F6:               c = KC_KEY_F6;      break;
    case GDK_F7:               c = KC_KEY_F7;      break;
    case GDK_F8:               c = KC_KEY_F8;      break;
    case GDK_F9:               c = KC_KEY_F9;      break;
    case GDK_F10:              c = KC_KEY_F10;     break;
    case GDK_F11:              c = KC_KEY_F11;     break;
    case GDK_F12:              c = KC_KEY_F12;     break;
    case GDK_F13:              c = KC_KEY_F13;     break;
    case GDK_F14:              c = KC_KEY_F14;     break;
    case GDK_F15:              c = KC_KEY_F15;     break;
    case GDK_KP_0:             c = '0';            break;
    case GDK_KP_1:             c = '1';            break;
    case GDK_KP_2:             c = '2';            break;
    case GDK_KP_3:             c = '3';            break;
    case GDK_KP_4:             c = '4';            break;
    case GDK_KP_5:             c = '5';            break;
    case GDK_KP_6:             c = '6';            break;
    case GDK_KP_7:             c = '7';            break;
    case GDK_KP_8:             c = '8';            break;
    case GDK_KP_9:             c = '9';            break;
    case GDK_KP_Equal:         c = '=';            break;
    case GDK_KP_Multiply:      c = '*';            break;
    case GDK_KP_Add:           c = '+';            break;
    case GDK_KP_Subtract:      c = '-';            break;
    case GDK_KP_Divide:        c = '/';            break;
    case GDK_KP_Enter:         c = 0x0d;           break;
    default:
      c = event->keyval & 0xff;
      break;
    }

  DBG(2, form("KCemu/UI/key_kc",
              "%s - keyval = %5d/0x%04x / keycode = %5d -> kccode = %5d/0x%04x\n",
              press ? "press  " : "release",
              event->keyval, event->keyval, key_code, c, c));
  
  if (press)
    keyboard->keyPressed(c, key_code);
  else
    keyboard->keyReleased(c, key_code);
}

gboolean
UI_Gtk::sf_key_press(GtkWidget */*widget*/, GdkEventKey *event)
{
  DBG(2, form("KCemu/UI/key_press",
              "key_press:   keyval = %04x, keycode = %04x\n",
              event->keyval, event->hardware_keycode));

  key_press_release(event, true);

  return TRUE;
}

gboolean
UI_Gtk::sf_key_release(GtkWidget */*widget*/, GdkEventKey *event)
{
  DBG(2, form("KCemu/UI/key_release",
              "key_release: keyval = %04x, keycode = %04x\n",
              event->keyval, event->hardware_keycode));

  key_press_release(event, false);

  return TRUE;
}

void
UI_Gtk::sf_focus_in(GtkWidget * /* widget */, GdkEventFocus *event)
{
  DBG(2, form("KCemu/UI/focus_in",
              "got focus\n"));

  keyboard->keyReleased(-1, -1);
}

void
UI_Gtk::sf_focus_out(GtkWidget *widget, GdkEventFocus *event)
{
  DBG(2, form("KCemu/UI/focus_out",
              "lost focus\n"));

  keyboard->keyReleased(-1, -1);
}

void
UI_Gtk::sf_leave_notify(GtkWidget *widget, GdkEventCrossing *event)
{
  keyboard->keyReleased(-1, -1);
}

void
UI_Gtk::status_bar_toggle(void)
{
  if (GTK_WIDGET_VISIBLE(_main.st_hbox))
    gtk_widget_hide(_main.st_hbox);
  else
    gtk_widget_show(_main.st_hbox);
}

void
UI_Gtk::menu_bar_toggle(void)
{
  if (GTK_WIDGET_VISIBLE(_main.menubar))
    gtk_widget_hide(_main.menubar);
  else
    gtk_widget_show(_main.menubar);
}

void
UI_Gtk::speed_limit_toggle(void)
{
  _speed_limit = !_speed_limit;
}

void
UI_Gtk::text_update(void)
{
#if 0
  static GdkFont *font = 0;
  unsigned long val = 0;

  if (font == 0)
    font = gdk_font_load("fixed");

  byte_t *irm = memory->getIRM();
  for (int a = 0;a < 8;a++)
    {
      val |= (irm[40 * a + 320] & 0xf);
      val <<= 4;
    }

  char buf[100];
  snprintf(buf, 100, "%08x", val);
  gdk_draw_string(_text.canvas->window, font, _gc, 10, 10, buf);
#endif
}

void
UI_Gtk::gtk_sync(void)
{
  static int count = 0;
  static bool first = true;
  static long tv_sec, tv_usec;
  static long tv1_sec = 0, tv1_usec = 0;
  static long tv2_sec, tv2_usec;
  static unsigned long frame = 25;
  static unsigned long long base, d2;
  static long basetime_sec = 0, basetime_usec = 0;

  char buf[10];
  unsigned long timeframe, diff, fps;

  if (++count >= 60)
    {
      count = 0;
      sys_gettimeofday(&tv2_sec, &tv2_usec);
      diff = ((1000000 * (tv2_sec - tv1_sec)) + (tv2_usec - tv1_usec));
      fps = 60500000 / diff;
      sprintf(buf, " %ld fps ", fps);
      gtk_label_set(GTK_LABEL(_main.st_fps), buf);
      tv1_sec = tv2_sec;
      tv1_usec = tv2_usec;
    }

  if (first)
    {
      first = false;
      sys_gettimeofday(&tv1_sec, &tv1_usec);
      sys_gettimeofday(&basetime_sec, &basetime_usec);
      base = (basetime_sec * 50) + basetime_usec / 20000;
      base -= 26; // see comment below
    }

  sys_gettimeofday(&tv_sec, &tv_usec);
  d2 = (tv_sec * 50) + tv_usec / 20000;
  timeframe = (unsigned long)(d2 - base);
  frame++;
  
  /*
   *  because of this test we start with frame = 25 otherwise it
   *  would fail due to the fact that timeframe is unsigned!
   */
  if (frame < (timeframe - 20))
    {
      DBG(2, form("KCemu/UI/update",
                  "counter = %lu, frame = %lu, timeframe = %lu\n",
                  (unsigned long)z80->getCounter(), frame, timeframe));
      frame = timeframe;
    }

  if (_speed_limit)
    {
      if (frame > (timeframe + 1))
	{
	  sys_usleep(20000 * (frame - timeframe - 1));
	}
    }
  else
    {
      frame = timeframe;
    }

  /*
    if (!_auto_skip)
    {
    processEvents();
    update();
    }
  */

  sys_gettimeofday(&tv_sec, &tv_usec);
  d2 = (tv_sec * 50) + tv_usec / 20000;
  timeframe = (unsigned long)(d2 - base);
  _auto_skip = false;

  if (frame < timeframe)
    {
      if (++_cur_auto_skip > _max_auto_skip)
	_cur_auto_skip = 0;
      else
	_auto_skip = true;
    }
}

void
UI_Gtk::hsv_to_gdk_color(double h, double s, double v, GdkColor *col)
{
  int r, g, b;

  hsv2rgb(h, s, v, &r, &g, &b);
  col->red   = r << 8;
  col->green = g << 8;
  col->blue  = b << 8;
}

/*
 *  define some ugly typecasts to suppress some
 *  compiler warnings :-(
 */
#define CF(func)   ((GtkItemFactoryCallback)(func))
#define CD(string) ((unsigned int)(string))

void
UI_Gtk::create_main_window(void)
{
  GtkItemFactoryEntry entries[] = {
    { _("/_File"),                 NULL,     NULL,            0,                               "<Branch>" },
    { _("/File/Run..."),           NULL,     CF(cmd_exec_mc), CD("kc-image-run"),              NULL },
    { _("/File/Load..."),          "<alt>L", CF(cmd_exec_mc), CD("kc-image-load"),             NULL },
    { _("/File/Tape..."),          "<alt>T", CF(cmd_exec_mc), CD("ui-tape-window-toggle"),     NULL },
    { _("/File/Disk..."),          "<alt>D", CF(cmd_exec_mc), CD("ui-disk-window-toggle"),     NULL },
    { _("/File/Module..."),        "<alt>M", CF(cmd_exec_mc), CD("ui-module-window-toggle"),   NULL },
    { _("/File/Audio..."),         "<alt>A", CF(cmd_exec_mc), CD("ui-wav-window-toggle"),      NULL },
    { _("/File/sep1"),             NULL,     NULL,            0,                               "<Separator>" },
    { _("/File/Reset"),            "<alt>R", CF(cmd_exec_mc), CD("emu-reset"),                 NULL },
    { _("/File/Power On"),         NULL,     CF(cmd_exec_mc), CD("emu-power-on"),              NULL },
    { _("/File/sep2"),             NULL,     NULL,            0,                               "<Separator>" },
    { _("/File/Quit Emulator"),    "<alt>Q", CF(cmd_exec_mc), CD("emu-quit"),                  NULL },
    { _("/_View"),                 NULL,     NULL,            0,                               "<Branch>" },
    { _("/View/Zoom x1"),          "<alt>1", CF(cmd_exec_mc), CD("ui-zoom-1"),                 NULL },
    { _("/View/Zoom x2"),          "<alt>2", CF(cmd_exec_mc), CD("ui-zoom-2"),                 NULL },
    { _("/View/Zoom x3"),          "<alt>3", CF(cmd_exec_mc), CD("ui-zoom-3"),                 NULL },
    { _("/View/Keyboard"),         "<alt>K", CF(cmd_exec_mc), CD("ui-keyboard-window-toggle"), NULL },
    { _("/View/Debugger"),         NULL,     CF(cmd_exec_mc), CD("ui-debug-window-toggle"),    NULL },
    { _("/View/Info"),             "<alt>I", CF(cmd_exec_mc), CD("ui-info-window-toggle"),     NULL },
    { _("/View/Menubar"),          NULL,     CF(cmd_exec_mc), CD("ui-menu-bar-toggle"),        NULL },
    { _("/View/Statusbar"),        NULL,     CF(cmd_exec_mc), CD("ui-status-bar-toggle"),      NULL },
    { _("/_Configuration"),        NULL,     NULL,            0,                               "<Branch>" },
    { _("/Configuration/Colors"),  "<alt>C", CF(cmd_exec_mc), CD("ui-color-window-toggle"),    NULL },
    { _("/Configuration/No Speed Limit"),NULL, CF(cmd_exec_mc), CD("ui-speed-limit-toggle"),     "<ToggleItem>" },
    { _("/_Help"),                 NULL,     NULL,            0,                               "<LastBranch>" },
    { _("/Help/About KCemu"),      NULL,     CF(cmd_exec_mc), CD("ui-about-window-toggle"),    NULL },
    { _("/Help/sep3"),             NULL,     NULL,            0,                               "<Separator>" },
    { _("/Help/KCemu Licence"),    NULL,     CF(cmd_exec_mc), CD("ui-copying-window-toggle"),  NULL },
    { _("/Help/No Warranty!"),     NULL,     CF(cmd_exec_mc), CD("ui-warranty-window-toggle"), NULL },
  };
  GtkItemFactoryEntry entriesP[] = {
    { _("/_Run..."),               NULL,     CF(cmd_exec_mc), CD("kc-image-run"),              NULL },
    { _("/_Load..."),              NULL,     CF(cmd_exec_mc), CD("kc-image-load"),             NULL },
    { _("/_Tape..."),              NULL,     CF(cmd_exec_mc), CD("ui-tape-window-toggle"),     NULL },
    { _("/_Disk..."),              NULL,     CF(cmd_exec_mc), CD("ui-disk-window-toggle"),     NULL },
    { _("/_Module..."),            NULL,     CF(cmd_exec_mc), CD("ui-module-window-toggle"),   NULL },
    { _("/_Audio..."),             NULL,     CF(cmd_exec_mc), CD("ui-wav-window-toggle"),      NULL },
    { _("/sep1"),                  NULL,     NULL,            0,                               "<Separator>" },
    { _("/_View"),                 NULL,     NULL,            0,                               "<Branch>" },
    { _("/View/Keyboard"),         NULL,     CF(cmd_exec_mc), CD("ui-keyboard-window-toggle"), NULL },
    { _("/View/Debugger"),         NULL,     NULL,            0,                               NULL },
    { _("/View/Info"),             NULL,     NULL,            0,                               NULL },
    { _("/View/Menubar"),          NULL,     CF(cmd_exec_mc), CD("ui-menu-bar-toggle"),        NULL },
    { _("/View/Statusbar"),        NULL,     CF(cmd_exec_mc), CD("ui-status-bar-toggle"),      NULL },
    { _("/sep2"),                  NULL,     NULL,            0,                               "<Separator>" },
    { _("/Reset"),                 NULL,     CF(cmd_exec_mc), CD("emu-reset"),                 NULL },
    { _("/Power On"),              NULL,     CF(cmd_exec_mc), CD("emu-power-on"),              NULL },
    { _("/sep3"),                  NULL,     NULL,            0,                               "<Separator>" },
    { _("/Quit Emulator"),         NULL,     CF(cmd_exec_mc), CD("emu-quit"),                  NULL },
  };
  static GtkTargetEntry targetlist[] = {
    { "STRING",        0, 1 },
    { "TEXT",          0, 2 },
    { "COMPOUND_TEXT", 0, 3 }
  };
  gint ntargets  = sizeof(targetlist) / sizeof(targetlist[0]);
  gint nentries  = sizeof(entries)    / sizeof(entries[0]);
  gint nentriesP = sizeof(entriesP)   / sizeof(entriesP[0]);

  /*
   *  main window
   */
  _main.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name(_main.window, "MainWindow");
  gtk_window_set_title(GTK_WINDOW(_main.window), get_title());
  gtk_signal_connect(GTK_OBJECT(_main.window), "delete_event",
                     GTK_SIGNAL_FUNC(cmd_exec_sft), (gpointer)"emu-quit");
  gtk_signal_connect(GTK_OBJECT(_main.window), "property_notify_event",
                     GTK_SIGNAL_FUNC(property_change), this);
  gtk_signal_connect(GTK_OBJECT(_main.window), "selection_received",
                     GTK_SIGNAL_FUNC(sf_selection_received), this);
  gtk_selection_add_targets(_main.window, GDK_SELECTION_PRIMARY,
                            targetlist, ntargets);
  gtk_signal_connect(GTK_OBJECT(_main.window), "key_press_event",
		     GTK_SIGNAL_FUNC(UI_Gtk::sf_key_press), NULL);
  gtk_signal_connect(GTK_OBJECT(_main.window), "key_release_event",
		     GTK_SIGNAL_FUNC(UI_Gtk::sf_key_release), NULL);
  gtk_signal_connect(GTK_OBJECT(_main.window), "focus_in_event",
                     GTK_SIGNAL_FUNC(&UI_Gtk::sf_focus_in), NULL);
  gtk_signal_connect(GTK_OBJECT(_main.window), "focus_out_event",
                     GTK_SIGNAL_FUNC(&UI_Gtk::sf_focus_out), NULL);
  gtk_signal_connect(GTK_OBJECT(_main.window), "button_press_event",
 		     GTK_SIGNAL_FUNC(UI_Gtk::sf_button_press), NULL);
  gtk_widget_set_events(_main.window, (GDK_KEY_PRESS_MASK |
                                       GDK_KEY_RELEASE_MASK |
                                       GDK_BUTTON_PRESS_MASK |
				       GDK_PROPERTY_CHANGE_MASK));

  /*
   *  main vbox
   */
  _main.vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(_main.window), _main.vbox);
  gtk_widget_show(_main.vbox);

  /*
   *  menubar item factory
   */
  _main.agroup = gtk_accel_group_new();
  _main.ifact = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<KCemu>", _main.agroup);
  gtk_item_factory_create_items(_main.ifact, nentries, entries, NULL);
  gtk_window_add_accel_group(GTK_WINDOW(_main.window), _main.agroup); // FIXME

  _main.agroupP = gtk_accel_group_new();
  _main.ifactP = gtk_item_factory_new(GTK_TYPE_MENU, "<KCemuP>", _main.agroupP);
  gtk_item_factory_create_items(_main.ifactP, nentriesP, entriesP, NULL);

  /*
   *  main menu bar
   */
  _main.menubar = gtk_item_factory_get_widget(_main.ifact, "<KCemu>");
  gtk_box_pack_start(GTK_BOX(_main.vbox), _main.menubar, FALSE, TRUE, 0);
  gtk_widget_show(_main.menubar);

  /*
   *  popup menu
   */
  _main.menu = gtk_item_factory_get_widget(_main.ifactP, "<KCemuP>");

  /*
   *  main canvas
   */
  _main.canvas = gtk_drawing_area_new();
  gtk_drawing_area_size(GTK_DRAWING_AREA(_main.canvas),
			get_width(), get_height());
  gtk_box_pack_start(GTK_BOX(_main.vbox), _main.canvas, FALSE, TRUE, 0);
  gtk_widget_show(_main.canvas);
  gtk_widget_set_events(_main.canvas, GDK_EXPOSURE_MASK);
  gtk_signal_connect(GTK_OBJECT(_main.canvas), "expose_event",
		     GTK_SIGNAL_FUNC(UI_Gtk::sf_expose), NULL);

  /*
   *  status hbox
   */
  _main.st_hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(_main.vbox), _main.st_hbox, FALSE, TRUE, 0);
  gtk_widget_show(_main.st_hbox);

  /*
   *  status label for fps display
   */
  _main.st_frame = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type(GTK_FRAME(_main.st_frame), GTK_SHADOW_IN);
  gtk_box_pack_start(GTK_BOX(_main.st_hbox), _main.st_frame, FALSE, TRUE, 0);
  gtk_widget_show(_main.st_frame);

  _main.st_fps = gtk_label_new("          ");
  gtk_container_add(GTK_CONTAINER(_main.st_frame), _main.st_fps);
  gtk_widget_show(_main.st_fps);

  /*
   *  statusbar
   */
  _main.st_statusbar = gtk_statusbar_new();
  gtk_box_pack_start(GTK_BOX(_main.st_hbox), _main.st_statusbar,
                     TRUE, TRUE, 0);
  gtk_statusbar_set_has_resize_grip(GTK_STATUSBAR(_main.st_statusbar), false);
  gtk_misc_set_padding(GTK_MISC(GTK_STATUSBAR(_main.st_statusbar)->label), 4, 0); // hmm, is there a better way?
  gtk_widget_show(_main.st_statusbar);
}

void
UI_Gtk::create_header_window(void)
{
  _header.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(_header.window), "Edit Header");

  gtk_widget_show(_header.window);
}

UI_Gtk::UI_Gtk(void)
{
  _gc = NULL;
  _image = NULL;
  _init = false;
}

UI_Gtk::~UI_Gtk(void)
{
  gtk_widget_destroy(_main.window);
        
  delete _about_window;
  delete _tape_window;
  delete _tape_add_window;
  delete _module_window;
  delete _copying_window;
  delete _debug_window;
  delete _info_window;
  delete _wav_window;
  delete _edit_header_window;
  delete _file_browser;
}

void
UI_Gtk::show_greeting(void)
{
  const char *msg = "KCemu v" KCEMU_VERSION;
  const char *variant = get_kc_variant_name();
  const char *fmt = _("%s (%s)");
  char *status = new char[strlen(msg) + strlen(variant) + strlen(fmt) + 10];
  sprintf(status, fmt, msg, variant);
  Status::instance()->setMessage(status);
  delete[] status;
}

void
UI_Gtk::gtk_resize(void)
{
  /*
   *  prevent early calls that my caused by module initialization
   *  (e.g. Z1013 GDC module)
   */
  if (!_init) // set by show() function
    return;

  if (_image)
    gdk_image_destroy(_image);
  _image  = gdk_image_new(GDK_IMAGE_FASTEST, _visual, get_width(), get_height());

  gtk_drawing_area_size(GTK_DRAWING_AREA(_main.canvas), get_width(), get_height());

  if (!GTK_WIDGET_VISIBLE(_main.window))
    gtk_widget_show(_main.window);

  if (_gc)
    gdk_gc_destroy(_gc);
  _gc = gdk_gc_new(GTK_WIDGET(_main.canvas)->window);

  _dirty_old = 0; // force reallocation of dirty buffer

#if 0
  switch (_visual->type)
    {
    case GDK_VISUAL_STATIC_GRAY:
      cout << "GDK_VISUAL_STATIC_GRAY" << endl;
      break;
    case GDK_VISUAL_GRAYSCALE:
      cout << "GDK_VISUAL_GRAYSCALE" << endl;
      break;
    case GDK_VISUAL_STATIC_COLOR:
      cout << "GDK_VISUAL_STATIC_COLOR" << endl;
      break;
    case GDK_VISUAL_PSEUDO_COLOR:
      cout << "GDK_VISUAL_PSEUDO_COLOR" << endl;
      break;
    case GDK_VISUAL_TRUE_COLOR:
      cout << "GDK_VISUAL_TRUE_COLOR" << endl;
      break;
    case GDK_VISUAL_DIRECT_COLOR:
      cout << "GDK_VISUAL_DIRECT_COLOR" << endl;
      break;
    default:
      cout << "unknown visual type" << endl;
      break;
    }
#endif
}

void
UI_Gtk::init(int *argc, char ***argv)
{
  char *filename, *tmp;

  self = this;
  _shift_lock = false;
  _speed_limit = true;

  _auto_skip = false;
  _cur_auto_skip = 0;
  _max_auto_skip = RC::instance()->get_int("Max Auto Skip", 6);

#ifdef ENABLE_NLS
  /*
   *  We need to get all text in UTF-8 because this is required
   *  for GTK versions above 2.0 (more precisely it's required by
   *  the Pango library).
   *
   *  And we do it here because this way it's possible to get the
   *  help/usage messages still in the default locale of the user.
   *  From this point all messages that are sent to the console are
   *  encoded in UTF-8 too.
   */
  bind_textdomain_codeset(PACKAGE, "UTF-8");
#endif /* ENABLE_NLS */

  gtk_init(argc, argv);

  /*
   *  load gtk ressource files
   */
  filename = new char [strlen(kcemu_datadir) + 14];
  strcpy(filename, kcemu_datadir);
  strcat(filename, "/.kcemurc.gtk");
  gtk_rc_parse(filename);
  delete[] filename;
  
  tmp = kcemu_homedir;
  if (tmp)
    {
      filename = new char[strlen(tmp) + 14];
      strcpy(filename, tmp);
      strcat(filename, "/.kcemurc.gtk");
      gtk_rc_parse(filename);
      delete[] filename;
    }
  else
    cerr << "Warning: HOME not set! can't locate file `.kcemurc.gtk'" << endl;

  /*
   *  don't let gtk catch SIGSEGV, make core dumps ;-)
   */
  signal(SIGSEGV, SIG_DFL);

  _main.statusbar_sec = 0;
  Status::instance()->addStatusListener(this);
  Error::instance()->addErrorListener(this);
  
  _main.window = NULL;

  _about_window       = new AboutWindow();
  _color_window       = new ColorWindow();
  _tape_window        = new TapeWindow();
  _tape_add_window    = new TapeAddWindow();
  _disk_window        = new DiskWindow();
  _module_window      = new ModuleWindow();
  _keyboard_window    = new KeyboardWindow();
  _copying_window     = new CopyingWindow();
  _debug_window       = new DebugWindow();
  _info_window        = new InfoWindow();
  _wav_window         = new WavWindow();
  _edit_header_window = new EditHeaderWindow();
  _dialog_window      = new DialogWindow();
  _file_browser       = new FileBrowser();

  allocate_colors(_color_window->get_saturation_fg(),
		  _color_window->get_saturation_bg(),
		  _color_window->get_brightness_fg(),
		  _color_window->get_brightness_bg(),
		  _color_window->get_black_level(),
		  _color_window->get_white_level());

  /* this _must_ come last due to some initialization for menus */
  create_main_window();

  _visual = gdk_visual_get_system();

  CMD *cmd;
  cmd = new CMD_ui_toggle(this);
  cmd = new CMD_update_colortable(this, _color_window);

  init();
}

void
UI_Gtk::show(void)
{
  _init = true;
  gtk_resize();
  gtk_window_set_policy(GTK_WINDOW(_main.window), FALSE, FALSE, TRUE);
  show_greeting();
}

void
UI_Gtk::gtk_zoom(int zoom)
{
  if (zoom < 1)
    zoom = 1;
  if (zoom > 3)
    zoom = 3;

  kcemu_ui_scale = zoom;

  gtk_resize();
  update(true, true);
}

void
UI_Gtk::processEvents(void)
{
  long tv_sec, tv_usec;

  if (_main.statusbar_sec != 0)
    {
      sys_gettimeofday(&tv_sec, &tv_usec);
      if (tv_sec - _main.statusbar_sec > 10)
        gtk_statusbar_pop(GTK_STATUSBAR(_main.st_statusbar), 1);
    }

  while (gtk_events_pending())
    gtk_main_iteration();
}

#define ADD_COL(weight) \
  r += weight * ((p >> 16) & 0xff); \
  g += weight * ((p >>  8) & 0xff); \
  b += weight * ((p      ) & 0xff); \
  w += weight

/*
 *  +---+---+---+
 *  | 0 | 1 | 2 |
 *  +---+---+---+
 *  | 3 | 4 | 5 |
 *  +---+---+---+
 *  | 6 | 7 | 8 |
 *  +---+---+---+
 */
gulong
UI_Gtk::get_col(byte_t *bitmap, int which, int idx, int width)
{
  gulong p;
  long r, g, b, w;

  w = 0; r = 0; g = 0; b = 0;

  switch (which)
    {
    default:
      return _col[bitmap[idx]].pixel;

    case 1:
      p = _col[bitmap[idx]].pixel;             ADD_COL(9);
      p = _col[bitmap[idx - width]].pixel;     ADD_COL(3);
      break;
    case 3:
      p = _col[bitmap[idx]].pixel;             ADD_COL(9);
      p = _col[bitmap[idx - 1]].pixel;         ADD_COL(3);
      break;
    case 5:
      p = _col[bitmap[idx]].pixel;             ADD_COL(9);
      p = _col[bitmap[idx + 1]].pixel;         ADD_COL(3);
      break;
    case 7:
      p = _col[bitmap[idx]].pixel;             ADD_COL(9);
      p = _col[bitmap[idx + width]].pixel;     ADD_COL(3);
      break;

    case 0:
      p = _col[bitmap[idx]].pixel;             ADD_COL(12);
      p = _col[bitmap[idx - 1]].pixel;         ADD_COL(5);
      p = _col[bitmap[idx - width]].pixel;     ADD_COL(5);
      p = _col[bitmap[idx - width - 1]].pixel; ADD_COL(1);
      break;
    case 2:
      p = _col[bitmap[idx]].pixel;             ADD_COL(12);
      p = _col[bitmap[idx + 1]].pixel;         ADD_COL(5);
      p = _col[bitmap[idx - width]].pixel;     ADD_COL(5);
      p = _col[bitmap[idx - width + 1]].pixel; ADD_COL(1);
      break;
    case 6:
      p = _col[bitmap[idx]].pixel;             ADD_COL(12);
      p = _col[bitmap[idx - 1]].pixel;         ADD_COL(5);
      p = _col[bitmap[idx + width]].pixel;     ADD_COL(5);
      p = _col[bitmap[idx + width - 1]].pixel; ADD_COL(1);
      break;
    case 8:
      p = _col[bitmap[idx]].pixel;             ADD_COL(12);
      p = _col[bitmap[idx + 1]].pixel;         ADD_COL(5);
      p = _col[bitmap[idx + width]].pixel;     ADD_COL(5);
      p = _col[bitmap[idx + width + 1]].pixel; ADD_COL(1);
      break;
    }

  r = r / w;
  g = g / w;
  b = b / w;

  return (r << 16) | (g << 8) | b;
}

static gulong
lighter_color(gulong col)
{
  static int color_add = RC::instance()->get_int("DEBUG UI_Gtk Color Add", 50);

  int r = (col >> 16) & 0xff;
  int g = (col >>  8) & 0xff;
  int b = (col      ) & 0xff;

  r += color_add;
  g += color_add;
  b += color_add;

  if (r > 255)
    r = 255;
  if (g > 255)
    g = 255;
  if (b > 255)
    b = 255;

  return (r << 16) | (g << 8) | b;
}

static gulong
darker_color(gulong col)
{
  int r = (col >> 16) & 0xff;
  int g = (col >>  8) & 0xff;
  int b = (col      ) & 0xff;

  r = (2 * r) / 3;
  g = (2 * g) / 3;
  b = (2 * b) / 3;

  return (r << 16) | (g << 8) | b;
}

void
UI_Gtk::gtk_update_1(byte_t *bitmap, byte_t *dirty, int dirty_size, int width, int height)
{
  int d = -1;
  for (int y = 0;y < height;y += 8)
    {
      for (int x = 0;x < width;x += 8)
	{
	  d++;
	  if (!dirty[d])
	    continue;
	  
	  int z = y * width + x;
	  
	  for (int yy = 0;yy < 8;yy++)
	    {
	      for (int xx = 0;xx < 8;xx++)
		{
		  gdk_image_put_pixel(_image, x + xx, y + yy, _col[bitmap[z + xx]].pixel);
		}
	      z += width;
	    }
	}
    }
}

void
UI_Gtk::gtk_update_1_debug(byte_t *bitmap, byte_t *dirty, int dirty_size, int width, int height)
{
  static int frame_delay;

  if (_dirty_old == 0)
    {
      _dirty_old = new byte_t[dirty_size];
      memset(_dirty_old, 0, dirty_size);
      frame_delay = RC::instance()->get_int("DEBUG UI_Gtk Frame Delay", 50);
    }

  int d = -1;
  for (int y = 0;y < height;y += 8)
    {
      for (int x = 0;x < width;x += 8)
	{
	  d++;
	  if (dirty[d])
	    _dirty_old[d] = frame_delay;
	  
	  if (_dirty_old[d] == 0)
	    continue;

	  if (_dirty_old[d] > 0)
	    _dirty_old[d]--;

	  dirty[d] = 1;
	  
	  int z = y * width + x;

	  if (_dirty_old[d])
	    {
	      for (int yy = 0;yy < 8;yy++)
		{
		  for (int xx = 0;xx < 8;xx++)
		    {
		      gdk_image_put_pixel(_image, x + xx, y + yy, lighter_color(_col[bitmap[z + xx]].pixel));
		    }
		  z += width;
		}
	    }
	  else
	    {
	      for (int yy = 0;yy < 8;yy++)
		{
		  for (int xx = 0;xx < 8;xx++)
		    {
		      gdk_image_put_pixel(_image, x + xx, y + yy, _col[bitmap[z + xx]].pixel);
		    }
		  z += width;
		}
	    }
	}
    }
}

void
UI_Gtk::gtk_update_2(byte_t *bitmap, byte_t *dirty, int dirty_size, int width, int height)
{
  int d = -1;
  for (int y = 0;y < height;y += 8)
    {
      for (int x = 0;x < width;x += 8)
	{
	  d++;
	  if (!dirty[d])
	    continue;
	  
	  int z = y * width + x;
	  
	  for (int yy = 0;yy < 16;yy += 2)
	    {
	      for (int xx = 0;xx < 16;xx += 2)
		{
		  gulong pix = _col[bitmap[z++]].pixel;
		  gdk_image_put_pixel(_image, 2 * x + xx,     2 * y + yy    , pix);
		  gdk_image_put_pixel(_image, 2 * x + xx + 1, 2 * y + yy    , pix);
		  gdk_image_put_pixel(_image, 2 * x + xx    , 2 * y + yy + 1, darker_color(pix));
		  gdk_image_put_pixel(_image, 2 * x + xx + 1, 2 * y + yy + 1, darker_color(pix));
		}
	      z += width - 8;
	    }
	}
    }
}

void
UI_Gtk::gtk_update_3(byte_t *bitmap, byte_t *dirty, int dirty_size, int width, int height)
{
  int d = -1;
  byte_t dirty_buf[dirty_size];

  memcpy(dirty_buf, dirty, dirty_size);

  for (int y = 0;y < height;y += 8)
    {
      for (int x = 0;x < width;x += 8)
	{
	  d++;

	  if (dirty[d])
	    {	  
	      int z = y * width + x;
	  
	      for (int yy = 0;yy < 24;yy += 3)
		{
		  for (int xx = 0;xx < 24;xx += 3)
		    {
		      gdk_image_put_pixel(_image, 3 * x + xx    , 3 * y + yy    , get_col(bitmap, 0, z, width));
		      gdk_image_put_pixel(_image, 3 * x + xx + 1, 3 * y + yy    , get_col(bitmap, 1, z, width));
		      gdk_image_put_pixel(_image, 3 * x + xx + 2, 3 * y + yy    , get_col(bitmap, 2, z, width));
		      gdk_image_put_pixel(_image, 3 * x + xx    , 3 * y + yy + 1, get_col(bitmap, 3, z, width));
		      gdk_image_put_pixel(_image, 3 * x + xx + 1, 3 * y + yy + 1, get_col(bitmap, 4, z, width));
		      gdk_image_put_pixel(_image, 3 * x + xx + 2, 3 * y + yy + 1, get_col(bitmap, 5, z, width));
		      gdk_image_put_pixel(_image, 3 * x + xx    , 3 * y + yy + 2, get_col(bitmap, 6, z, width));
		      gdk_image_put_pixel(_image, 3 * x + xx + 1, 3 * y + yy + 2, get_col(bitmap, 7, z, width));
		      gdk_image_put_pixel(_image, 3 * x + xx + 2, 3 * y + yy + 2, get_col(bitmap, 8, z, width));		      z++;
		    }
		  z += width - 8;
		} 
	    }
	  else
	    {
	      /*
	       *  not dirty but we need to check the neighbour pixels due to
	       *  the antialiasing
	       */
	      if ((d > 0) && (dirty[d - 1]))
		{
		  int z = y * width + x;
		  for (int yy = 0;yy < 24;yy += 3)
		    {
		      gdk_image_put_pixel(_image, 3 * x, 3 * y + yy    , get_col(bitmap, 0, z, width));
		      gdk_image_put_pixel(_image, 3 * x, 3 * y + yy + 1, get_col(bitmap, 3, z, width));
		      gdk_image_put_pixel(_image, 3 * x, 3 * y + yy + 2, get_col(bitmap, 6, z, width));
		      z += width;
		    }
		  dirty_buf[d] = 1;
		}
	      if (dirty[d + 1])
		{
		  int z = y * width + x + 7;
		  for (int yy = 0;yy < 24;yy += 3)
		    {
		      gdk_image_put_pixel(_image, 3 * x + 23, 3 * y + yy    , get_col(bitmap, 2, z, width));
		      gdk_image_put_pixel(_image, 3 * x + 23, 3 * y + yy + 1, get_col(bitmap, 5, z, width));
		      gdk_image_put_pixel(_image, 3 * x + 23, 3 * y + yy + 2, get_col(bitmap, 8, z, width));
		      z += width;
		    }
		  dirty_buf[d] = 1;
		}
	      if (dirty[d + width / 8])
		{
		  int z = (y + 7) * width + x;
		  for (int xx = 0;xx < 24;xx += 3)
		    {
		      gdk_image_put_pixel(_image, 3 * x + xx    , 3 * y + 23, get_col(bitmap, 6, z, width));
		      gdk_image_put_pixel(_image, 3 * x + xx + 1, 3 * y + 23, get_col(bitmap, 7, z, width));
		      gdk_image_put_pixel(_image, 3 * x + xx + 2, 3 * y + 23, get_col(bitmap, 8, z, width));
		      z++;
		    }
		  dirty_buf[d] = 1;
		}
	      if ((d > width / 8) && (dirty[d - width / 8]))
		{
		  int z = y * width + x;
		  for (int xx = 0;xx < 24;xx += 3)
		    {
		      gdk_image_put_pixel(_image, 3 * x + xx    , 3 * y, get_col(bitmap, 0, z, width));
		      gdk_image_put_pixel(_image, 3 * x + xx + 1, 3 * y, get_col(bitmap, 1, z, width));
		      gdk_image_put_pixel(_image, 3 * x + xx + 2, 3 * y, get_col(bitmap, 2, z, width));
		      z++;
		    }
		  dirty_buf[d] = 1;
		}
	    }
	}
    }

  memcpy(dirty, dirty_buf, dirty_size);
}

void
UI_Gtk::gtk_update(byte_t *bitmap, byte_t *dirty, int dirty_size, int width, int height, bool full_update)
{
  switch (kcemu_ui_scale)
    {
    case 1:
      if (kcemu_ui_debug)
	gtk_update_1_debug(bitmap, dirty, dirty_size, width, height);
      else
	gtk_update_1(bitmap, dirty, dirty_size, width, height);
      break;
    case 2:
      gtk_update_2(bitmap, dirty, dirty_size, width, height);
      break;
    case 3:
      gtk_update_3(bitmap, dirty, dirty_size, width, height);
      break;
    }

  if (full_update)
    {
      gdk_draw_image(GTK_WIDGET(_main.canvas)->window, _gc, _image,
		     0, 0, 0, 0, get_width(), get_height());
      return;
    }

  int d = -1;
  int s = 8 * kcemu_ui_scale;
  for (int y = 0;y < get_height();y += s)
    {
      for (int x = 0;x < get_width();x += s)
	{
	  d++;	      
	  if (!dirty[d])
	    continue;

	  dirty[d] = 0;
	  gdk_draw_image(GTK_WIDGET(_main.canvas)->window, _gc, _image, x, y, x, y, s, s);
	}
    }
}

UI_ModuleInterface *
UI_Gtk::getModuleInterface(void)
{
  return _module_window;
}

TapeInterface *
UI_Gtk::getTapeInterface(void)
{
  return _tape_window;
}

DebugInterface *
UI_Gtk::getDebugInterface(void)
{
  return _debug_window;
}

void
UI_Gtk::setStatus(const char *msg)
{
  long tv_sec, tv_usec;

  sys_gettimeofday(&tv_sec, &tv_usec);
  _main.statusbar_sec = tv_sec;
  gtk_statusbar_pop(GTK_STATUSBAR(_main.st_statusbar), 1);
  gtk_statusbar_push(GTK_STATUSBAR(_main.st_statusbar), 1, msg);
}

void
UI_Gtk::errorInfo(const char *msg)
{
  static GtkWidget *w;
  GtkWidget *vbox;
  GtkWidget *label;
  GtkWidget *separator;
  GtkWidget *ok;

  w = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(w), "Info");
  gtk_window_position(GTK_WINDOW(w), GTK_WIN_POS_MOUSE);
  gtk_signal_connect(GTK_OBJECT(w), "destroy",
                     GTK_SIGNAL_FUNC(gtk_widget_destroyed), &w);

  /*
   *  vbox
   */
  vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_border_width(GTK_CONTAINER(vbox), 6);
  gtk_container_add(GTK_CONTAINER(w), vbox);
  gtk_widget_show(vbox);

  /*
   *  label
   */
  label = gtk_label_new(msg);
  gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 6);
  gtk_widget_show(label);

  /*
   *  separator
   */
  separator = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 5);
  gtk_widget_show(separator);

  /*
   *  ok button
   */
  ok = gtk_button_new_with_label("Ok");
  gtk_signal_connect_object(GTK_OBJECT(ok), "clicked",
                            GTK_SIGNAL_FUNC(gtk_widget_destroy),
                            GTK_OBJECT(w));
  gtk_box_pack_start(GTK_BOX(vbox), ok, FALSE, TRUE, 0);
  GTK_WIDGET_SET_FLAGS(ok, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(ok);
  gtk_widget_show(ok);

  /*
   *  show dialog and make it modal
   */
  gtk_widget_show(w);
  gtk_grab_add(w);
}
