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
#include <stdlib.h>

#include "kc/system.h"

#include "kc/prefs/prefs.h"

#include "ui/gtk/cmd.h"
#include "ui/gtk/selector.h"

class CMD_profile_selector_window_toggle : public CMD {
private:
    ProfileSelectorWindow *_w;
    
public:
    CMD_profile_selector_window_toggle(ProfileSelectorWindow *w) : CMD("ui-profile-selector-window-toggle") {
        _w = w;
        register_cmd("ui-profile-selector-window-toggle");
    }
    
    void execute(CMD_Args *args, CMD_Context context) {
        _w->toggle();
    }
};

ProfileSelectorWindow::ProfileSelectorWindow(const char *ui_xml_file) : UI_Gtk_Window(ui_xml_file) {
    _w.liststore_iconview = NULL;
    _w.treestore_treeview = NULL;
    _cmd = new CMD_profile_selector_window_toggle(this);
}

ProfileSelectorWindow::~ProfileSelectorWindow(void) {
    if (_w.liststore_iconview != NULL)
        gtk_list_store_clear(_w.liststore_iconview);
    if (_w.treestore_treeview != NULL)
        gtk_tree_store_clear(_w.treestore_treeview);
    delete _cmd;
}

void
ProfileSelectorWindow::on_dialog_ok(GtkWidget *widget, gpointer user_data) {
    ProfileSelectorWindow *self = (ProfileSelectorWindow *)user_data;
    self->close();
}

void
ProfileSelectorWindow::on_dialog_quit(GtkWidget *widget, gpointer user_data) {
    exit(0);
}

void
ProfileSelectorWindow::on_dialog_cancel(GtkWidget *widget, gpointer user_data) {
    ProfileSelectorWindow *self = (ProfileSelectorWindow *)user_data;
    self->cancel();
}

gboolean
ProfileSelectorWindow::on_window_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
    ProfileSelectorWindow *self = (ProfileSelectorWindow *)user_data;
    self->cancel();
    return TRUE; /* DO NOT propagate event */
}

void
ProfileSelectorWindow::on_iconview_selection_changed(GtkIconView *iconview, gpointer user_data) {
    ProfileSelectorWindow *self = (ProfileSelectorWindow *)user_data;

    GList *list = gtk_icon_view_get_selected_items(iconview);
    if (g_list_length(list) == 0)
    {
        self->select(NULL);
        return;
    }
 
    gtk_tree_selection_unselect_all(gtk_tree_view_get_selection(self->_w.treeview));

    GtkTreePath *path = (GtkTreePath *)list->data;
    self->select(GTK_TREE_MODEL(self->_w.liststore_iconview), path, ICONVIEW_DATA_COLUMN);
    g_list_foreach(list, (GFunc)gtk_tree_path_free, NULL);
    g_list_free(list);
}

void
ProfileSelectorWindow::on_iconview_item_activated(GtkIconView *iconview, GtkTreePath *path, gpointer user_data) {
    ProfileSelectorWindow *self = (ProfileSelectorWindow *)user_data;
    self->select(GTK_TREE_MODEL(self->_w.liststore_iconview), path, ICONVIEW_DATA_COLUMN);
    self->close();
}

void
ProfileSelectorWindow::on_tree_selection_changed(GtkTreeSelection *selection, gpointer user_data) {
    ProfileSelectorWindow *self = (ProfileSelectorWindow *)user_data;

    GtkTreeIter iter;
    GtkTreeModel *model;
    
    if (!gtk_tree_selection_get_selected(selection, &model, &iter))
        return;
    
    gtk_icon_view_unselect_all(self->_w.iconview);

    gchar *profile;
    gtk_tree_model_get(model, &iter, TREEVIEW_DATA_COLUMN, &profile, -1);
    self->select(profile);
    g_free(profile);
}

void
ProfileSelectorWindow::on_tree_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data) {
    ProfileSelectorWindow *self = (ProfileSelectorWindow *)user_data;
    self->select(GTK_TREE_MODEL(self->_w.treestore_treeview), path, TREEVIEW_DATA_COLUMN);
    self->close();
}

void
ProfileSelectorWindow::on_edit_button_clicked(GtkButton *button, gpointer user_data)
{
    CMD_EXEC("ui-options-window-toggle");
}

