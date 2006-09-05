/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: ui_gtk.h,v 1.20 2002/10/31 01:38:07 torsten_paul Exp $
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

#ifndef __ui_ui_gtk_h
#define __ui_ui_gtk_h

#include <gtk/gtk.h>

#ifdef GTK_MAJOR_VERSION
#ifdef GTK_MINOR_VERSION
#if GTK_MINOR_VERSION > 1
#define GTK_HAVE_1_2
#endif
#endif
#endif

#include "ui/ui.h"
#include "ui/errorl.h"
#include "ui/statusl.h"
#include "ui/commands.h"
#include "ui/gtk/cmd.h"

#include "ui/gtk/fbrowse.h"
#include "ui/gtk/dialog.h"
#include "ui/gtk/hedit.h"

#include "cmd/cmd.h"
#include "cmd/cmdargs.h"

class UI_Gtk : public StatusListener, public ErrorListener, public UI
{
 protected:
  enum {
    CANVAS_WIDTH  = 320,
    CANVAS_HEIGHT = 256,
    
    CB_OFFSET = 35000, /* 50 Hz */
  };
  
  bool       _auto_skip;
  int        _cur_auto_skip;
  int        _max_auto_skip;
    
  UI_Gtk_Window *_debug_window;
  UI_Gtk_Window *_about_window;
  UI_Gtk_Window *_help_window;
  UI_Gtk_Window *_thanks_window;
  UI_Gtk_Window *_color_window;
  UI_Gtk_Window *_copying_window;
  UI_Gtk_Window *_tape_window;
  UI_Gtk_Window *_tape_add_window;
  UI_Gtk_Window *_disk_window;
  UI_Gtk_Window *_module_window;
  UI_Gtk_Window *_keyboard_window;
  UI_Gtk_Window *_info_window;
  UI_Gtk_Window *_wav_window;

  FileBrowser      *_file_browser;
  DialogWindow     *_dialog_window;
  EditHeaderWindow *_edit_header_window;

    struct {
      GtkWidget *window;
    } _header;

    struct {
      GtkWidget *window;
      GtkWidget *eventbox;
      GtkWidget *vbox;
      GtkWidget *menubar;
      GtkWidget *menu;
      GtkWidget *menuitem;
      GtkWidget *canvas;
      GtkWidget *st_frame;
      GtkWidget *st_hbox;
      GtkWidget *st_fps;
      GtkWidget *st_statusbar;
      long       statusbar_sec;
      GtkItemFactory *ifact;
      GtkItemFactory *ifactP;
      GtkAccelGroup  *agroup;
      GtkAccelGroup  *agroupP;
    } _main;

    struct {
      GtkWidget *window;
      GtkWidget *canvas;
    } _text;

    GdkGC         *_gc;
    GdkVisual     *_visual;
    GdkImage      *_image;
    GdkColormap   *_colormap;
    GdkColor       _col[24];
    byte_t        *_dirty_old;

    bool           _init;
    bool           _shift_lock;
    bool           _speed_limit;

    void create_main_window(void);
    void create_header_window(void);
    void setup_ui_defaults(void);
    void show_greeting(void);
    void tapeSelect(void);
    void hsv_to_gdk_color(double h, double s, double v, GdkColor *col);
    gulong get_col(byte_t *bitmap, int which, int idx, int width);

    void text_update(void);
    void attach_remote_listener(void);

    static void sf_selection_received(GtkWidget *widget,
                                      GtkSelectionData *sel_data,
                                      gpointer *data);
    static gboolean property_change(GtkWidget *widget,
                                    GdkEventProperty *event,
                                    gpointer data);

    static void key_press_release(GdkEventKey *event, bool press);
    
