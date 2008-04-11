/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2008 Torsten Paul
 *
 *  $Id$
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

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/prefs/prefs.h"

#undef Status
#include "ui/status.h"

#include "ui/gtk/cmd.h"
#include "ui/gtk/main.h"

#include "libdbg/dbg.h"

class CMD_toggle_main : public CMD {
private:
    MainWindow *_w;
    
public:
    CMD_toggle_main(MainWindow *w) : CMD("ui-toggle-main") {
        _w = w;
        register_cmd("ui-status-bar-toggle", 0);
        register_cmd("ui-menu-bar-toggle", 1);
    }
    
    void execute(CMD_Args *args, CMD_Context context) {
        switch (context) {
            case 0:
                _w->status_bar_toggle();
                break;
            case 1:
                _w->menu_bar_toggle();
                break;
        }
    }
};

MainWindow::MainWindow(const char *glade_xml_file) : UI_Gtk_Window(glade_xml_file) {
    _gc = NULL;
    _image = NULL;
    _colormap = NULL;
    _dirty_old = NULL;
    _visual = gdk_visual_get_system();
    
    _width = 0;
    _height = 0;
    
    _expose = false;
    _w.status_sec = 0;
    
    _cmd_ui_toggle = new CMD_toggle_main(this);
    
    _accel_map_path = string(kcemu_homedir) + "/keys.map";
    
#if 0
    switch (_visual->type) {
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

MainWindow::~MainWindow(void) {
    delete _cmd_ui_toggle;
}

gboolean
MainWindow::on_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) {
    static int x = 1;
    MainWindow *self = (MainWindow *)user_data;
    
    self->_expose = true;

    if (x) {
        x = 0;
        
        //self->attach_remote_listener();
        Status::instance()->addStatusListener(self);
        
        string greeting("KCemu v" KCEMU_VERSION " (");
        greeting += Preferences::instance()->get_kc_variant_name();
        greeting += ")";
        Status::instance()->setMessage(greeting.c_str());
    }
    
    return FALSE; /* propagate event */
}

gboolean
MainWindow::on_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    MainWindow *self = (MainWindow *)user_data;
    
    switch (event->button) {
        case 2: {
            /* hmm, is TEXT the correct selection type ??? */
            GdkAtom atom = gdk_atom_intern("TEXT", FALSE);
            if (atom == GDK_NONE)
                break;
            gtk_selection_convert(self->_window, GDK_SELECTION_PRIMARY, atom, GDK_CURRENT_TIME);
            break;
        }
        case 3: {
            gtk_menu_popup(GTK_MENU(self->_w.popup_menu), NULL, NULL, NULL, NULL, 3, event->time);
            break;
        }
    }
    
    return FALSE; /* propagate event */
}

void
MainWindow::on_accel_group_changed(GtkAccelGroup *accel_group, guint keyval, GdkModifierType modifier, GClosure *accel_closure, gpointer user_data) {
    MainWindow *self = (MainWindow *)user_data;
    if (self->_w.idle_id == 0) {
        // prevent multiple saves as a single user change might cause many signals
        self->_w.idle_id = g_idle_add(on_accel_group_changed_idle_func, self);
    }
}

gboolean
MainWindow::on_accel_group_changed_idle_func(gpointer data) {
    MainWindow *self = (MainWindow *)data;
    gtk_accel_map_save(self->_accel_map_path.c_str());
    self->_w.idle_id = 0;
    return FALSE; /* remove from idle list */
}

GtkWidget *
MainWindow::get_main_window(void) {
    return _window;
}

void
MainWindow::wire_menu_item(const char *name, const char *shortcut, const char *command) {
    GtkMenuItem *item = GTK_MENU_ITEM(get_widget(name));
    
    GtkWidget *parent = gtk_widget_get_parent(GTK_WIDGET(item));
    const char *parent_name = gtk_widget_get_name(parent);
    string path = string("<KCemu>/") + parent_name + "/" + name;

    gtk_menu_item_set_accel_path(item, path.c_str());
    g_signal_connect(item, "activate", G_CALLBACK(cmd_exec_mc), (gpointer)(command));

    string popup_name = string(name) + "_p";
    GtkMenuItem *popup_item = GTK_MENU_ITEM(get_widget_or_null(popup_name.c_str()));
    if (popup_item != NULL) {
    gtk_menu_item_set_accel_path(popup_item, path.c_str());
        g_signal_connect(popup_item, "activate", G_CALLBACK(cmd_exec_mc), (gpointer)(command));
    }

    if (shortcut != NULL) {
        guint accel_key;
        GdkModifierType accel_mods;
        gtk_accelerator_parse(shortcut, &accel_key, &accel_mods);

        gtk_accel_map_add_entry(path.c_str(), accel_key, accel_mods);
    }
}

