/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: ui_gtk.h,v 1.16 2001/04/14 15:15:33 tp Exp $
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
#include "ui/gtk/tape.h"
#include "ui/gtk/disk.h"
#include "ui/gtk/info.h"
#include "ui/gtk/about.h"
#include "ui/gtk/debug.h"
#include "ui/gtk/color.h"
#include "ui/gtk/module.h"
#include "ui/gtk/copying.h"
#include "ui/gtk/tapeadd.h"
#include "ui/gtk/fbrowse.h"
#include "ui/gtk/dialog.h"
#include "ui/gtk/hedit.h"

#include "cmd/cmd.h"
#include "cmd/cmdargs.h"

class UI_Gtk : public UI, public StatusListener, public ErrorListener
{
 protected:
  enum {
    CANVAS_WIDTH  = 320,
    CANVAS_HEIGHT = 256,
    
    CB_OFFSET = 35000, /* 50 Hz */
    
    PROFILE_HIST_LEN = 256,
  };
    
    DebugWindow      *_debug_window;
    AboutWindow      *_about_window;
    ColorWindow      *_color_window;
    CopyingWindow    *_copying_window;
    TapeWindow       *_tape_window;
    TapeAddWindow    *_tape_add_window;
    DiskWindow       *_disk_window;
    ModuleWindow     *_module_window;
    InfoWindow       *_info_window;
    EditHeaderWindow *_edit_header_window;
    DialogWindow     *_dialog_window;
    FileBrowser      *_file_browser;

    struct {
      GtkWidget *window;
    } _header;

    struct {
      GtkWidget *window;
      GtkWidget *d_area;
    } _profile;

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
    } _main;

    GdkGC         *_gc;
    GdkVisual     *_visual;
    GdkImage      *_image;
    GdkImage      *_imageb;
    GdkColormap   *_colormap;
    GdkColor       _col[24];

    bool           _shift_lock;
    bool           _flash;
    char           _dirty_buf[8192];
    char          *_dirty;
    unsigned char *_pix_mem;
    unsigned char *_col_mem;
    unsigned int   _profile_hist_ptr;
    unsigned int   _profile_hist[PROFILE_HIST_LEN];

    void create_main_window(void);
    void create_header_window(void);
    void create_profile_window(void);
    void tapeSelect(void);
    
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

    static void sf_key_press(GtkWidget *widget, GdkEventKey *event);
    static void sf_key_release(GtkWidget *widget, GdkEventKey *event);
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
    static void sf_profile_event(GtkWidget *widget, GdkEventButton *event);

  public:
    UI_Gtk(void);
    virtual ~UI_Gtk(void);
    virtual void processEvents(void);
    
    virtual void update(bool full_update = false, bool clear_cache = false) = 0;
    virtual void memWrite(int addr, char val) = 0;
    virtual void callback(void *data) = 0;
    virtual void flash(bool enable) = 0;

    virtual void profile_mem_access(int addr, pf_type type);

    virtual void init(int *argc, char ***argv);
    virtual const char * get_title(void) = 0;
    virtual int get_width(void) = 0;
    virtual int get_height(void) = 0;

    virtual void status_bar_toggle(void);
    virtual void menu_bar_toggle(void);

    virtual void allocate_colors(double saturation_fg,
				 double saturation_bg,
				 double brightness_fg,
				 double brightness_bg,
				 double black_level,
				 double white_level) = 0;
    
    virtual UI_ModuleInterface * getModuleInterface(void);
    virtual TapeInterface  * getTapeInterface(void);
    virtual DebugInterface * getDebugInterface(void);
    
    /*
     *  StatusListener
     */
    void setStatus(const char *msg);

    /*
     *  ErrorListener
     */
    void errorInfo(const char *msg);
};

#endif /* __ui_ui_gtk_h */