char *
ProfileSelectorWindow::get_selected_profile(void) {
    if (_selected_profile.length() == 0)
        return NULL;
    
    return strdup(_selected_profile.c_str());
}

void
ProfileSelectorWindow::select(const char *profile) {
    if (profile == NULL) {
        _selected_profile = "";
    } else {
        _selected_profile = profile;
    }
    gtk_widget_set_sensitive(GTK_WIDGET(_w.button_ok), profile != NULL);
}

void
ProfileSelectorWindow::select(GtkTreeModel *model, GtkTreePath *path, int column) {
    GtkTreeIter iter;
    GValue value = { 0, };
    gtk_tree_model_get_iter(model, &iter, path);
    gtk_tree_model_get_value(model, &iter, column, &value);
    const gchar *profile = g_value_get_string(&value);
    select(profile);
}

void
ProfileSelectorWindow::iconview_model_add(GtkListStore *store, GdkPixbuf *pixbuf, const char *text, const char *tooltip, const char *profile)
{
    GtkTreeIter iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
            ICONVIEW_PIXBUF_COLUMN, pixbuf,
            ICONVIEW_TEXT_COLUMN, text,
            ICONVIEW_TOOLTIP_COLUMN, tooltip,
            ICONVIEW_DATA_COLUMN, profile, -1);
}