void
MainWindow::add_menu_accel_group(const char *base, const char *name) {
    string path = string(base) + "/" + name;
    GtkMenu *menu = GTK_MENU(get_widget(name));
    gtk_menu_set_accel_group(menu, _w.accel_group);
    //gtk_menu_set_accel_path(menu, path.c_str());
}

gboolean
MainWindow::get_display_effect(void) {
    return kcemu_ui_display_effect;
}

void
MainWindow::set_display_effect(gboolean effect) {
    GtkWidget *widget = get_widget("menuitem_display_effects");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(widget), effect);
    kcemu_ui_display_effect = effect;
}

void
MainWindow::init(void) {
    _window = get_widget("main_window");
    g_signal_connect(_window, "delete_event", G_CALLBACK(cmd_exec_sft), (gpointer)"emu-quit");
    
    _w.accel_group = gtk_accel_group_new();
    gtk_window_add_accel_group(GTK_WINDOW(_window), _w.accel_group);

    init_icon();

    string title(Preferences::instance()->get_kc_type_name());
    title += " Emulator";
    gtk_window_set_title(GTK_WINDOW(_window), title.c_str());

    _w.menubar = GTK_MENU_BAR(get_widget("menubar"));
    _w.status_hbox = GTK_HBOX(get_widget("status_hbox"));
    
    _w.drawingarea = GTK_DRAWING_AREA(get_widget("drawingarea"));
    _w.popup_menu = GTK_MENU(get_widget("menu_popup"));
    
    _w.status_label = GTK_LABEL(get_widget("status_label"));
    _w.status_statusbar = GTK_STATUSBAR(get_widget("status_statusbar"));
    
    g_signal_connect(_w.drawingarea, "expose_event", G_CALLBACK(on_expose_event), this);
    g_signal_connect(_window, "button-press-event", G_CALLBACK(on_button_press_event), this);

    add_menu_accel_group("<KCemu>", "menu_emulator");
    add_menu_accel_group("<KCemu>", "menu_view");
    add_menu_accel_group("<KCemu>", "menu_options");
    add_menu_accel_group("<KCemu>", "menu_popup");
    add_menu_accel_group("<KCemu>", "menu_view_p");
    
    GtkMenuItem *helpitem = GTK_MENU_ITEM(get_widget("menubaritem_help"));
    gtk_menu_item_set_right_justified(helpitem, TRUE);
    
    // initialize before attaching signal handlers
    set_display_effect(get_display_effect());

    wire_menu_item("menuitem_run",             NULL,     "kc-image-run");
    wire_menu_item("menuitem_load",            "<alt>L", "kc-image-load");
    wire_menu_item("menuitem_save",            "<alt>S", "ui-save-memory-window-toggle");
    wire_menu_item("menuitem_tape",            "<alt>T", "ui-tape-window-toggle");
    wire_menu_item("menuitem_disk",            "<alt>D", "ui-disk-window-toggle");
    wire_menu_item("menuitem_module",          "<alt>M", "ui-module-window-toggle");
    wire_menu_item("menuitem_audio",           "<alt>U", "ui-wav-window-toggle");
    wire_menu_item("menuitem_reset",           "<alt>R", "emu-reset");
    wire_menu_item("menuitem_power",           "<alt>P", "emu-power-on");
    wire_menu_item("menuitem_quit",            "<alt>Q", "emu-quit");
    wire_menu_item("menuitem_zoom1",           "<alt>1", "ui-zoom-1");
    wire_menu_item("menuitem_zoom2",           "<alt>2", "ui-zoom-2");
    wire_menu_item("menuitem_zoom3",           "<alt>3", "ui-zoom-3");
    wire_menu_item("menuitem_keyboard",        "<alt>K", "ui-keyboard-window-toggle");
    wire_menu_item("menuitem_menubar",         NULL,     "ui-menu-bar-toggle");
    wire_menu_item("menuitem_statusbar",       NULL,     "ui-status-bar-toggle");
    wire_menu_item("menuitem_colors",          "<alt>C", "ui-color-window-toggle");
    wire_menu_item("menuitem_display_effects", NULL,     "ui-display-effects-toggle");
    wire_menu_item("menuitem_no_speed_limit",  "<alt>G", "ui-speed-limit-toggle");
    wire_menu_item("menuitem_profiles",        NULL,     "ui-options-window-toggle");
    wire_menu_item("menuitem_help",            NULL,     "ui-help-window-toggle-home");
    wire_menu_item("menuitem_help_index",      NULL,     "ui-help-window-toggle-index");
    wire_menu_item("menuitem_context_help",    NULL,     "ui-help-window-context-help");
    wire_menu_item("menuitem_about",           NULL,     "ui-about-window-toggle");
    wire_menu_item("menuitem_thanks",          NULL,     "ui-thanks-window-toggle");
    wire_menu_item("menuitem_licence",         NULL,     "ui-copying-window-toggle");
    wire_menu_item("menuitem_no_warranty",     NULL,     "ui-warranty-window-toggle");

    gtk_accel_map_load(_accel_map_path.c_str());
    g_signal_connect(_w.accel_group, "accel-changed", G_CALLBACK(on_accel_group_changed), this);
}

