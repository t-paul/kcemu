/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-1998 Torsten Paul
 *
 *  $Id: ui_gtk.cc,v 1.17 2001/01/05 18:25:10 tp Exp $
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

#include <iostream.h>
#include <iomanip.h>

#include <signal.h> /* FIXME: only for testing */
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include <X11/Xatom.h>

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>

#include "kc/config.h"
#include "kc/system.h"

#include "kc/z80.h"
#include "kc/keys.h"
#include "kc/keyboard.h"

#include "cmd/cmd.h"

/*  hmm, Xlib.h defines Status as int :-( */
#undef Status
#include "ui/error.h"
#include "ui/status.h"
#include "ui/gtk/cmd.h"
//#include "ui/gtk/cmds.h"
#include "ui/gtk/debug.h"
#include "ui/gtk/ui_gtk.h"

#include "libdbg/dbg.h"

static UI_Gtk *self;

// #define UPDATE_PROFILING

#ifdef UPDATE_PROFILING
static long __update_calls = 0;
static long __update_calls_dirty = 0;
#endif /* UPDATE_PROFILING */

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
	}
    }
};

void
UI_Gtk::attach_remote_listener(void)
{
  GdkAtom atom;

  atom = gdk_atom_intern("_KCEMU_REMOTE_COMMAND", FALSE);
  gdk_property_change(_main.window->window,
                      atom, XA_STRING, 8, GDK_PROP_MODE_REPLACE,
                      (unsigned char *)"", 1);
  gdk_flush();
}