    static void mc_view_menubar(GtkWidget *widget, gpointer data);
    static void mc_view_tape_window(GtkWidget *widget, gpointer data);
    static void mc_view_tape_add_window(GtkWidget *widget, gpointer data);
    static void mc_view_statusbar(GtkWidget *widget, gpointer data);
    static void mc_tape_remove_file(GtkWidget *widget, gpointer data);
    static void mc_tape_load_file(GtkWidget *widget, gpointer data);
    static void mc_tape_delete_file(GtkWidget *widget, gpointer data);
    static void mc_tape_add_tape_file(GtkWidget *widget, gpointer data);
    
    static void sf_focus_in(GtkWidget *widget, GdkEventFocus *event);
    static void sf_focus_out(GtkWidget *widget, GdkEventFocus *event);

    static gboolean sf_key_press(GtkWidget *widget, GdkEventKey *event);
    static gboolean sf_key_release(GtkWidget *widget, GdkEventKey *event);
    static void sf_leave_notify(GtkWidget *widget, GdkEventCrossing *event);
    static void sf_button_press(GtkWidget *widget, GdkEventButton *event);
    static void sf_tape_button_press(GtkWidget *widget, GdkEventButton *event);
    static void sf_tape_file_selection_ok(GtkWidget *w, GtkFileSelection *fs);
    static void sf_tape_file_select(GtkWidget *widget,
                                    gint row, 
                                    gint column, 
                                    GdkEventButton * bevent);
    static void sf_expose(void);

    static void sf_load(void);
    static void sf_load_ok(GtkWidget *widget, GtkFileSelection *fs);
    static void sf_quit(void);
    static void idle(void);

  public:
    UI_Gtk(void);
    virtual ~UI_Gtk(void);
    virtual void processEvents(void);
    
    virtual void update(bool full_update = false, bool clear_cache = false) = 0;
    virtual void flash(bool enable) = 0;
    virtual int  get_mode(void)     = 0;
    virtual void set_mode(int mode) = 0;

    virtual void init(int *argc, char ***argv);
    virtual void show(void);
    virtual void init(void) = 0; // init subclasses after creation of main window
    virtual const char * get_title(void) = 0;
    virtual int get_width(void) = 0;
    virtual int get_height(void) = 0;

    virtual void status_bar_toggle(void);
    virtual void menu_bar_toggle(void);
    virtual void speed_limit_toggle(void);
    virtual void display_effects_toggle(void);

    virtual void allocate_colors(double saturation_fg,
				 double saturation_bg,
				 double brightness_fg,
				 double brightness_bg,
				 double black_level,
				 double white_level) = 0;

    void gtk_sync(void);
    void gtk_resize(void);
    void gtk_zoom(int zoom);
    void gtk_enable_display_effect(int effect);
    void gtk_update(byte_t *bitmap, byte_t *dirty, int dirty_size, int width, int height, bool full_update);
    void gtk_update_1(byte_t *bitmap, byte_t *dirty, int dirty_size, int width, int height);
    void gtk_update_2(byte_t *bitmap, byte_t *dirty, int dirty_size, int width, int height);
    void gtk_update_3(byte_t *bitmap, byte_t *dirty, int dirty_size, int width, int height);
    void gtk_update_1_debug(byte_t *bitmap, byte_t *dirty, int dirty_size, int width, int height);
    void gtk_update_2_scanline(byte_t *bitmap, byte_t *dirty, int dirty_size, int width, int height);
    void gtk_update_3_smooth(byte_t *bitmap, byte_t *dirty, int dirty_size, int width, int height);

    /*
     *  StatusListener
     */
    void setStatus(const char *msg);

    /*
     *  ErrorListener
     */
    void errorInfo(const char *msg);

    /*
     *  interface handling
     */
    virtual UI_ModuleInterface * getModuleInterface(void);
    virtual TapeInterface  * getTapeInterface(void);
    virtual DebugInterface * getDebugInterface(void);

    friend class KeyboardWindow; // allow KeyboardWindow to call event handlers
};

#endif /* __ui_ui_gtk_h */