void
MainWindow::init_icon(void) {
    GdkPixbuf *icon = NULL;
    
    switch (Preferences::instance()->get_kc_type()) {
        case KC_TYPE_85_1:
        case KC_TYPE_87:
            icon = get_icon("kcemu-kc87.xpm");
            g_object_set_data(G_OBJECT(_window), "help-topic", (gpointer)"sys-z9001");
            break;
        case KC_TYPE_85_2:
        case KC_TYPE_85_3:
        case KC_TYPE_85_4:
            icon = get_icon("kcemu-kc85.xpm");
            g_object_set_data(G_OBJECT(_window), "help-topic", (gpointer)"sys-kc85");
            break;
        case KC_TYPE_85_5:
            icon = get_icon("kcemu-kc85.xpm");
            g_object_set_data(G_OBJECT(_window), "help-topic", (gpointer)"sys-kc855");
            break;
        case KC_TYPE_LC80:
            icon = get_icon("kcemu-lc80.xpm");
            g_object_set_data(G_OBJECT(_window), "help-topic", (gpointer)"sys-lc80");
            break;
        case KC_TYPE_Z1013:
            icon = get_icon("kcemu-z1013.xpm");
            g_object_set_data(G_OBJECT(_window), "help-topic", (gpointer)"sys-z1013");
            break;
        case KC_TYPE_A5105:
            icon = get_icon("kcemu-bic.xpm");
            g_object_set_data(G_OBJECT(_window), "help-topic", (gpointer)"sys-a5105");
            break;
        case KC_TYPE_POLY880:
            icon = get_icon("kcemu-poly880.xpm");
            g_object_set_data(G_OBJECT(_window), "help-topic", (gpointer)"sys-poly880");
            break;
        case KC_TYPE_KRAMERMC:
            icon = get_icon("kcemu-kramermc.xpm");
            g_object_set_data(G_OBJECT(_window), "help-topic", (gpointer)"sys-kramermc");
            break;
        case KC_TYPE_MUGLERPC:
            icon = get_icon("kcemu-pcm.xpm");
            g_object_set_data(G_OBJECT(_window), "help-topic", (gpointer)"sys-muglerpc");
            break;
        case KC_TYPE_VCS80:
            icon = get_icon("kcemu-vcs80.xpm");
            g_object_set_data(G_OBJECT(_window), "help-topic", (gpointer)"sys-vcs80");
            break;
        case KC_TYPE_C80:
            icon = get_icon("kcemu-c80.xpm");
            g_object_set_data(G_OBJECT(_window), "help-topic", (gpointer)"sys-c80");
            break;
        case KC_TYPE_ALL:
        case KC_TYPE_NONE:
        case KC_TYPE_85_1_CLASS:
        case KC_TYPE_85_2_CLASS:
            DBG(0, form("KCemu/internal_error",
                    "KCemu: got unhandled value from get_kc_type(): %d\n",
                    Preferences::instance()->get_kc_type()));
            break;
    }
    
    if (icon != NULL)
        gtk_window_set_icon(GTK_WINDOW(_window), icon);
}

void
MainWindow::process_events(void) {
    if (_w.status_sec == 0)
        return;
    
    long tv_sec, tv_usec;
    
    sys_gettimeofday(&tv_sec, &tv_usec);
    if (tv_sec - _w.status_sec > 10) {
        gtk_statusbar_pop(_w.status_statusbar, 1);
    }
}

