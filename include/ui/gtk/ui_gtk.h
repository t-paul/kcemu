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

#ifndef __ui_ui_gtk_h
#define __ui_ui_gtk_h

#include <gtk/gtk.h>

#include "ui/ui.h"
#include "ui/errorl.h"
#include "ui/commands.h"

#include "ui/generic/ui_base.h"
#include "ui/gtk/ui_gtk_base.h"

#include "ui/gtk/cmd.h"
#include "ui/gtk/fbrowse.h"
#include "ui/gtk/dialog.h"
#include "ui/gtk/hedit.h"
#include "ui/gtk/videoenc.h"

#include "cmd/cmd.h"
#include "cmd/cmdargs.h"

class MainWindow;

class UI_Gtk : public ErrorListener, public UI {
protected:
    enum {
        CB_OFFSET = 35000, /* 50 Hz */
    };
    
    bool       _auto_skip;
    int        _cur_auto_skip;
    int        _max_auto_skip;
    
    int        _video_skip;
    int        _video_frame;

    int        _video_encoder_state;
    
    unsigned long long _callback_value;
    unsigned long long _callback_value_retrace;
    
    UI_Gtk_Base *_ui;

    MainWindow *_main_window;

    VideoEncoder *_video_encoder;
    VideoEncoder *_video_encoder_dummy;
    const char   *_video_encoder_filename;
    double        _video_encoder_quality;
    int           _video_encoder_frame_skip;
    bool          _video_encoder_start_on_reset;

    UI_Gtk_Window *_debug_window;
    UI_Gtk_Window *_about_window;
    UI_Gtk_Window *_help_window;
    UI_Gtk_Window *_thanks_window;
    UI_Gtk_Window *_color_window;
    UI_Gtk_Window *_copying_window;
    UI_Gtk_Window *_options_window;
    UI_Gtk_Window *_selector_window;
    UI_Gtk_Window *_tape_window;
    UI_Gtk_Window *_tape_add_window;
    UI_Gtk_Window *_disk_window;
    UI_Gtk_Window *_vdip_window;
    UI_Gtk_Window *_module_window;
    UI_Gtk_Window *_keyboard_window;
    UI_Gtk_Window *_info_window;
    UI_Gtk_Window *_wav_window;
    UI_Gtk_Window *_screenshot_window;
    UI_Gtk_Window *_plotter_window;
    UI_Gtk_Window *_save_memory_window;
    UI_Gtk_Window *_video_window;
    
    FileBrowser      *_file_browser;
    DialogWindow     *_dialog_window;
    EditHeaderWindow *_edit_header_window;
    
    bool           _init;
    bool           _shift_lock;
    bool           _speed_limit;

    void create_ui(void);
    void destroy_ui(void);
    void add_callback(void);
    int ensure_range(double val);
    
    void tapeSelect(void);
    
    void text_update(void);
    void wire_menu_item(const char *name, const char *shortcut, const char *command);

    static void on_pixbuf_destroy(guchar *pixels, gpointer user_data);
    
    static void sf_selection_received(GtkWidget *widget,
            GtkSelectionData *sel_data,
            gpointer *data);
    
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
    
    static void sf_load(void);
    static void sf_load_ok(GtkWidget *widget, GtkFileSelection *fs);
    static void sf_quit(void);
    static void idle(void);
    
public:
    UI_Gtk(void);
    virtual ~UI_Gtk(void);
    virtual void processEvents(void);

    virtual void update(bool full_update = false, bool clear_cache = false);
    virtual void flash(bool enable);
    virtual int  get_mode(void);
    virtual void set_mode(int mode);
    
    virtual void init(int *argc, char ***argv);
    virtual void init2(void);
    virtual void show(void);
    virtual int get_width(void);
    virtual int get_height(void);
    virtual void memory_read(word_t addr);
    virtual void memory_write(word_t addr);
    virtual void callback(void *data);

    virtual char *select_profile(void);

    virtual GdkPixbuf * get_screenshot(void);
    
    virtual void speed_limit_toggle(void);
    virtual void display_effects_toggle(void);
    
    virtual void allocate_colors(double brightness_fg, double contrast_fg, double brightness_bg, double contrast_bg);

    virtual void set_video_encoder(VideoEncoder *encoder);
    virtual void set_video_encoder_state(int state);
    virtual void set_video_encoder_config(const char *filename, double quality, int frame_skip, bool start_on_reset);

    void gtk_sync(void);
    void gtk_resize(void);
    void gtk_zoom(int zoom);

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

    virtual void reset(bool power_on = false);
};

#endif /* __ui_ui_gtk_h */