GtkListStore *
ProfileSelectorWindow::get_iconview_model(void) {
    
    GtkListStore *store = gtk_list_store_new(ICONVIEW_N_COLUMNS, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

    emulation_type_list_t list = EmulationType::get_emulation_types();
    for (emulation_type_list_t::iterator it = list.begin();it != list.end();it++) {
        GdkPixbuf *pixbuf = get_icon((*it)->get_image_name());
        iconview_model_add(store, pixbuf, (*it)->get_short_name(), (*it)->get_name(), (*it)->get_config_name());
        g_object_unref(pixbuf);
    }
    
    return store;
}

void
ProfileSelectorWindow::treeview_model_add(GtkTreeStore *store, GdkPixbuf *pixbuf, const char *text, const char *comment, const char *profile, GtkTreeIter *iter, GtkTreeIter *parent)
{
    gtk_tree_store_append(store, iter, parent);
    gtk_tree_store_set(store, iter,
            TREEVIEW_PIXBUF_COLUMN, pixbuf,
            TREEVIEW_TEXT_COLUMN, text,
            TREEVIEW_COMMENT_COLUMN, comment,
            TREEVIEW_DATA_COLUMN, profile,
            TREEVIEW_ALIGN_COLUMN, 0.0,
            TREEVIEW_WEIGHT_COLUMN, parent == NULL ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL,
            TREEVIEW_WEIGHT_SET_COLUMN, TRUE,
            -1);
}

GtkTreeStore *
ProfileSelectorWindow::get_treeview_model(void) {
    GtkTreeStore *store = gtk_tree_store_new(TREEVIEW_N_COLUMNS,
            GDK_TYPE_PIXBUF,
            G_TYPE_STRING,
            G_TYPE_STRING,
            G_TYPE_STRING,
            G_TYPE_DOUBLE,
            G_TYPE_INT,
            G_TYPE_BOOLEAN);

    int nr_of_user_profiles = 0;
    GdkPixbuf *pixbuf_default = NULL;
    
    emulation_type_list_t types = EmulationType::get_emulation_types();
    for (emulation_type_list_t::const_iterator it = types.begin();it != types.end();it++) {
        list<Profile *> profiles = Preferences::instance()->find_child_profiles((*it)->get_config_name());
        if (profiles.size() == 0)
            continue;

        if (pixbuf_default == NULL) {
            pixbuf_default = get_icon("icon-default.png");
        }
        
        GtkTreeIter parent;
        GdkPixbuf *pixbuf = get_icon((*it)->get_icon_name());
        treeview_model_add(store, pixbuf, (*it)->get_name(), "", (*it)->get_config_name(), &parent, NULL);
        g_object_unref(pixbuf);

        for (list<Profile *>::const_iterator it2 = profiles.begin();it2 != profiles.end();it2++) {
            GtkTreeIter iter;
            treeview_model_add(store, pixbuf_default, (*it2)->get_name(), (*it2)->get_comment(), (*it2)->get_config_name(), &iter, &parent);
            nr_of_user_profiles++;
        }
    }
    
    _nr_of_user_profiles = nr_of_user_profiles;
    
    return store;
}

void
ProfileSelectorWindow::show(void) {
    UI_Gtk_Window::show();
    gtk_main();
}

void
ProfileSelectorWindow::close(void) {
    hide();
    gtk_main_quit();
}

void
ProfileSelectorWindow::cancel(void) {
    select(NULL);
    close();
}

void
ProfileSelectorWindow::init(void) {
    _window = get_widget("profile_selector_window");
    gtk_signal_connect(GTK_OBJECT(_window), "delete_event", GTK_SIGNAL_FUNC(on_window_delete_event), this);

    _w.iconview = GTK_ICON_VIEW(get_widget("iconview_system_profiles"));
    _w.liststore_iconview = get_iconview_model();

    gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW(_w.iconview), ICONVIEW_PIXBUF_COLUMN);
    gtk_icon_view_set_text_column(GTK_ICON_VIEW(_w.iconview), ICONVIEW_TEXT_COLUMN);
    gtk_icon_view_set_tooltip_column(GTK_ICON_VIEW(_w.iconview), ICONVIEW_TOOLTIP_COLUMN);
    gtk_icon_view_set_model(GTK_ICON_VIEW(_w.iconview), GTK_TREE_MODEL(_w.liststore_iconview));
    g_signal_connect(_w.iconview, "selection-changed", G_CALLBACK(on_iconview_selection_changed), this);
    g_signal_connect(_w.iconview, "item-activated", G_CALLBACK(on_iconview_item_activated), this);

    _w.treeview = GTK_TREE_VIEW(get_widget("treeview_user_profiles"));
    _w.treestore_treeview = get_treeview_model();
    
    gtk_tree_view_set_model(GTK_TREE_VIEW(_w.treeview), GTK_TREE_MODEL(_w.treestore_treeview));

    GtkTreeViewColumn *column1 = gtk_tree_view_column_new();
    add_icon_renderer(GTK_TREE_VIEW(_w.treeview), column1, "",
            "pixbuf", TREEVIEW_PIXBUF_COLUMN,
            "yalign", TREEVIEW_ALIGN_COLUMN,
            NULL);
    add_text_renderer(GTK_TREE_VIEW(_w.treeview), column1, "Profile",
            "text", TREEVIEW_TEXT_COLUMN,
            "yalign", TREEVIEW_ALIGN_COLUMN,
            "weight", TREEVIEW_WEIGHT_COLUMN,
            NULL);
    gtk_tree_view_append_column(_w.treeview, column1);
    GtkTreeViewColumn *column2 = gtk_tree_view_column_new();
    add_text_renderer(GTK_TREE_VIEW(_w.treeview), column2, "Comment",
            "text", TREEVIEW_COMMENT_COLUMN,
            "yalign", TREEVIEW_ALIGN_COLUMN,
            NULL);
    gtk_tree_view_append_column(_w.treeview, column2);
    gtk_tree_view_expand_all(_w.treeview);
    gtk_tree_view_columns_autosize(GTK_TREE_VIEW(_w.treeview));
    g_signal_connect(_w.treeview, "row-activated", G_CALLBACK(on_tree_row_activated), this);

    /* Setup the selection handler */
    GtkTreeSelection *select = gtk_tree_view_get_selection(_w.treeview);
    gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);
    g_signal_connect(select, "changed", G_CALLBACK(on_tree_selection_changed), this);

    GtkWidget *cancel_button = get_widget("dialog_button_cancel");
    g_signal_connect(cancel_button, "clicked", G_CALLBACK(on_dialog_cancel), this);

    _w.button_ok = GTK_BUTTON(get_widget("dialog_button_close"));
    g_signal_connect(_w.button_ok, "clicked", G_CALLBACK(on_dialog_ok), this);

    _w.button_quit = GTK_BUTTON(get_widget("dialog_button_quit"));
    g_signal_connect(_w.button_quit, "clicked", G_CALLBACK(on_dialog_quit), this);

    _w.expander_system_profiles = GTK_EXPANDER(get_widget("expander_system_profiles"));
    _w.expander_user_profiles = GTK_EXPANDER(get_widget("expander_user_profiles"));
    gtk_expander_set_expanded(_w.expander_user_profiles, _nr_of_user_profiles != 0);

    this->select(NULL);
    init_dialog(NULL, NULL);
}
