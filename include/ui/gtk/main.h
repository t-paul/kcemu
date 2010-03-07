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

#ifndef __ui_gtk_main_h
#define __ui_gtk_main_h

#include "kc/system.h"

#include "ui/statusl.h"
#include "ui/hsv2rgb.h"

#include "ui/generic/ui_base.h"

#include "ui/gtk/window.h"

using namespace std;

class MainWindow : public StatusListener, public UI_Gtk_Window {
private:
    struct {
        GtkDrawingArea *drawingarea;

        GtkMenu        *popup_menu;
        
        GtkMenuBar     *menubar;
        GtkHBox        *status_hbox;
        GtkLabel       *status_label;
        GtkStatusbar   *status_statusbar;

        GtkAccelGroup  *accel_group;

        guint           idle_id;
        long            status_sec;
    } _w;
    
    int           _width;
    int           _height;

    bool          _expose;
    GdkGC         *_gc;
    GdkVisual     *_visual;
    GdkImage      *_image;
    byte_t        *_dirty_old;
    GdkColormap   *_colormap;
    GdkColor       _col[24];
    
    CMD           *_cmd_ui_toggle;
    
    string        _accel_map_path;
    
protected:
    void init(void);
    void init_icon(void);
    void add_menu_accel_group(const char *base, const char *name);
    void wire_menu_item(const char *name, const char *shortcut, const char *command);

    gulong get_col(byte_t *bitmap, int which, int idx, int width);
    void update_1(byte_t *bitmap, byte_t *dirty, int dirty_size, int width, int height);
    void update_2(byte_t *bitmap, byte_t *dirty, int dirty_size, int width, int height);
    void update_3(byte_t *bitmap, byte_t *dirty, int dirty_size, int width, int height);
    void update_1_debug(byte_t *bitmap, byte_t *dirty, int dirty_size, int width, int height);
    void update_2_scanline(byte_t *bitmap, byte_t *dirty, int dirty_size, int width, int height);
    void update_3_smooth(byte_t *bitmap, byte_t *dirty, int dirty_size, int width, int height);
    
    void attach_remote_listener(void);

    static gboolean on_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
    static gboolean on_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
    static void on_accel_group_changed(GtkAccelGroup *accel_group, guint keyval, GdkModifierType modifier, GClosure *accel_closure, gpointer user_data);
    static gboolean on_accel_group_changed_idle_func(gpointer data);
    static gboolean on_property_change(GtkWidget *widget, GdkEventProperty *event, gpointer data);
    
public:
    MainWindow(const char *ui_xml_file);
    virtual ~MainWindow(void);

    virtual GtkWidget * get_main_window(void);
    
    virtual void process_events(void);
    virtual void show(int width, int height);
    virtual bool resize(int width, int height);
    virtual void update(UI_Base *ui, int image_width, int image_height, bool full_update);
    virtual void allocate_color_rgb(int idx, int r, int g, int b);

    virtual gboolean get_display_effect(void);
    virtual void set_display_effect(gboolean effect);

    virtual void set_fps(unsigned long fps);
    virtual void status_bar_toggle(void);
    virtual void menu_bar_toggle(void);
    
    virtual GdkColor * get_colormap(void);
    /*
     *  StatusListener
     */
    virtual void setStatus(const char *msg);
};

#endif /* __ui_gtk_main_h */