void
MainWindow::show(int width, int height) {
    init();
    gtk_drawing_area_size(_w.drawingarea, width, height);
    UI_Gtk_Window::show();
}

void
MainWindow::resize(int width, int height) {
    if ((_width == width) && (_height == height))
        return;
    
    if (_image)
        gdk_image_destroy(_image);
    _image  = gdk_image_new(GDK_IMAGE_FASTEST, _visual, width, height);
    
    gtk_drawing_area_size(_w.drawingarea, width, height);
    
    if (!GTK_WIDGET_VISIBLE(_window))
        gtk_widget_show(_window);
    
    if (_gc)
        gdk_gc_destroy(_gc);
    
    _gc = gdk_gc_new(GTK_WIDGET(_w.drawingarea)->window);
    
    // force reallocation of dirty buffer
    if (_dirty_old != NULL) {
        delete _dirty_old;
        _dirty_old = NULL;
    }
}

void
MainWindow::allocate_color_hsv(int idx, double h, double s, double v) {
    int r, g, b;
    
    hsv2rgb(h, s, v, &r, &g, &b);

    _col[idx].red = r << 8;
    _col[idx].green = g << 8;
    _col[idx].blue = b << 8;
    _colormap = gdk_colormap_get_system();
    gdk_color_alloc(_colormap, &_col[idx]);
}

void
MainWindow::allocate_color_rgb(int idx, int r, int g, int b) {
    _col[idx].red = r << 8;
    _col[idx].green = g << 8;
    _col[idx].blue = b << 8;
    _colormap = gdk_colormap_get_system();
    gdk_color_alloc(_colormap, &_col[idx]);
}

void
MainWindow::allocate_colors(const char *color_names[]) {
    _colormap = gdk_colormap_get_system();
    for (int a = 0;color_names[a];a++) {
        gdk_color_parse(color_names[a], &_col[a]);
        gdk_color_alloc(_colormap, &_col[a]);
    }
}

