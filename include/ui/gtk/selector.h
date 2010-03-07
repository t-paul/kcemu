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

#ifndef __ui_gtk_selector_h
#define __ui_gtk_selector_h

#include "kc/system.h"
#include "kc/prefs/profile.h"

#include "ui/gtk/window.h"

using namespace std;

class ProfileSelectorWindow : public UI_Gtk_Window {
private:
    enum {
        ICONVIEW_PIXBUF_COLUMN,
        ICONVIEW_TEXT_COLUMN,
        ICONVIEW_TOOLTIP_COLUMN,
        ICONVIEW_DATA_COLUMN,
                
        ICONVIEW_N_COLUMNS
    };
    
    enum {
        TREEVIEW_PIXBUF_COLUMN,
        TREEVIEW_TEXT_COLUMN,
        TREEVIEW_COMMENT_COLUMN,
        TREEVIEW_DATA_COLUMN,
        TREEVIEW_ALIGN_COLUMN,
        TREEVIEW_WEIGHT_COLUMN,
        TREEVIEW_WEIGHT_SET_COLUMN,
        
        TREEVIEW_N_COLUMNS
    };

    struct {
        GtkIconView *iconview;
        GtkTreeView *treeview;
        GtkExpander *expander_system_profiles;
        GtkExpander *expander_user_profiles;
        GtkButton   *button_ok;
        GtkButton   *button_quit;
        
        GtkListStore *liststore_iconview;
        GtkTreeStore *treestore_treeview;
    } _w;
    
    CMD *_cmd;
    string _selected_profile;
    int _nr_of_user_profiles;
    
protected:
    void init(void);
    
    GtkListStore * get_iconview_model(void);
    GtkTreeStore * get_treeview_model(void);

    void show(void);
    void close(void);
    void cancel(void);

    void select(const char *profile);
    void select(GtkTreeModel *model, GtkTreePath *path, int column);
    void iconview_model_add(GtkListStore *store, GdkPixbuf *pixbuf, const char *text, const char *tooltip, const char *profile);
    void treeview_model_add(GtkTreeStore *store, GdkPixbuf *pixbuf, const char *text, const char *comment, const char *profile, GtkTreeIter *iter, GtkTreeIter *parent);
    
    static void on_dialog_ok(GtkWidget *widget, gpointer user_data);
    static void on_dialog_cancel(GtkWidget *widget, gpointer user_data);
    static void on_dialog_quit(GtkWidget *widget, gpointer user_data);
    static void on_edit_button_clicked(GtkButton *button, gpointer user_data);
    static void on_iconview_selection_changed(GtkIconView *iconview, gpointer user_data);
    static void on_iconview_item_activated(GtkIconView *iconview, GtkTreePath *path, gpointer user_data);
    static void on_tree_selection_changed(GtkTreeSelection *selection, gpointer user_data);
    static void on_tree_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
    static gboolean on_window_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);

public:
    ProfileSelectorWindow(const char *ui_xml_file);
    virtual ~ProfileSelectorWindow(void);
    
    char * get_selected_profile(void);
};

#endif /* __ui_gtk_selector_h */