gboolean
UI_Gtk::property_change(GtkWidget *widget,
                        GdkEventProperty *event,
                        gpointer data)
{
  char *ptr, *val;
  guchar *prop_data;
  GdkAtom actual_property_type;
  gint    actual_format, actual_length;
  CMD_Args *args;

  UI_Gtk *self = (UI_Gtk *)data;

  if (strcmp(gdk_atom_name(event->atom), "_KCEMU_REMOTE_COMMAND") == 0)
    {
      DBG(1, form("KCemu/UI/remote",
                  "property_change: %s\n",
                  gdk_atom_name(event->atom)));
      gdk_property_get(self->_main.window->window,
                       event->atom, XA_STRING,
                       0, (65536 / sizeof(long)), FALSE,
                       &actual_property_type,
                       &actual_format, &actual_length,
                       &prop_data);
      if (*prop_data == '\0')
        {
          DBG(1, form("KCemu/UI/remote",
                      "empty property!\n"));
          return TRUE;
        }
      
      ptr = (char *)prop_data;
      DBG(1, form("KCemu/UI/remote",
                  " command: %s'\n",
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

  key_code = XKeysymToKeycode(GDK_DISPLAY(), event->keyval);

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
    default:
      c = event->keyval & 0xff;
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

void
UI_Gtk::sf_key_press(GtkWidget */*widget*/, GdkEventKey *event)
{
  DBG(2, form("KCemu/UI/key_press",
              "key_press:   keyval = %04x, keycode = %04x\n",
              event->keyval, XKeysymToKeycode(GDK_DISPLAY(), event->keyval)));

  key_press_release(event, true);
}

void
UI_Gtk::sf_key_release(GtkWidget */*widget*/, GdkEventKey *event)
{
  DBG(2, form("KCemu/UI/key_release",
              "key_release: keyval = %04x, keycode = %04x\n",
              event->keyval, XKeysymToKeycode(GDK_DISPLAY(), event->keyval)));

  key_press_release(event, false);
}

void
UI_Gtk::sf_focus_in(GtkWidget * /* widget */, GdkEventFocus *event)
{
  DBG(2, form("KCemu/UI/focus_in",
              "got focus\n"));

  keyboard->keyReleased(-1, -1);
  gdk_key_repeat_disable();
}

void
UI_Gtk::sf_focus_out(GtkWidget *widget, GdkEventFocus *event)
{
  DBG(2, form("KCemu/UI/focus_out",
              "lost focus\n"));

  keyboard->keyReleased(-1, -1);
  gdk_key_repeat_restore();
}

void
UI_Gtk::sf_leave_notify(GtkWidget *widget, GdkEventCrossing *event)
{
  // cerr.form("UI_Gtk::sf_leave_notify()\n");
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

/*
 *  define some ungly typecasts to suppress some
 *  compiler warnings :-(
 */
#define CF(func)   ((GtkItemFactoryCallback)(func))
#define CD(string) ((unsigned int)(string))

void
UI_Gtk::create_main_window(void)
{
  GtkItemFactory *ifact, *ifactP;
  GtkAccelGroup *agroup, *agroupP;
  GtkItemFactoryEntry entries[] = {
    { _("/_File"),              NULL,     NULL,            0,                               "<Branch>" },
    { _("/File/Run..."),        NULL,     CF(cmd_exec_mc), CD("kc-image-run"),              NULL },
    { _("/File/Load..."),       "<alt>L", CF(cmd_exec_mc), CD("kc-image-load"),             NULL },
    { _("/File/Tape..."),       "<alt>T", CF(cmd_exec_mc), CD("ui-tape-window-toggle"),     NULL },
    { _("/File/Disk..."),       "<alt>D", CF(cmd_exec_mc), CD("ui-disk-window-toggle"),     NULL },
    { _("/File/Module..."),     "<alt>M", CF(cmd_exec_mc), CD("ui-module-window-toggle"),   NULL },
    { _("/File/sep1"),          NULL,     NULL,            0,                               "<Separator>" },
    { _("/File/Reset"),         "<alt>R", CF(cmd_exec_mc), CD("emu-reset"),                 NULL },
    { _("/File/Power On"),      NULL,     CF(cmd_exec_mc), CD("emu-power-on"),              NULL },
    { _("/File/sep2"),          NULL,     NULL,            0,                               "<Separator>" },
    { _("/File/Quit Emulator"), "<alt>Q", CF(cmd_exec_mc), CD("emu-quit"),                  NULL },
    { _("/_View"),              NULL,     NULL,            0,                               "<Branch>" },
    { _("/View/Debugger"),      NULL,     CF(cmd_exec_mc), CD("ui-debug-window-toggle"),    NULL },
    { _("/View/Info"),          "<alt>I", CF(cmd_exec_mc), CD("ui-info-window-toggle"),     NULL },
    { _("/View/Menubar"),       NULL,     CF(cmd_exec_mc), CD("ui-menu-bar-toggle"),        NULL },
    { _("/View/Statusbar"),     NULL,     CF(cmd_exec_mc), CD("ui-status-bar-toggle"),      NULL },
    { _("/_Help"),              NULL,     NULL,            0,                               "<LastBranch>" },
    { _("/Help/About KCemu"),   NULL,     CF(cmd_exec_mc), CD("ui-about-window-toggle"),    NULL },
    { _("/Help/sep3"),          NULL,     NULL,            0,                               "<Separator>" },
    { _("/Help/KCemu Licence"), NULL,     CF(cmd_exec_mc), CD("ui-copying-window-toggle"),  NULL },
    { _("/Help/No Warranty!"),  NULL,     CF(cmd_exec_mc), CD("ui-warranty-window-toggle"), NULL },
  };
  GtkItemFactoryEntry entriesP[] = {
    { _("/_Run..."),            NULL,     CF(cmd_exec_mc), CD("kc-image-run"),              NULL },
    { _("/_Load..."),           NULL,     CF(cmd_exec_mc), CD("kc-image-load"),             NULL },
    { _("/_Tape..."),           NULL,     CF(cmd_exec_mc), CD("ui-tape-window-toggle"),     NULL },
    { _("/_Disk..."),           NULL,     CF(cmd_exec_mc), CD("ui-disk-window-toggle"),     NULL },
    { _("/_Module..."),         NULL,     CF(cmd_exec_mc), CD("ui-module-window-toggle"),   NULL },
    { _("/sep1"),               NULL,     NULL,            0,                               "<Separator>" },
    { _("/_View"),              NULL,     NULL,            0,                               "<Branch>" },
    { _("/View/Debugger"),      NULL,     NULL,            0,                               NULL },
    { _("/View/Info"),          NULL,     NULL,            0,                               NULL },
    { _("/View/Menubar"),       NULL,     CF(cmd_exec_mc), CD("ui-menu-bar-toggle"),        NULL },
    { _("/View/Statusbar"),     NULL,     CF(cmd_exec_mc), CD("ui-status-bar-toggle"),      NULL },
    { _("/sep2"),               NULL,     NULL,            0,                               "<Separator>" },
    { _("/Reset"),              NULL,     CF(cmd_exec_mc), CD("emu-reset"),                 NULL },
    { _("/Power On"),           NULL,     CF(cmd_exec_mc), CD("emu-power-on"),              NULL },
    { _("/sep3"),               NULL,     NULL,            0,                               "<Separator>" },
    { _("/Quit Emulator"),      NULL,     CF(cmd_exec_mc), CD("emu-quit"),                  NULL },
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
  gtk_widget_set_uposition(_main.window, 650, 50);
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
  agroup = gtk_accel_group_new();
  ifact = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<KCemu>", agroup);
  gtk_item_factory_create_items(ifact, nentries, entries, NULL);
  gtk_accel_group_attach(agroup, GTK_OBJECT(_main.window));

  agroupP = gtk_accel_group_new();
  ifactP = gtk_item_factory_new(GTK_TYPE_MENU, "<KCemuP>", agroupP);
  gtk_item_factory_create_items(ifactP, nentriesP, entriesP, NULL);

  /*
   *  main menu bar
   */
  _main.menubar = gtk_item_factory_get_widget(ifact, "<KCemu>");
  gtk_box_pack_start(GTK_BOX(_main.vbox), _main.menubar, FALSE, TRUE, 0);
  gtk_widget_show(_main.menubar);

  /*
   *  popup menu
   */
  _main.menu = gtk_item_factory_get_widget(ifactP, "<KCemuP>");

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

  _main.st_fps = gtk_label_new("");
  gtk_container_add(GTK_CONTAINER(_main.st_frame), _main.st_fps);
  gtk_widget_show(_main.st_fps);

  /*
   *  statusbar
   */
  _main.st_statusbar = gtk_statusbar_new();
  gtk_box_pack_start(GTK_BOX(_main.st_hbox), _main.st_statusbar,
                     TRUE, TRUE, 0);
  gtk_widget_show(_main.st_statusbar);

  /*
   *
   */
  gtk_widget_show(_main.window);

  gtk_window_set_policy(GTK_WINDOW(_main.window), FALSE, FALSE, TRUE);
}

void
UI_Gtk::create_header_window(void)
{
  _header.window = gtk_window_new(GTK_WINDOW_DIALOG);
  gtk_window_set_title(GTK_WINDOW(_header.window), "Edit Header");

  gtk_widget_show(_header.window);
}

void
UI_Gtk::sf_profile_event(GtkWidget * /* widget */, GdkEventButton *event)
{
  int x = (int)event->x;
  int y = (int)event->y;
  int addr = ((y / 2) << 8) | (x / 2);
  cerr.form("sf_profile_event -> [%d, %d] => %04x\n", x, y, addr);
}

void
UI_Gtk::create_profile_window(void)
{
  _profile.window = gtk_window_new(GTK_WINDOW_DIALOG);
  gtk_window_set_title(GTK_WINDOW(_profile.window), "Profiling...");

  _profile.d_area = gtk_drawing_area_new();
  gtk_drawing_area_size(GTK_DRAWING_AREA(_profile.d_area), 512, 512);
  gtk_widget_set_events(_profile.d_area, GDK_BUTTON_PRESS_MASK);
  gtk_signal_connect(GTK_OBJECT(_profile.d_area), "button_press_event",
                     GTK_SIGNAL_FUNC(UI_Gtk::sf_profile_event), NULL);
  
  gtk_container_add(GTK_CONTAINER(_profile.window), _profile.d_area);
  gtk_widget_show(_profile.d_area);
  gtk_widget_show(_profile.window);
}

UI_Gtk::UI_Gtk(void)
{
  z80->register_ic(this);
}

UI_Gtk::~UI_Gtk(void)
{
  z80->unregister_ic(this);
  gtk_widget_destroy(_main.window);
  gdk_key_repeat_restore();
        
#ifdef UPDATE_PROFILING
  cerr.form("UI_Gtk: update calls total: %ld\n", __update_calls);
  cerr.form("UI_Gtk: update calls dirty: %ld\n", __update_calls_dirty);
  cerr.form("UI_Gtk: update ratio      : %.2f%%\n",
            100.0 * (float)__update_calls_dirty / (float)__update_calls);
#endif /* UPDATE_PROFILING */

  delete _about_window;
  delete _tape_window;
  delete _tape_add_window;
  delete _module_window;
  delete _copying_window;
  delete _debug_window;
  delete _info_window;
  delete _edit_header_window;
  delete _file_browser;
}

void
UI_Gtk::init(int *argc, char ***argv)
{
  int a;
  char *filename, *tmp;

  self = this;
  _flash = false;
  _shift_lock = false;

  gtk_set_locale();
  gtk_init(argc, argv);

  /*
   *  load gtk ressource files
   */
  filename = new char [strlen(kcemu_datadir) + 14];
  strcpy(filename, kcemu_datadir);
  strcat(filename, "/.kcemurc.gtk");
  gtk_rc_parse(filename);
  delete filename;
  
  tmp = getenv("HOME");
  if (tmp)
    {
      filename = new char[strlen(tmp) + 14];
      strcpy(filename, tmp);
      strcat(filename, "/.kcemurc.gtk");
      gtk_rc_parse(filename);
      delete filename;
    }
  else
    cerr << "Warning: HOME not set! can't locate file `.kcemurc.gtk'" << endl;

  /*
   *  don't let gtk catch SIGSEGV, make core dumps ;-)
   */
  signal(SIGSEGV, SIG_DFL);

  gdk_key_repeat_disable();

  _main.statusbar_sec = 0;
  Status::instance()->addStatusListener(this);
  Error::instance()->addErrorListener(this);
  
  for (a = 0;a < 4096;a++) {
	  _dirty_buf[a] = 1;
  }
  _dirty = &_dirty_buf[0x800];

  _pix_mem = (unsigned char *)g_malloc(CANVAS_WIDTH * CANVAS_HEIGHT / 8);
  _col_mem = (unsigned char *)g_malloc(CANVAS_WIDTH * CANVAS_HEIGHT / 8);

  _main.window = NULL;

  _about_window       = new AboutWindow();
  _tape_window        = new TapeWindow();
  _tape_add_window    = new TapeAddWindow();
  _disk_window        = new DiskWindow();
  _module_window      = new ModuleWindow();
  _copying_window     = new CopyingWindow();
  _debug_window       = new DebugWindow();
  _info_window        = new InfoWindow();
  _edit_header_window = new EditHeaderWindow();
  _dialog_window      = new DialogWindow();
  _file_browser       = new FileBrowser();

  allocate_colors();
#ifdef PROFILE_WINDOW
  create_profile_window();
#endif
  /* this _must_ come last due to some initialization for menus */
  create_main_window();

  //UI_Cmds::init();

  _profile_hist_ptr = 0;

  _visual = gdk_visual_get_system();
  _image  = gdk_image_new(GDK_IMAGE_FASTEST, _visual, CANVAS_WIDTH, CANVAS_HEIGHT);
  _imageb = gdk_image_new(GDK_IMAGE_FASTEST, _visual, CANVAS_WIDTH, CANVAS_HEIGHT);
  _gc = gdk_gc_new(GTK_WIDGET(_main.canvas)->window);

  CMD *cmd;
  cmd = new CMD_ui_toggle(this);

  Status::instance()->setMessage(" KCemu v" VERSION);
}

void
UI_Gtk::processEvents(void)
{
  struct timeval tv;

  if (_main.statusbar_sec != 0)
    {
      gettimeofday(&tv, NULL);
      if (tv.tv_sec - _main.statusbar_sec > 10)
        gtk_statusbar_pop(GTK_STATUSBAR(_main.st_statusbar), 1);
    }

  while (gtk_events_pending())
    gtk_main_iteration();
}

void
UI_Gtk::profile_mem_access(int addr, pf_type type)
{
  int x;
  int y;
  GdkGC *gc;
  GtkStyle *style;

  if (type != PF_MEM_EXEC) return;
        
  style = _profile.d_area->style;

  gc = style->bg_gc[GTK_STATE_NORMAL];
  x = _profile_hist[_profile_hist_ptr] & 0xff;
  y = (_profile_hist[_profile_hist_ptr] >> 8) & 0xff;
  /*
    gdk_draw_point(_profile.d_area->window, gc, 2 * x    , 2 * y    );
    gdk_draw_point(_profile.d_area->window, gc, 2 * x + 1, 2 * y    );
    gdk_draw_point(_profile.d_area->window, gc, 2 * x    , 2 * y + 1);
    gdk_draw_point(_profile.d_area->window, gc, 2 * x + 1, 2 * y + 1);
    */

  _profile_hist[_profile_hist_ptr++] = addr;
  if (_profile_hist_ptr >= PROFILE_HIST_LEN) _profile_hist_ptr = 0;
  
  gc = style->fg_gc[GTK_STATE_NORMAL];
  x = addr & 0xff;
  y = (addr >> 8) & 0xff;
  gdk_draw_point(_profile.d_area->window, gc, 2 * x    , 2 * y    );
  gdk_draw_point(_profile.d_area->window, gc, 2 * x + 1, 2 * y    );
  gdk_draw_point(_profile.d_area->window, gc, 2 * x    , 2 * y + 1);
  gdk_draw_point(_profile.d_area->window, gc, 2 * x + 1, 2 * y + 1);
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
  struct timeval tv;

  gettimeofday(&tv, NULL);
  _main.statusbar_sec = tv.tv_sec;
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

  w = gtk_window_new(GTK_WINDOW_DIALOG);
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