static gulong
lighter_color(gulong col) {
    static int color_add = 50; // RC::instance()->get_int("DEBUG UI_Gtk Color Add", 50);
    
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
darker_color(gulong col) {
    int r = (col >> 16) & 0xff;
    int g = (col >>  8) & 0xff;
    int b = (col      ) & 0xff;
    
    r = (2 * r) / 3;
    g = (2 * g) / 3;
    b = (2 * b) / 3;
    
    return (r << 16) | (g << 8) | b;
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
MainWindow::get_col(byte_t *bitmap, int which, int idx, int width) {
    gulong p;
    long r, g, b, w;
    
    w = 0; r = 0; g = 0; b = 0;
    
    switch (which) {
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

void
MainWindow::update_1(byte_t *bitmap, byte_t *dirty, int dirty_size, int width, int height) {
    int d = -1;
    for (int y = 0;y < height;y += 8) {
        for (int x = 0;x < width;x += 8) {
            d++;
            if (!dirty[d])
                continue;
            
            int z = y * width + x;
            
            for (int yy = 0;yy < 8;yy++) {
                for (int xx = 0;xx < 8;xx++) {
                    gdk_image_put_pixel(_image, x + xx, y + yy, _col[bitmap[z + xx]].pixel);
                }
                z += width;
            }
        }
    }
}

void
MainWindow::update_1_debug(byte_t *bitmap, byte_t *dirty, int dirty_size, int width, int height) {
    static int frame_delay = 50;
    
    if (_dirty_old == NULL) {
        _dirty_old = new byte_t[dirty_size];
        memset(_dirty_old, 0, dirty_size);
        //frame_delay = 50; // RC::instance()->get_int("DEBUG UI_Gtk Frame Delay", 50);
    }
    
    int d = -1;
    for (int y = 0;y < height;y += 8) {
        for (int x = 0;x < width;x += 8) {
            d++;
            if (dirty[d])
                _dirty_old[d] = frame_delay;
            
            if (_dirty_old[d] == 0)
                continue;
            
            if (_dirty_old[d] > 0)
                _dirty_old[d]--;
            
            dirty[d] = 1;
            
            int z = y * width + x;
            
            if (_dirty_old[d]) {
                for (int yy = 0;yy < 8;yy++) {
                    for (int xx = 0;xx < 8;xx++) {
                        gdk_image_put_pixel(_image, x + xx, y + yy, lighter_color(_col[bitmap[z + xx]].pixel));
                    }
                    z += width;
                }
            }
            else {
                for (int yy = 0;yy < 8;yy++) {
                    for (int xx = 0;xx < 8;xx++) {
                        gdk_image_put_pixel(_image, x + xx, y + yy, _col[bitmap[z + xx]].pixel);
                    }
                    z += width;
                }
            }
        }
    }
}

void
MainWindow::update_2(byte_t *bitmap, byte_t *dirty, int dirty_size, int width, int height) {
    int d = -1;
    for (int y = 0;y < height;y += 8) {
        for (int x = 0;x < width;x += 8) {
            d++;
            if (!dirty[d])
                continue;
            
            int z = y * width + x;
            
            for (int yy = 0;yy < 16;yy += 2) {
                for (int xx = 0;xx < 16;xx += 2) {
                    gulong pix = _col[bitmap[z++]].pixel;
                    gdk_image_put_pixel(_image, 2 * x + xx,     2 * y + yy    , pix);
                    gdk_image_put_pixel(_image, 2 * x + xx + 1, 2 * y + yy    , pix);
                    gdk_image_put_pixel(_image, 2 * x + xx    , 2 * y + yy + 1, pix);
                    gdk_image_put_pixel(_image, 2 * x + xx + 1, 2 * y + yy + 1, pix);
                }
                z += width - 8;
            }
        }
    }
}

void
MainWindow::update_2_scanline(byte_t *bitmap, byte_t *dirty, int dirty_size, int width, int height) {
    int d = -1;
    for (int y = 0;y < height;y += 8) {
        for (int x = 0;x < width;x += 8) {
            d++;
            if (!dirty[d])
                continue;
            
            int z = y * width + x;
            
            for (int yy = 0;yy < 16;yy += 2) {
                for (int xx = 0;xx < 16;xx += 2) {
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
MainWindow::update_3(byte_t *bitmap, byte_t *dirty, int dirty_size, int width, int height) {
    int d = -1;
    for (int y = 0;y < height;y += 8) {
        for (int x = 0;x < width;x += 8) {
            d++;
            if (!dirty[d])
                continue;
            
            int z = y * width + x;
            
            for (int yy = 0;yy < 24;yy += 3) {
                for (int xx = 0;xx < 24;xx += 3) {
                    gulong pix = _col[bitmap[z++]].pixel;
                    gdk_image_put_pixel(_image, 3 * x + xx    , 3 * y + yy    , pix);
                    gdk_image_put_pixel(_image, 3 * x + xx + 1, 3 * y + yy    , pix);
                    gdk_image_put_pixel(_image, 3 * x + xx + 2, 3 * y + yy    , pix);
                    gdk_image_put_pixel(_image, 3 * x + xx    , 3 * y + yy + 1, pix);
                    gdk_image_put_pixel(_image, 3 * x + xx + 1, 3 * y + yy + 1, pix);
                    gdk_image_put_pixel(_image, 3 * x + xx + 2, 3 * y + yy + 1, pix);
                    gdk_image_put_pixel(_image, 3 * x + xx    , 3 * y + yy + 2, pix);
                    gdk_image_put_pixel(_image, 3 * x + xx + 1, 3 * y + yy + 2, pix);
                    gdk_image_put_pixel(_image, 3 * x + xx + 2, 3 * y + yy + 2, pix);
                }
                z += width - 8;
            }
        }
    }
}

void
MainWindow::update_3_smooth(byte_t *bitmap, byte_t *dirty, int dirty_size, int width, int height) {
    int d = -1;
    byte_t dirty_buf[dirty_size];
    
    memcpy(dirty_buf, dirty, dirty_size);
    
    for (int y = 0;y < height;y += 8) {
        for (int x = 0;x < width;x += 8) {
            d++;
            
            if (dirty[d]) {
                int z = y * width + x;
                
                for (int yy = 0;yy < 24;yy += 3) {
                    for (int xx = 0;xx < 24;xx += 3) {
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
            else {
                /*
                 *  not dirty but we need to check the neighbour pixels due to
                 *  the antialiasing
                 */
                if ((d > 0) && (dirty[d - 1])) {
                    int z = y * width + x;
                    for (int yy = 0;yy < 24;yy += 3) {
                        gdk_image_put_pixel(_image, 3 * x, 3 * y + yy    , get_col(bitmap, 0, z, width));
                        gdk_image_put_pixel(_image, 3 * x, 3 * y + yy + 1, get_col(bitmap, 3, z, width));
                        gdk_image_put_pixel(_image, 3 * x, 3 * y + yy + 2, get_col(bitmap, 6, z, width));
                        z += width;
                    }
                    dirty_buf[d] = 1;
                }
                if (dirty[d + 1]) {
                    int z = y * width + x + 7;
                    for (int yy = 0;yy < 24;yy += 3) {
                        gdk_image_put_pixel(_image, 3 * x + 23, 3 * y + yy    , get_col(bitmap, 2, z, width));
                        gdk_image_put_pixel(_image, 3 * x + 23, 3 * y + yy + 1, get_col(bitmap, 5, z, width));
                        gdk_image_put_pixel(_image, 3 * x + 23, 3 * y + yy + 2, get_col(bitmap, 8, z, width));
                        z += width;
                    }
                    dirty_buf[d] = 1;
                }
                if (dirty[d + width / 8]) {
                    int z = (y + 7) * width + x;
                    for (int xx = 0;xx < 24;xx += 3) {
                        gdk_image_put_pixel(_image, 3 * x + xx    , 3 * y + 23, get_col(bitmap, 6, z, width));
                        gdk_image_put_pixel(_image, 3 * x + xx + 1, 3 * y + 23, get_col(bitmap, 7, z, width));
                        gdk_image_put_pixel(_image, 3 * x + xx + 2, 3 * y + 23, get_col(bitmap, 8, z, width));
                        z++;
                    }
                    dirty_buf[d] = 1;
                }
                if ((d > width / 8) && (dirty[d - width / 8])) {
                    int z = y * width + x;
                    for (int xx = 0;xx < 24;xx += 3) {
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
MainWindow::update(UI_Base *ui, int image_width, int image_height, bool full_update) {

    byte_t *bitmap = ui->get_buffer();
    byte_t *dirty = ui->get_dirty_buffer();
    int dirty_size = ui->get_dirty_buffer_size();
    int width = ui->get_real_width();
    int height = ui->get_real_height();

    switch (kcemu_ui_scale) {
        case 1:
            if (kcemu_ui_debug)
                update_1_debug(bitmap, dirty, dirty_size, width, height);
            else
                update_1(bitmap, dirty, dirty_size, width, height);
            break;
        case 2:
            if (kcemu_ui_display_effect)
                update_2_scanline(bitmap, dirty, dirty_size, width, height);
            else
                update_2(bitmap, dirty, dirty_size, width, height);
            break;
        case 3:
            if (kcemu_ui_display_effect)
                update_3_smooth(bitmap, dirty, dirty_size, width, height);
            else
                update_3(bitmap, dirty, dirty_size, width, height);
            break;
    }
    
    if (full_update || _expose) {
        _expose = false;
        gdk_draw_image(GTK_WIDGET(_w.drawingarea)->window, _gc, _image,
                0, 0, 0, 0, image_width, image_height);
        return;
    }
    
    int d = -1;
    int s = 8 * kcemu_ui_scale;
    for (int y = 0;y < image_height;y += s) {
        for (int x = 0;x < image_width;x += s) {
            d++;
            if (!dirty[d])
                continue;
            
            dirty[d] = 0;
            gdk_draw_image(GTK_WIDGET(_w.drawingarea)->window, _gc, _image, x, y, x, y, s, s);
        }
    }
}

void
MainWindow::set_fps(unsigned long fps) {
    char buf[20];
    snprintf(buf, sizeof(buf), " %ld fps ", fps);
    gtk_label_set(_w.status_label, buf);
}

void
MainWindow::status_bar_toggle(void) {
    if (GTK_WIDGET_VISIBLE(_w.status_hbox))
        gtk_widget_hide(GTK_WIDGET(_w.status_hbox));
    else
        gtk_widget_show(GTK_WIDGET(_w.status_hbox));
}

void
MainWindow::menu_bar_toggle(void) {
    if (GTK_WIDGET_VISIBLE(_w.menubar))
        gtk_widget_hide(GTK_WIDGET(_w.menubar));
    else
        gtk_widget_show(GTK_WIDGET(_w.menubar));
}

void
MainWindow::setStatus(const char *msg) {
    long tv_sec, tv_usec;
    
    sys_gettimeofday(&tv_sec, &tv_usec);
    _w.status_sec = tv_sec;
    gtk_statusbar_pop(GTK_STATUSBAR(_w.status_statusbar), 1);
    gtk_statusbar_push(GTK_STATUSBAR(_w.status_statusbar), 1, msg);
}
