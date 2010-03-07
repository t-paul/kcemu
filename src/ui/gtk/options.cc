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

/*
 * Enable Sound
 * Joystick Device
 * Enable V24-Module
 * Custom ROM Module
 * Custom ROM Module (Name)
 * Custom ROM Module (Id)
 * Custom ROM Module (File)
 * Custom ROM Module (Size)
 * V24-Baudrate
 * V24-Device
 * Flash Offset
 * Foreground Saturation
 * Foreground Brightness
 * Background Saturation
 * Background Brightness
 * Black Value
 * White Value
 * Max Auto Skip
 * DEBUG UI_Gtk Color Add
 * DEBUG UI_Gtk Frame Delay
 */

#include <stdlib.h>

#include <list>
#include <string>

#include "kc/system.h"
#include "kc/prefs/types.h"
#include "kc/prefs/prefs.h"
#include "kc/prefs/strlist.h"

#include "kc/kc.h"

#include "kc/mod_list.h"

#include "sys/sysdep.h"

#include "ui/gtk/cmd.h"
#include "ui/gtk/dialog.h"
#include "ui/gtk/options.h"

class CMD_options_window_toggle : public CMD {
private:
    OptionsWindow *_w;
    
public:
    CMD_options_window_toggle(OptionsWindow *w) : CMD("ui-options-window-toggle") {
        _w = w;
        register_cmd("ui-options-window-toggle");
    }
    
    void execute(CMD_Args *args, CMD_Context context) {
        _w->toggle();
    }
};

class CMD_options_window_dialog : public CMD {
private:
    OptionsWindow *_w;
    
public:
    CMD_options_window_dialog(OptionsWindow *w) : CMD("ui-options-window-dialog") {
        _w = w;
        register_cmd("ui-options-window-dialog");
    }
    
    void execute(CMD_Args *args, CMD_Context context) {
        if (args == NULL)
            args = new CMD_Args();
        
        switch (context) {
        case 0:
            args->set_string_arg("ui-dialog-title", _("Save profiles?"));
            args->set_string_arg("ui-dialog-text",
                               _("Some profiles were changed!\n\n"
                                 "'Yes' will save changes to disk, 'No' will reject all changes.\n"
                                 "'Cancel': will allow to continue editing."));
            args->set_string_arg("ui-dialog-text-arg", "filename");
            args->add_callback("ui-dialog-cancel-CB", this, 1);
            args->add_callback("ui-dialog-yes-no-CB-no", this, 2);
            args->add_callback("ui-dialog-yes-no-CB-yes", this, 3);
            CMD_EXEC_ARGS("ui-dialog-yes-no-cancel", args);
            break;
        case 1: // cancel
            break;
        case 2: // no
            _w->reject();
            break;
        case 3: // yes
            _w->save();
            break;
        }
    }
};

const char * OptionsWindow::FILE_CHOOSER_BUTTON_KEY = "filechooser_button";
const char * OptionsWindow::FILE_CHOOSER_CLEAR_BUTTON_KEY = "filechooser_clear_button";
const char * OptionsWindow::DATA_KEY_CHECK_BUTTON = "key_check_button";
const char * OptionsWindow::PREFERENCES_KEY = "preferences_key";
const char * OptionsWindow::DEFAULT_DIR_KEY = "default_dir_key";
const char * OptionsWindow::TARGET_WIDGET1_KEY = "target_widget1_key";
const char * OptionsWindow::TARGET_WIDGET2_KEY = "target_widget2_key";
const char * OptionsWindow::TARGET_WIDGET3_KEY = "target_widget3_key";

OptionsWindow::OptionsWindow(const char *ui_xml_file) : UI_Gtk_Window(ui_xml_file) {
    _current_profile = NULL;
    _current_kc_type = KC_TYPE_NONE;
    _open_rom_last_path = NULL;
    _w.liststore_modules = NULL;

    _cmd = new CMD_options_window_toggle(this);
    _cmd_dialog = new CMD_options_window_dialog(this);
}

OptionsWindow::~OptionsWindow(void) {
    delete _cmd;
    delete _cmd_dialog;
}

void
OptionsWindow::on_button_up_clicked(GtkButton *button, gpointer user_data) {
    OptionsWindow *self = (OptionsWindow *)user_data;
    self->move_row((GtkPathMoveFunc)gtk_tree_path_prev, gtk_tree_store_move_before);
}

void
OptionsWindow::on_button_down_clicked(GtkButton *button, gpointer user_data) {
    OptionsWindow *self = (OptionsWindow *)user_data;
    self->move_row(gtk_tree_path_next, gtk_tree_store_move_after);
}

void
OptionsWindow::on_button_expand_clicked(GtkButton *button, gpointer user_data) {
    OptionsWindow *self = (OptionsWindow *)user_data;
    self->expand_tree();
}

void
OptionsWindow::on_button_collapse_clicked(GtkButton *button, gpointer user_data) {
    OptionsWindow *self = (OptionsWindow *)user_data;
    self->collapse_tree();
}


void
OptionsWindow::expand_tree(void) {
    gtk_tree_view_expand_all(GTK_TREE_VIEW(_w.treeview));
}

void
OptionsWindow::collapse_tree(void) {
    gtk_tree_view_collapse_all(GTK_TREE_VIEW(_w.treeview));

    GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(_w.treeview));
    GtkTreePath *path = gtk_tree_path_new_first();
    gtk_tree_view_expand_to_path(GTK_TREE_VIEW(_w.treeview), path);
    gtk_tree_selection_select_path(select, path);
    gtk_tree_path_free(path);
}

void
OptionsWindow::on_button_new_clicked(GtkButton *button, gpointer user_data) {
    OptionsWindow *self = (OptionsWindow *)user_data;
    Profile *profile = Preferences::instance()->create_user_profile(self->_current_profile);
    if (profile == NULL)
        return;
    
    GtkTreeIter iter;
    GtkTreeModel *model = self->get_selected_tree_iter(&iter);
    if (model == NULL)
        return;
    
    GtkTreeIter iter_child;
    gtk_tree_store_append(GTK_TREE_STORE(model), &iter_child, &iter);
    gtk_tree_store_set(GTK_TREE_STORE(model), &iter_child,
            TREE_INT_COLUMN, 2,
            TREE_TEXT_COLUMN, profile->get_name(),
            TREE_CONFIG_NAME_COLUMN, profile->get_config_name(),
            TREE_ICON_COLUMN, self->_w.pixbuf_default,
            -1);

    self->expand_and_select(GTK_TREE_VIEW(self->_w.treeview), model, &iter_child);
    gtk_notebook_set_current_page(self->_w.notebook, 0);
}

void
OptionsWindow::expand_and_select(GtkTreeView *treeview, GtkTreeModel *model, GtkTreeIter *iter)
{
    GtkTreePath *path = gtk_tree_model_get_path(model, iter);
    gtk_tree_view_expand_to_path(treeview, path);
    gtk_tree_path_free(path);
    gtk_tree_selection_select_iter(gtk_tree_view_get_selection(treeview), iter);
}

void
OptionsWindow::on_button_copy_clicked(GtkButton *button, gpointer user_data) {
    OptionsWindow *self = (OptionsWindow *)user_data;
    Profile *profile = Preferences::instance()->copy_user_profile(self->_current_profile);
    if (profile == NULL)
        return;
    
    GtkTreeIter iter;
    GtkTreeModel *model = self->get_selected_tree_iter(&iter);
    if (model == NULL)
        return;
    
    GtkTreeIter parent;
    if (!gtk_tree_model_iter_parent(model, &parent, &iter))
        return;
    
    GtkTreeIter copy;
    gtk_tree_store_insert_after(GTK_TREE_STORE(model), &copy, &parent, &iter);
    gtk_tree_store_set(GTK_TREE_STORE(model), &copy,
            TREE_INT_COLUMN, 2,
            TREE_TEXT_COLUMN, profile->get_name(),
            TREE_CONFIG_NAME_COLUMN, profile->get_config_name(),
            TREE_ICON_COLUMN, self->_w.pixbuf_default,
            -1);
    
    self->expand_and_select(GTK_TREE_VIEW(self->_w.treeview), model, &copy);
    gtk_notebook_set_current_page(self->_w.notebook, 0);
}

void
OptionsWindow::on_button_delete_clicked(GtkButton *button, gpointer user_data) {
    OptionsWindow *self = (OptionsWindow *)user_data;
    GtkTreeIter iter;
    GtkTreeModel *model = self->get_selected_tree_iter(&iter);
    if (model == NULL)
        return;
    
    gtk_tree_store_set(GTK_TREE_STORE(model), &iter, TREE_ICON_COLUMN, self->_w.pixbuf_deleted, -1);
    self->_current_profile->set_deleted(true);
}

gboolean
OptionsWindow::tree_model_foreach_func_delete(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer user_data)
{
    OptionsWindow *self = (OptionsWindow *)user_data;

    gchar *config_name;
    gtk_tree_model_get(model, iter, TREE_CONFIG_NAME_COLUMN, &config_name, -1);

    if (Preferences::instance()->find_profile(config_name) == NULL) {
        gchar *path_string = gtk_tree_path_to_string(path);
        self->_delete_path.push_back(path_string);
        g_free(path_string);
    }
    
    g_free(config_name);
    return FALSE;
}

void
OptionsWindow::on_button_ok_clicked(GtkButton *button, gpointer user_data) {
    OptionsWindow *self = (OptionsWindow *)user_data;
    self->save();
}

void
OptionsWindow::on_button_close_clicked(GtkButton *button, gpointer user_data) {
    OptionsWindow *self = (OptionsWindow *)user_data;
    
    if (Preferences::instance()->has_changed_profiles()) {
        CMD_EXEC("ui-options-window-dialog");
    } else {
        self->hide();
    }
}

void
OptionsWindow::on_tree_selection_changed(GtkTreeSelection *selection, gpointer user_data) {
    OptionsWindow *self = (OptionsWindow *)user_data;
    
    GtkTreeIter iter;
    GtkTreeModel *model;
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gchar *key;
        gtk_tree_model_get(model, &iter, TREE_CONFIG_NAME_COLUMN, &key, -1);
        
        self->_current_profile = Preferences::instance()->find_profile(key);
        self->apply_profile();
        g_free(key);
        
        profile_level_t level = self->_current_profile->get_level();
        gtk_widget_set_sensitive(self->_w.b_new, level == PROFILE_LEVEL_SYSTEM_USER);
        gtk_widget_set_sensitive(self->_w.b_copy, level == PROFILE_LEVEL_USER);
        gtk_widget_set_sensitive(self->_w.b_delete, level == PROFILE_LEVEL_USER);
        gtk_widget_set_sensitive(self->_w.b_up, level == PROFILE_LEVEL_USER);
        gtk_widget_set_sensitive(self->_w.b_down, level == PROFILE_LEVEL_USER);
    }
}

void
OptionsWindow::on_media_check_button_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
    OptionsWindow *self = (OptionsWindow *)user_data;
    
    GtkWidget *filechooser_button = GTK_WIDGET(g_object_get_data(G_OBJECT(togglebutton), FILE_CHOOSER_BUTTON_KEY));
    GtkWidget *filechooser_clear_button = GTK_WIDGET(g_object_get_data(G_OBJECT(togglebutton), FILE_CHOOSER_CLEAR_BUTTON_KEY));
    gtk_widget_set_sensitive(filechooser_button, togglebutton->active);
    gtk_widget_set_sensitive(filechooser_clear_button, togglebutton->active);
    if (!togglebutton->active) {
        const char * key = (const char *)g_object_get_data(G_OBJECT(filechooser_button), PREFERENCES_KEY);
        self->_current_profile->remove_value(key);
        self->apply_filechooserbutton(GTK_FILE_CHOOSER(filechooser_button)); // apply parent value
    }
}

void
OptionsWindow::on_media_clear_button_clicked(GtkButton *button, gpointer user_data) {
    OptionsWindow *self = (OptionsWindow *)user_data;
    
    GtkFileChooser *filechooser = GTK_FILE_CHOOSER(g_object_get_data(G_OBJECT(button), FILE_CHOOSER_BUTTON_KEY));
    gtk_file_chooser_unselect_all(filechooser);
    const char * key = (const char *)g_object_get_data(G_OBJECT(filechooser), PREFERENCES_KEY);
    self->_current_profile->set_null_value(key);
}

void
OptionsWindow::on_media_filechooser_file_set(GtkFileChooserButton *filechooserbutton, gpointer user_data) {
    OptionsWindow *self = (OptionsWindow *)user_data;
    
    const char * key = (const char *)g_object_get_data(G_OBJECT(filechooserbutton), PREFERENCES_KEY);
    gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(filechooserbutton));
    if (filename == NULL) {
        self->_current_profile->set_null_value(key);
    } else {
        self->_current_profile->set_string_value(key, filename);
    }
    g_free(filename);
}

void
OptionsWindow::set_widget_sensitive_by_key(GObject *object, const gchar *key, bool sensitive) {
    gpointer widget = g_object_get_data(object, key);
    if (widget == NULL)
        return;
    
    gtk_widget_set_sensitive(GTK_WIDGET(widget), sensitive);
}

/**
 *  Returns true if the profile value was removed. In this case the display value
 *  need to be applied again to fetch the now valid parent value.
 */
bool
OptionsWindow::check_button_toggled(GtkToggleButton *togglebutton) {
    set_widget_sensitive_by_key(G_OBJECT(togglebutton), TARGET_WIDGET1_KEY, togglebutton->active);
    set_widget_sensitive_by_key(G_OBJECT(togglebutton), TARGET_WIDGET2_KEY, togglebutton->active);
    set_widget_sensitive_by_key(G_OBJECT(togglebutton), TARGET_WIDGET3_KEY, togglebutton->active);
    
    const char * key = (const char *)g_object_get_data(G_OBJECT(togglebutton), PREFERENCES_KEY);
    if (togglebutton->active) {
        ProfileValue *value = _current_profile->get_value(key);
        if (value != NULL)
          _current_profile->set_value(key, new ProfileValue(value));
        //_current_profile->set_int_value(key, _current_profile->get_int_value(key, 0));
    } else {
        _current_profile->remove_value(key);
    }
    
    return !togglebutton->active;
}

void
OptionsWindow::on_display_check_button_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
    OptionsWindow *self = (OptionsWindow *)user_data;
    if (self->check_button_toggled(togglebutton))
        self->apply_display_settings();
}

void
OptionsWindow::on_display_scale_value_changed(GtkSpinButton *spin_button, gpointer user_data) {
    OptionsWindow *self = (OptionsWindow *)user_data;
    int display_scale = gtk_spin_button_get_value_as_int(spin_button);
    self->_current_profile->set_int_value("display_scale", display_scale);
}

void
OptionsWindow::on_display_effects_changed(GtkComboBox *combobox, gpointer user_data) {
    OptionsWindow *self = (OptionsWindow *)user_data;
    int display_effects = gtk_combo_box_get_active(combobox);
    self->_current_profile->set_int_value("display_effects", display_effects);
}

void
OptionsWindow::on_display_debug_changed(GtkComboBox *combobox, gpointer user_data) {
    OptionsWindow *self = (OptionsWindow *)user_data;
    int display_debug = gtk_combo_box_get_active(combobox);
    self->_current_profile->set_int_value("display_debug", display_debug);
}

void
OptionsWindow::on_display_mem_access_changed(GtkComboBox *combobox, gpointer user_data) {
    OptionsWindow *self = (OptionsWindow *)user_data;
    int display_mem_access = gtk_combo_box_get_active(combobox);
    self->_current_profile->set_int_value("display_mem_access", display_mem_access);
}

void
OptionsWindow::on_profile_name_changed(GtkEditable *editable, gpointer user_data) {
    OptionsWindow *self = (OptionsWindow *)user_data;
    const char *name = gtk_entry_get_text(GTK_ENTRY(editable));
    
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(self->_w.treeview));
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_store_set(self->_w.treestore, &iter, TREE_TEXT_COLUMN, name, -1);
    }
    
    self->_current_profile->set_name(name);
    self->_current_profile->set_string_value("name", name);
}

void
OptionsWindow::on_comment_changed(GtkTextBuffer *textbuffer, gpointer user_data) {
    OptionsWindow *self = (OptionsWindow *)user_data;
    
    GtkTextIter start, end;
    
    gtk_text_buffer_get_iter_at_offset(textbuffer, &start, 0);
    gtk_text_buffer_get_iter_at_offset(textbuffer, &end, 0);
    gtk_text_iter_forward_to_end(&end);
    
    gchar *text = gtk_text_buffer_get_text(textbuffer, &start, &end, FALSE);
    
    const char *key = "comment";
    if (strlen(text) == 0) {
        self->_current_profile->remove_value(key);
    } else {
        self->_current_profile->set_string_value(key, text);
    }
    
    g_free(text);
}

void
OptionsWindow::on_system_type_changed(GtkComboBox *combobox, gpointer user_data) {
    OptionsWindow *self = (OptionsWindow *)user_data;
    
    GtkTreeIter iter;
    if (!gtk_combo_box_get_active_iter(self->_w.combobox_system_type, &iter))
        return;
    
    int kc_type;
    gtk_tree_model_get(GTK_TREE_MODEL(self->_w.liststore_system), &iter, 1, &kc_type, -1);
    
    self->_current_profile->set_int_value("system", kc_type);
    self->apply_system_variant((kc_type_t)kc_type, KC_VARIANT_NONE);
    self->apply_modules_settings((kc_type_t)kc_type);
    self->apply_roms_settings((kc_type_t)kc_type, KC_VARIANT_NONE);
}

void
OptionsWindow::on_system_variant_check_button_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
    const char *key = "variant";
    OptionsWindow *self = (OptionsWindow *)user_data;
    
    gtk_widget_set_sensitive(GTK_WIDGET(self->_w.combobox_system_variant), togglebutton->active);
    
    if (togglebutton->active) {
        self->_current_profile->set_int_value(key, self->_current_profile->get_int_value(key, KC_VARIANT_NONE));
    } else {
        self->_current_profile->remove_value(key);
        self->apply_system_variant(self->_current_kc_type, (kc_variant_t)self->_current_profile->get_int_value(key, KC_VARIANT_NONE));
    }
}

void
OptionsWindow::on_system_variant_changed(GtkComboBox *combobox, gpointer user_data) {
    OptionsWindow *self = (OptionsWindow *)user_data;
    
    GtkTreeIter iter;
    if (!gtk_combo_box_get_active_iter(self->_w.combobox_system_variant, &iter))
        return;
    
    int kc_variant;
    GtkTreeModel *model = gtk_combo_box_get_model(self->_w.combobox_system_variant);
    gtk_tree_model_get(model, &iter, 1, &kc_variant, -1);
    
    self->_current_profile->set_int_value("variant", kc_variant);
    self->apply_roms_settings(self->_current_kc_type, (kc_variant_t)kc_variant);
}

void
OptionsWindow::on_module_changed(GtkComboBox *combobox, gpointer user_data) {
    OptionsWindow *self = (OptionsWindow *)user_data;
    
    string list;
    const char *ptr = "";
    for (int a = 0;a < NR_OF_MODULES;a++) {
        GtkTreeIter iter;
        if (gtk_combo_box_get_active_iter(self->_w.combobox_module[a], &iter)) {
            gchar *modif;
            GtkTreeModel *model = gtk_combo_box_get_model(self->_w.combobox_module[a]);
            gtk_tree_model_get(model, &iter, MODULES_MODIF_COLUMN, &modif, -1);
            if (modif != NULL) {
                list += ptr;
                list += modif;
                ptr = ",";
                g_free(modif);
            }
        }
    }
    self->_current_profile->set_string_value("modules", list.c_str());
}

void
OptionsWindow::on_modules_check_button_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
    const char *key = "modules";
    OptionsWindow *self = (OptionsWindow *)user_data;
    
    for (int a = 0;a < NR_OF_MODULES;a++) {
        gtk_widget_set_sensitive(GTK_WIDGET(self->_w.combobox_module[a]), togglebutton->active);
    }
    
    if (togglebutton->active) {
        self->_current_profile->set_string_value(key, self->_current_profile->get_string_value(key, ""));
    } else {
        self->_current_profile->remove_value(key);
        self->apply_modules_settings(self->_current_kc_type);
    }
}

void
OptionsWindow::on_kc85_settings_check_button_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
    OptionsWindow *self = (OptionsWindow *)user_data;
    if (self->check_button_toggled(togglebutton))
        self->apply_kc85_settings();
}

void
OptionsWindow::on_kc85_d004_changed(GtkComboBox *combobox, gpointer user_data) {
    OptionsWindow *self = (OptionsWindow *)user_data;
    int d004 = gtk_combo_box_get_active(combobox);
    self->_current_profile->set_int_value("d004", d004);
}

void
OptionsWindow::on_kc85_f8_rom_changed(GtkComboBox *combobox, gpointer user_data) {
    OptionsWindow *self = (OptionsWindow *)user_data;
    int d004_f8 = gtk_combo_box_get_active(combobox);
    self->_current_profile->set_int_value("d004_f8", d004_f8);
}

void
OptionsWindow::on_kc85_busdrivers_changed(GtkSpinButton *spin_button, gpointer user_data) {
    OptionsWindow *self = (OptionsWindow *)user_data;
    int busdrivers = gtk_spin_button_get_value_as_int(spin_button);
    self->_current_profile->set_int_value("busdrivers", busdrivers);
}

void
OptionsWindow::on_roms_settings_check_button_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
    OptionsWindow *self = (OptionsWindow *)user_data;
    if (self->check_button_toggled(togglebutton))
        self->apply_system_type();
}

void
OptionsWindow::on_rom_changed(GtkComboBoxEntry *comboboxentry, gpointer user_data) {
    OptionsWindow *self = (OptionsWindow *)user_data;
    
    GtkTreeIter iter;
    if (!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(comboboxentry), &iter))
        return;

    gchar *rom;
    GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(comboboxentry));
    gtk_tree_model_get(model, &iter, 1, &rom, -1);
    
    const char *key = self->get_preferences_key(G_OBJECT(g_object_get_data(G_OBJECT(comboboxentry), DATA_KEY_CHECK_BUTTON)));
    self->_current_profile->set_string_value(key, rom);

    self->apply_system_type(); // let apply_roms_settings change the tooltip text for the combobox!
}

void
OptionsWindow::on_rom_open_clicked(GtkButton *button, gpointer user_data) {
  OptionsWindow *self = (OptionsWindow *)user_data;

  GtkWidget *filechooser = gtk_file_chooser_dialog_new(_("Open ROM Image..."),
                                                       GTK_WINDOW(self->_window),
                                                       GTK_FILE_CHOOSER_ACTION_OPEN,
                                                       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                       GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                                       NULL);

  if (self->_open_rom_last_path == NULL)
      self->_open_rom_last_path = sys_gethome();

  gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(filechooser), self->_open_rom_last_path);
  free(self->_open_rom_last_path);

  const char *key = self->get_preferences_key(G_OBJECT(g_object_get_data(G_OBJECT(button), DATA_KEY_CHECK_BUTTON)));
  if (gtk_dialog_run(GTK_DIALOG(filechooser)) == GTK_RESPONSE_ACCEPT)
    {
      char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(filechooser));
      self->_current_profile->set_string_value(key, filename);
      g_free(filename);
    }
    
  self->_open_rom_last_path = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(filechooser));

  gtk_widget_destroy(filechooser);

  self->apply_system_type(); // let apply_roms_settings add the filename from profile to combobox model!
}

void
OptionsWindow::apply_entry_value(GtkEntry *entry, gint signal_id)
{
  const char * key = get_preferences_key(G_OBJECT(entry));
  ProfileValue *value = get_current_profile_value(key);
  g_signal_handler_block(entry, signal_id);
  gtk_entry_set_text(entry, value == NULL ? "" : value->get_string_value());
  g_signal_handler_unblock(entry, signal_id);

  GtkToggleButton *toggle_button = GTK_TOGGLE_BUTTON(g_object_get_data(G_OBJECT(entry), DATA_KEY_CHECK_BUTTON));
  gtk_toggle_button_set_active(toggle_button, _current_profile->contains_key(key));
}

void
OptionsWindow::apply_network_settings(void)
{
  apply_entry_value(_w.entry_network_ip_address, _w.on_network_changed_id[0]);
  apply_entry_value(_w.entry_network_netmask, _w.on_network_changed_id[1]);
  apply_entry_value(_w.entry_network_gateway, _w.on_network_changed_id[2]);
  apply_entry_value(_w.entry_network_dns_server, _w.on_network_changed_id[3]);
}

void
OptionsWindow::on_network_settings_check_button_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
    OptionsWindow *self = (OptionsWindow *)user_data;
    if (self->check_button_toggled(togglebutton))
        self->apply_network_settings();
}

void
OptionsWindow::on_network_changed(GtkEntry *entry, gpointer user_data)
{
  OptionsWindow *self = (OptionsWindow *)user_data;
  const char * key = self->get_preferences_key(G_OBJECT(entry));
  gchar *text = pango_trim_string(gtk_entry_get_text(entry));
  gtk_entry_set_icon_from_stock(entry, GTK_ENTRY_ICON_PRIMARY, *text && !self->is_ip_address(text) ? GTK_STOCK_DIALOG_WARNING : NULL);
  gtk_entry_set_icon_activatable(entry, GTK_ENTRY_ICON_PRIMARY, FALSE);
  self->_current_profile->set_string_value(key, text);
  g_free(text);
}

GtkTreeModel *
OptionsWindow::get_selected_tree_iter(GtkTreeIter *iter) {
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(_w.treeview));
    
    GtkTreeModel *model;
    if (gtk_tree_selection_get_selected(selection, &model, iter))
        return model;
    
    return NULL;
}

/**
 * FreeBSD 8 still uses glib 2.20 as default version. That does
 * not include g_hostname_is_ip_address() which has the good point
 * to be available in MinGW environment too.
 * I'm not going to to create a wrapper just for the GUI check, so
 * there's no nice warning icon until glib is upgraded to at least 2.22.
 */
bool
OptionsWindow::is_ip_address(const char *addr)
{
#if GLIB_CHECK_VERSION(2, 22, 0)
  return g_hostname_is_ip_address(addr);
#else
  return true;
#endif
}

ProfileValue *
OptionsWindow::get_current_profile_value(const char *key) {
    if (_current_profile == NULL)
        return NULL;
    
    return _current_profile->get_value(key);
}

const char *
OptionsWindow::get_preferences_key(GObject *object) {
    const char * key = (const char *)g_object_get_data(object, PREFERENCES_KEY);
    g_assert(key != NULL);
    return key;
}

void
OptionsWindow::set_preferences_key(GObject *object, const char *key)
{
    g_assert(object != NULL);
    g_assert(key != NULL);
    g_object_set_data(object, PREFERENCES_KEY, strdup(key));
}

/**
 *  Apply the profile value to the given spinbutton. The key of the value to apply will
 *  be fetched from the data set on the checkbutton that enables/disables this spinbutton.
 *
 *  While setting the value the signal handler is blocked to prevent loops with the
 *  handler writing changes back to the profile.
 */
void
OptionsWindow::apply_spin_button_value(GtkCheckButton *check_button, GtkSpinButton *spin_button, gint signal_id, int default_value) {
    const char *key = get_preferences_key(G_OBJECT(check_button));
    ProfileValue *value = get_current_profile_value(key);
    int spin_value = value == NULL ? default_value : value->get_int_value();
    
    g_signal_handler_block(spin_button, signal_id);
    gtk_spin_button_set_value(spin_button, spin_value);
    g_signal_handler_unblock(spin_button, signal_id);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button), _current_profile->contains_key(key));
}

/**
 *  Apply the profile value to the given combobox. The key of the value to apply will
 *  be fetched from the data set on the checkbutton that enables/disables this combobox.
 *
 *  While setting the value the signal handler is blocked to prevent loops with the
 *  handler writing changes back to the profile.
 *
 *  The value for the combobox is currently limited to be 0 or 1.
 */
void
OptionsWindow::apply_combobox_value(GtkCheckButton *check_button, GtkComboBox *combobox, gint handler_id) {
    const char *key = get_preferences_key(G_OBJECT(check_button));
    ProfileValue *value = get_current_profile_value(key);
    int val = value == NULL ? 0 : value->get_int_value();
    
    g_signal_handler_block(combobox, handler_id);
    gtk_combo_box_set_active(combobox, val ? 1 : 0);
    g_signal_handler_unblock(combobox, handler_id);
    
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button), _current_profile->contains_key(key));
}

void
OptionsWindow::apply_profile(void) {
    g_signal_handler_block(_w.entry_profile_name, _w.on_profile_name_changed_id);
    gtk_entry_set_text(_w.entry_profile_name, _current_profile->get_name());
    g_signal_handler_unblock(_w.entry_profile_name, _w.on_profile_name_changed_id);
    gtk_widget_set_sensitive(GTK_WIDGET(_w.entry_profile_name), _current_profile->get_level() == PROFILE_LEVEL_USER);
    
    apply_comment();
    apply_system_type();
    apply_display_settings();
    apply_kc85_settings();
    apply_network_settings();
    
    apply_filechooserbutton(GTK_FILE_CHOOSER(get_widget("media_filechooserbutton_tape")));
    apply_filechooserbutton(GTK_FILE_CHOOSER(get_widget("media_filechooserbutton_audio")));
    apply_filechooserbutton(GTK_FILE_CHOOSER(get_widget("media_filechooserbutton_disk1")));
    apply_filechooserbutton(GTK_FILE_CHOOSER(get_widget("media_filechooserbutton_disk2")));
    apply_filechooserbutton(GTK_FILE_CHOOSER(get_widget("media_filechooserbutton_disk3")));
    apply_filechooserbutton(GTK_FILE_CHOOSER(get_widget("media_filechooserbutton_disk4")));
    apply_filechooserbutton(GTK_FILE_CHOOSER(get_widget("media_filechooserbutton_hd1")));
    apply_filechooserbutton(GTK_FILE_CHOOSER(get_widget("media_filechooserbutton_hd2")));
}

void
OptionsWindow::apply_comment(void) {
    g_signal_handler_block(_w.textbuffer_comment, _w.on_comment_changed_id);
    gtk_text_buffer_set_text(_w.textbuffer_comment, _current_profile->get_comment(), -1);
    g_signal_handler_unblock(_w.textbuffer_comment, _w.on_comment_changed_id);
    
    gtk_widget_set_sensitive(GTK_WIDGET(_w.textview_comment), _current_profile->get_level() == PROFILE_LEVEL_USER);
}

void
OptionsWindow::apply_system_type(void) {
    gtk_widget_set_sensitive(GTK_WIDGET(_w.combobox_system_type), _current_profile->get_level() == PROFILE_LEVEL_DEFAULT);
    
    ProfileValue *system = get_current_profile_value("system");
    ProfileValue *variant = get_current_profile_value("variant");
    
    g_signal_handler_block(_w.combobox_system_type, _w.on_system_type_changed_id);

    GtkTreeIter iter;
    bool valid = system != NULL;
    if (valid)
        valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(_w.liststore_system), &iter);
    while (valid) {
        int kc_type;
        gtk_tree_model_get(GTK_TREE_MODEL(_w.liststore_system), &iter, 1, &kc_type, -1);
        if (system->get_int_value() == kc_type) {
            gtk_combo_box_set_active_iter(_w.combobox_system_type, &iter);
            apply_system_variant((kc_type_t)kc_type, variant == NULL ? KC_VARIANT_NONE : (kc_variant_t)variant->get_int_value());
            apply_roms_settings((kc_type_t)kc_type, variant == NULL ? KC_VARIANT_NONE : (kc_variant_t)variant->get_int_value());
            apply_modules_settings((kc_type_t)kc_type);
            break;
        }
        valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(_w.liststore_system), &iter);
    }
    if (!valid) {
        gtk_combo_box_set_active(_w.combobox_system_type, -1);
        apply_system_variant(KC_TYPE_NONE, KC_VARIANT_NONE);
        apply_roms_settings(KC_TYPE_NONE, KC_VARIANT_NONE);
        apply_modules_settings(KC_TYPE_NONE);
    }

    g_signal_handler_unblock(_w.combobox_system_type, _w.on_system_type_changed_id);
}

void
OptionsWindow::set_roms_liststore(int idx, const char *rom_key, SystemROM *rom) {
    ProfileValue *profile_value = get_current_profile_value(rom_key);
    const char *profile_rom_filename = (profile_value == NULL) ? NULL : profile_value->get_string_value();

    GtkListStore *store = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(_w.roms_comboboxentry[idx])));
    gtk_list_store_clear(store);

    int active = -1;
    int list_entry = 0;
    ROMEntry *active_entry = NULL;
    const rom_entry_list_t roms = rom->get_roms();
    for (rom_entry_list_t::const_iterator it2 = roms.begin();it2 != roms.end();it2++) {
        ROMEntry *entry = (*it2);
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                           ROMS_DESCRIPTION_COLUMN, entry->get_description().c_str(),
                           ROMS_FILENAME_COLUMN, entry->get_filename().c_str(),
                           ROMS_TYPE_COLUMN, 1,
                           -1);
        
        if ((profile_rom_filename != NULL) && (entry->get_filename().compare(profile_rom_filename) == 0)) {
            active = list_entry;
            active_entry = entry;
        }
        list_entry++;
    }
    
    if ((active < 0) && (profile_rom_filename != NULL)) {
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                           ROMS_DESCRIPTION_COLUMN, sys_basename(profile_rom_filename),
                           ROMS_FILENAME_COLUMN, profile_rom_filename,
                           ROMS_TYPE_COLUMN, 2,
                           -1);
        active = list_entry;
    }
    
    if (active < 0) {
        active = 0;
        active_entry = *roms.begin();

        int idx = 0;
        for (rom_entry_list_t::const_iterator it = roms.begin();it != roms.end();it++) {
            ROMEntry *entry = (*it);
            if (entry->is_default()) {
                active = idx;
                active_entry = entry;
                break;
            }
            idx++;
        }
    }
    
    char tooltip[128];
    snprintf(tooltip, sizeof(tooltip), _("ROM image is '%s'. The ROM size needs to be %04xh (%d) bytes."),
             active_entry != NULL ? active_entry->get_filename().c_str() : profile_rom_filename,
             rom->get_size(),
             rom->get_size());
    gtk_widget_set_tooltip_text(GTK_WIDGET(_w.roms_comboboxentry[idx]), tooltip);

    g_signal_handler_block(_w.roms_comboboxentry[idx], _w.on_rom_changed_id[idx]);
    gtk_combo_box_set_model(GTK_COMBO_BOX(_w.roms_comboboxentry[idx]), GTK_TREE_MODEL(store));
    gtk_combo_box_set_active(GTK_COMBO_BOX(_w.roms_comboboxentry[idx]), active);
    g_signal_handler_unblock(_w.roms_comboboxentry[idx], _w.on_rom_changed_id[idx]);
}

void
OptionsWindow::apply_roms_settings(kc_type_t kc_type, kc_variant_t kc_variant) {
    const SystemType *type = SystemInformation::instance()->get_system_type(kc_type, kc_variant);
    if (type == NULL)
      return;

    system_rom_list_t rom_list = type->get_rom_list();
    int idx = 0;
    for (system_rom_list_t::const_iterator it = rom_list.begin();(it != rom_list.end()) && (idx < NR_OF_ROMS);it++) {
        SystemROM *rom = (*it);
        const char *rom_key = rom->get_name().c_str();
        
        set_roms_liststore(idx, rom_key, rom);
        
        string text = string(gettext(rom_key)) + ":";
        gtk_label_set_text(_w.roms_label[idx], text.c_str());

        gpointer val = g_object_get_data(G_OBJECT(_w.roms_check_button[idx]), PREFERENCES_KEY);
        g_object_set_data(G_OBJECT(_w.roms_check_button[idx]), PREFERENCES_KEY, strdup(rom_key));
        free(val);

        gtk_widget_show(GTK_WIDGET(_w.roms_label[idx]));
        gtk_widget_show(GTK_WIDGET(_w.roms_comboboxentry[idx]));
        gtk_widget_show(GTK_WIDGET(_w.roms_open_button[idx]));
        gtk_widget_show(GTK_WIDGET(_w.roms_check_button[idx]));

        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(_w.roms_check_button[idx]), _current_profile->contains_key(rom_key));
        gtk_widget_set_sensitive(GTK_WIDGET(_w.roms_check_button[idx]), _current_profile->get_level() != PROFILE_LEVEL_DEFAULT);

        idx++;
    }
    
    for (;idx < NR_OF_ROMS;idx++) {
        gtk_widget_hide(GTK_WIDGET(_w.roms_label[idx]));
        gtk_widget_hide(GTK_WIDGET(_w.roms_comboboxentry[idx]));
        gtk_widget_hide(GTK_WIDGET(_w.roms_open_button[idx]));
        gtk_widget_hide(GTK_WIDGET(_w.roms_check_button[idx]));
    }
}

void
OptionsWindow::apply_system_variant(kc_type_t kc_type, kc_variant_t kc_variant) {
    _current_kc_type = kc_type;

    system_type_list_t list = SystemInformation::instance()->get_system_types();

    GtkListStore *store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    for (system_type_list_t::iterator it = list.begin();it != list.end();it++) {
        SystemType *type = (*it);
        if (type->get_kc_type() != kc_type)
          continue;

        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, type->get_display_name(), 1, type->get_kc_variant(), -1);
    }
    
    g_signal_handler_block(_w.combobox_system_variant, _w.on_system_variant_changed_id);
    
    gtk_combo_box_set_model(_w.combobox_system_variant, GTK_TREE_MODEL(store));
    g_object_unref(store);

    if (kc_variant == KC_VARIANT_NONE) {
        gtk_combo_box_set_active(_w.combobox_system_variant, 0);
    } else {
        GtkTreeIter iter;
        bool valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter);
        while (valid) {
            int v;
            gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, 1, &v, -1);
            if (kc_variant == v) {
                gtk_combo_box_set_active_iter(_w.combobox_system_variant, &iter);
                break;
            }
            valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter);
        }
        if (!valid) {
            gtk_combo_box_set_active(_w.combobox_system_variant, 0);
        }
    }
    
    g_signal_handler_unblock(_w.combobox_system_variant, _w.on_system_variant_changed_id);
    
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(_w.check_button_system_variant), _current_profile->contains_key("variant"));
}

void
OptionsWindow::apply_display_settings(void) {
    apply_spin_button_value(_w.check_button_display_scale, _w.spin_button_display_scale, _w.on_display_scale_value_changed_id, 1);

    apply_combobox_value(_w.check_button_display_effects, _w.combobox_display_effects, _w.on_display_effects_changed_id);
    apply_combobox_value(_w.check_button_display_mem_access, _w.combobox_display_mem_access, _w.on_display_mem_access_changed_id);
    apply_combobox_value(_w.check_button_display_debug, _w.combobox_display_debug, _w.on_display_debug_changed_id);
}

void
OptionsWindow::apply_kc85_settings(void) {
    apply_spin_button_value(_w.check_button_busdrivers, _w.spin_button_busdrivers, _w.on_kc85_busdrivers_changed_id, 0);

    apply_combobox_value(_w.check_button_d004, _w.combobox_d004, _w.on_kc85_d004_changed_id);
    apply_combobox_value(_w.check_button_f8_rom, _w.combobox_f8_rom, _w.on_kc85_f8_rom_changed_id);
}

void
OptionsWindow::apply_modules_settings(kc_type_t kc_type) {
    const char *key = "modules";
    ProfileValue *modules = get_current_profile_value(key);
    string modules_list = modules == NULL ? "" : modules->get_string_value();
    
    StringList list(modules == NULL ? "" : modules->get_string_value());
    
    for (int a = 0;a < NR_OF_MODULES;a++) {
        g_signal_handler_block(_w.combobox_module[a], _w.on_module_changed_id[a]);
    }
    
    if (_w.liststore_modules != NULL) {
        gtk_list_store_clear(_w.liststore_modules);
        // FIXME: gtk_list_store_free() ???
    }
    _w.liststore_modules = get_modules_list_model(kc_type);
    for (int a = 0;a < NR_OF_MODULES;a++) {
        gtk_combo_box_set_model(_w.combobox_module[a], GTK_TREE_MODEL(_w.liststore_modules));
        gtk_combo_box_set_active(_w.combobox_module[a], 0);
    }
    
    int a = 0;
    for (StringList::iterator it = list.begin();(it != list.end()) && (a < NR_OF_MODULES);it++) {
        GtkTreeIter iter;
        GtkTreeModel *model = gtk_combo_box_get_model(_w.combobox_module[a]);
        bool valid = gtk_tree_model_get_iter_first(model, &iter);
        while (valid) {
            gchar *modif;
            gtk_tree_model_get(model, &iter, MODULES_MODIF_COLUMN, &modif, -1);
            if ((modif != NULL) && (strcmp((*it).c_str(), modif) == 0)) {
                gtk_combo_box_set_active_iter(_w.combobox_module[a], &iter);
                break;
            }
            g_free(modif);
            valid = gtk_tree_model_iter_next(model, &iter);
        }
        a++;
    }
    
    for (int a = 0;a < NR_OF_MODULES;a++) {
        g_signal_handler_unblock(_w.combobox_module[a], _w.on_module_changed_id[a]);
    }
    
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(_w.check_button_modules), _current_profile->contains_key(key));
}

void
OptionsWindow::apply_filechooserbutton(GtkFileChooser *filechooser) {
    const char * key = (const char *)g_object_get_data(G_OBJECT(filechooser), PREFERENCES_KEY);
    const char * dir = (const char *)g_object_get_data(G_OBJECT(filechooser), DEFAULT_DIR_KEY);
    
    ProfileValue *value = get_current_profile_value(key);
    
    const char *filename = value == NULL ? NULL : value->get_string_value();
    
    if (filename == NULL) {
        gtk_file_chooser_unselect_all(filechooser);
    } else {
        if (!sys_isabsolutepath(filename) && (dir != NULL)) {
            string path = string(kcemu_datadir) + dir + filename;
            gtk_file_chooser_select_filename(filechooser, path.c_str());
        } else {
            gtk_file_chooser_select_filename(filechooser, filename);
        }
    }
    
    GtkToggleButton *toggle_button = GTK_TOGGLE_BUTTON(g_object_get_data(G_OBJECT(filechooser), DATA_KEY_CHECK_BUTTON));
    gtk_toggle_button_set_active(toggle_button, _current_profile->contains_key(key));
}

void
OptionsWindow::move_row(GtkPathMoveFunc path_func, GtkIterMoveFunc move_func) {
    GtkTreeIter iter;
    GtkTreeModel *model;
    
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(_w.treeview));
    if (!gtk_tree_selection_get_selected(selection, &model, &iter))
        return;
    
    GtkTreePath *path = gtk_tree_model_get_path(model, &iter);
    path_func(path);
    
    GtkTreeIter position;
    if (gtk_tree_model_get_iter(model, &position, path)) {
        move_func(_w.treestore, &iter, &position);
    }
    
    gtk_tree_path_free(path);
}

void
OptionsWindow::add_system(GtkTreeStore *store, GtkTreeIter *iter, const char *name, const char *config_name, const char *icon_name) {
    GtkTreeIter iter_system;
    GdkPixbuf *pixbuf = get_icon(icon_name);
    gtk_tree_store_append(store, &iter_system, iter);
    gtk_tree_store_set(store, &iter_system,
            TREE_INT_COLUMN, 2,
            TREE_TEXT_COLUMN, name,
            TREE_CONFIG_NAME_COLUMN, config_name,
            TREE_ICON_COLUMN, pixbuf,
            -1);
    
    list<Profile *> children = Preferences::instance()->find_child_profiles(config_name);
    for (list<Profile *>::iterator it = children.begin();it != children.end();it++) {
        GtkTreeIter iter_child;
        gtk_tree_store_append(store, &iter_child, &iter_system);
        gtk_tree_store_set(store, &iter_child,
                TREE_INT_COLUMN, 2,
                TREE_TEXT_COLUMN, (*it)->get_name(),
                TREE_CONFIG_NAME_COLUMN, (*it)->get_config_name(),
                TREE_ICON_COLUMN, _w.pixbuf_default,
                -1);
    }
}

GtkTreeStore *
OptionsWindow::get_tree_model(void) {
    GtkTreeStore *store = gtk_tree_store_new(TREE_N_COLUMNS, G_TYPE_INT, G_TYPE_STRING, GDK_TYPE_PIXBUF, G_TYPE_STRING);
    
    GtkTreeIter iter_root;
    gtk_tree_store_append(store, &iter_root, NULL);
    gtk_tree_store_set(store, &iter_root, TREE_INT_COLUMN, 1, TREE_TEXT_COLUMN, "Default", TREE_ICON_COLUMN, _w.pixbuf_root, TREE_CONFIG_NAME_COLUMN, "default", -1);
    
    emulation_type_list_t list = EmulationType::get_emulation_types();
    for (emulation_type_list_t::iterator it = list.begin();it != list.end();it++) {
        add_system(store, &iter_root, (*it)->get_name(), (*it)->get_config_name(), (*it)->get_icon_name());
    }
    
    return store;
}

GtkListStore *
OptionsWindow::get_system_list_model(void) {
    GtkListStore *store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    emulation_type_list_t list = EmulationType::get_emulation_types();
    for (emulation_type_list_t::iterator it = list.begin();it != list.end();it++) {
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, (*it)->get_name(), 1, (*it)->get_kc_type(), -1);
    }
    
    return store;
}

GtkListStore *
OptionsWindow::get_variant_list_model(kc_variant_t kc_variant, const char *name, ...) {
    GtkListStore *store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    va_list ap;
    va_start(ap, name);
    
    while (242) {
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, name, 1, kc_variant, -1);
        
        kc_variant = (kc_variant_t)va_arg(ap, int);
        if (kc_variant < 0)
            break;
        
        name = va_arg(ap, const char *);
    }
    
    va_end(ap);
    
    return store;
}

GtkListStore *
OptionsWindow::get_modules_list_model(kc_type_t kc_type) {
    GtkListStore *store = gtk_list_store_new(MODULES_N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
    
    for (ModuleList::iterator it = module_list->begin();it != module_list->end();it++) {
        kc_type_t mod_kc_type = (*it)->get_kc_type();
        if ((mod_kc_type & kc_type) == 0)
            continue;
        
        const char *name = (*it)->get_name();
        const char *modif_name = (*it)->get_mod() == NULL ? NULL : (*it)->get_mod()->get_name();
        
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, MODULES_NAME_COLUMN, name, MODULES_MODIF_COLUMN, modif_name, MODULES_POINTER_COLUMN, (*it), -1);
    }
    
    return store;
}

void
OptionsWindow::wire_check_button(const char *preferences_key, const char *default_dir, const char *check_button_name, const char *filechooser_button_name, const char *button_clear_name) {
    GtkWidget *check_button = get_widget(check_button_name);
    GtkWidget *filechooser_button = get_widget(filechooser_button_name);
    GtkWidget *button_clear = get_widget(button_clear_name);
    g_object_set_data(G_OBJECT(check_button), FILE_CHOOSER_BUTTON_KEY, filechooser_button);
    g_object_set_data(G_OBJECT(check_button), FILE_CHOOSER_CLEAR_BUTTON_KEY, button_clear);
    g_object_set_data(G_OBJECT(button_clear), FILE_CHOOSER_BUTTON_KEY, filechooser_button);
    g_object_set_data(G_OBJECT(filechooser_button), DATA_KEY_CHECK_BUTTON, check_button);
    g_object_set_data(G_OBJECT(filechooser_button), PREFERENCES_KEY, strdup(preferences_key));
    if (default_dir != NULL)
        g_object_set_data(G_OBJECT(filechooser_button), DEFAULT_DIR_KEY, strdup(default_dir));
    g_signal_connect(check_button, "toggled", G_CALLBACK(on_media_check_button_toggled), this);
    g_signal_connect(button_clear, "clicked", G_CALLBACK(on_media_clear_button_clicked), this);
    g_signal_connect(filechooser_button, "file-set", G_CALLBACK(on_media_filechooser_file_set), this);
}

void
OptionsWindow::wire_check_button(const char *preferences_key, GtkCheckButton *check_button, GCallback callback, GtkWidget *target1, GtkWidget *target2, GtkWidget *target3) {
    g_object_set_data(G_OBJECT(check_button), PREFERENCES_KEY, strdup(preferences_key));
    g_object_set_data(G_OBJECT(check_button), TARGET_WIDGET1_KEY, target1);
    g_object_set_data(G_OBJECT(target1), DATA_KEY_CHECK_BUTTON, check_button);

    if (target2 != NULL) {
        g_object_set_data(G_OBJECT(check_button), TARGET_WIDGET2_KEY, target2);
        g_object_set_data(G_OBJECT(target2), DATA_KEY_CHECK_BUTTON, check_button);
    }
    if (target3 != NULL) {
        g_object_set_data(G_OBJECT(check_button), TARGET_WIDGET3_KEY, target3);
        g_object_set_data(G_OBJECT(target3), DATA_KEY_CHECK_BUTTON, check_button);
    }

    g_signal_connect(check_button, "toggled", callback, this);
}

void
OptionsWindow::init(void) {
    _window = get_widget("options_window");
    gtk_signal_connect(GTK_OBJECT(_window), "delete_event",
            GTK_SIGNAL_FUNC(cmd_exec_sft),
            (char *)"ui-options-window-toggle"); // FIXME:
    
    _w.pixbuf_root = get_icon("icon-root.png");
    _w.pixbuf_default = get_icon("icon-default.png");
    _w.pixbuf_deleted = get_icon("icon-deleted.png");

    _w.treestore = get_tree_model();
    _w.treeview = get_widget("tree_treeview");
    gtk_tree_view_set_model(GTK_TREE_VIEW(_w.treeview), GTK_TREE_MODEL(_w.treestore));
    gtk_tree_view_columns_autosize(GTK_TREE_VIEW(_w.treeview));

    GtkTreeViewColumn *column1 = gtk_tree_view_column_new();
    add_icon_renderer(GTK_TREE_VIEW(_w.treeview), column1, "", "pixbuf", TREE_ICON_COLUMN, NULL);
    add_text_renderer(GTK_TREE_VIEW(_w.treeview), column1, "Profile", "text", TREE_TEXT_COLUMN, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(_w.treeview), column1);
    //add_text_column(GTK_TREE_VIEW(_w.treeview), "Comment", "text", TOOLTIP_COLUMN, NULL);
    
    _w.notebook = GTK_NOTEBOOK(get_widget("notebook"));

    _w.b_new = get_widget("tree_button_new");
    g_signal_connect(_w.b_new, "clicked", G_CALLBACK(on_button_new_clicked), this);
    
    _w.b_copy = get_widget("tree_button_copy");
    g_signal_connect(_w.b_copy, "clicked", G_CALLBACK(on_button_copy_clicked), this);
    
    _w.b_delete = get_widget("tree_button_delete");
    g_signal_connect(_w.b_delete, "clicked", G_CALLBACK(on_button_delete_clicked), this);
    
    _w.b_up = get_widget("tree_button_up");
    g_signal_connect(_w.b_up, "clicked", G_CALLBACK(on_button_up_clicked), this);
    
    _w.b_down = get_widget("tree_button_down");
    g_signal_connect(_w.b_down, "clicked", G_CALLBACK(on_button_down_clicked), this);
    
    _w.b_expand = get_widget("tree_button_expand");
    g_signal_connect(_w.b_expand, "clicked", G_CALLBACK(on_button_expand_clicked), this);
    
    _w.b_collapse = get_widget("tree_button_collapse");
    g_signal_connect(_w.b_collapse, "clicked", G_CALLBACK(on_button_collapse_clicked), this);

    _w.b_ok = get_widget("dialog_button_ok");
    g_signal_connect(_w.b_ok, "clicked", G_CALLBACK(on_button_ok_clicked), this);
    
    _w.b_close = get_widget("dialog_button_close");
    g_signal_connect(_w.b_close, "clicked", G_CALLBACK(on_button_close_clicked), this);
    
    /* setup the selection handler */
    GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(_w.treeview));
    gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);
    g_signal_connect(G_OBJECT(select), "changed", G_CALLBACK(on_tree_selection_changed), this);
    
    _w.entry_profile_name = GTK_ENTRY(get_widget("system_entry_profile_name"));
    _w.on_profile_name_changed_id = g_signal_connect(_w.entry_profile_name, "changed", G_CALLBACK(on_profile_name_changed), this);
    
    _w.textview_comment = GTK_TEXT_VIEW(get_widget("system_textview_comment"));
    _w.textbuffer_comment = gtk_text_view_get_buffer(_w.textview_comment);
    _w.on_comment_changed_id = g_signal_connect(_w.textbuffer_comment, "changed", G_CALLBACK(on_comment_changed), this);
    
    wire_check_button("tape_file",   "/tapes/", "media_check_button_tape",  "media_filechooserbutton_tape",  "media_button_tape_clear");
    wire_check_button("audio_file",       NULL, "media_check_button_audio", "media_filechooserbutton_audio", "media_button_audio_clear");
    wire_check_button("disk_1",      "/disks/", "media_check_button_disk1", "media_filechooserbutton_disk1", "media_button_disk1_clear");
    wire_check_button("disk_2",      "/disks/", "media_check_button_disk2", "media_filechooserbutton_disk2", "media_button_disk2_clear");
    wire_check_button("disk_3",      "/disks/", "media_check_button_disk3", "media_filechooserbutton_disk3", "media_button_disk3_clear");
    wire_check_button("disk_4",      "/disks/", "media_check_button_disk4", "media_filechooserbutton_disk4", "media_button_disk4_clear");
    wire_check_button("hard_disk_1",      NULL, "media_check_button_hd1",   "media_filechooserbutton_hd1",   "media_button_hd1_clear");
    wire_check_button("hard_disk_2",      NULL, "media_check_button_hd2",   "media_filechooserbutton_hd2",   "media_button_hd2_clear");
    
    _w.check_button_display_scale = GTK_CHECK_BUTTON(get_widget("display_check_button_scale"));
    _w.check_button_display_effects = GTK_CHECK_BUTTON(get_widget("display_check_button_effects"));
    _w.check_button_display_mem_access = GTK_CHECK_BUTTON(get_widget("display_check_button_mem_access"));
    _w.check_button_display_debug = GTK_CHECK_BUTTON(get_widget("display_check_button_debug"));
    _w.spin_button_display_scale = GTK_SPIN_BUTTON(get_widget("display_spin_button_scale"));
    _w.combobox_display_effects = GTK_COMBO_BOX(get_widget("display_combobox_effects"));
    _w.combobox_display_mem_access = GTK_COMBO_BOX(get_widget("display_combobox_mem_access"));
    _w.combobox_display_debug = GTK_COMBO_BOX(get_widget("display_combobox_debug"));
    wire_check_button("display_scale",      _w.check_button_display_scale, G_CALLBACK(on_display_check_button_toggled),      GTK_WIDGET(_w.spin_button_display_scale));
    wire_check_button("display_effects",    _w.check_button_display_effects, G_CALLBACK(on_display_check_button_toggled),    GTK_WIDGET(_w.combobox_display_effects));
    wire_check_button("display_mem_access", _w.check_button_display_mem_access, G_CALLBACK(on_display_check_button_toggled), GTK_WIDGET(_w.combobox_display_mem_access));
    wire_check_button("display_debug",      _w.check_button_display_debug, G_CALLBACK(on_display_check_button_toggled),      GTK_WIDGET(_w.combobox_display_debug));
    _w.on_display_scale_value_changed_id = g_signal_connect(_w.spin_button_display_scale, "value-changed", G_CALLBACK(on_display_scale_value_changed), this);
    _w.on_display_effects_changed_id = g_signal_connect(_w.combobox_display_effects, "changed", G_CALLBACK(on_display_effects_changed), this);
    _w.on_display_mem_access_changed_id = g_signal_connect(_w.combobox_display_mem_access, "changed", G_CALLBACK(on_display_mem_access_changed), this);
    _w.on_display_debug_changed_id = g_signal_connect(_w.combobox_display_debug, "changed", G_CALLBACK(on_display_debug_changed), this);
    
    _w.check_button_d004 = GTK_CHECK_BUTTON(get_widget("kc85_checkbutton_d004"));
    _w.check_button_f8_rom = GTK_CHECK_BUTTON(get_widget("kc85_checkbutton_f8_rom"));
    _w.check_button_busdrivers = GTK_CHECK_BUTTON(get_widget("kc85_checkbutton_busdrivers"));
    _w.combobox_d004 = GTK_COMBO_BOX(get_widget("kc85_combobox_d004"));
    _w.combobox_f8_rom = GTK_COMBO_BOX(get_widget("kc85_combobox_f8_rom"));
    _w.spin_button_busdrivers = GTK_SPIN_BUTTON(get_widget("kc85_spinbutton_busdrivers"));
    wire_check_button("d004", _w.check_button_d004, G_CALLBACK(on_kc85_settings_check_button_toggled), GTK_WIDGET(_w.combobox_d004));
    wire_check_button("d004_f8", _w.check_button_f8_rom, G_CALLBACK(on_kc85_settings_check_button_toggled), GTK_WIDGET(_w.combobox_f8_rom));
    wire_check_button("busdrivers", _w.check_button_busdrivers, G_CALLBACK(on_kc85_settings_check_button_toggled), GTK_WIDGET(_w.spin_button_busdrivers));

    _w.on_kc85_d004_changed_id = g_signal_connect(_w.combobox_d004, "changed", G_CALLBACK(on_kc85_d004_changed), this);
    _w.on_kc85_f8_rom_changed_id = g_signal_connect(_w.combobox_f8_rom, "changed", G_CALLBACK(on_kc85_f8_rom_changed), this);
    _w.on_kc85_busdrivers_changed_id = g_signal_connect(_w.spin_button_busdrivers, "value-changed", G_CALLBACK(on_kc85_busdrivers_changed), this);
    
    for (int a = 0;a < NR_OF_MODULES;a++) {
        _w.combobox_module[a] = GTK_COMBO_BOX(get_widget("modules_combobox_module", a + 1));
        _w.on_module_changed_id[a] = g_signal_connect(_w.combobox_module[a], "changed", G_CALLBACK(on_module_changed), this);
        bind_list_model_column(_w.combobox_module[a], MODULES_NAME_COLUMN, NULL);
    }
    _w.check_button_modules = GTK_CHECK_BUTTON(get_widget("modules_check_button"));
    g_signal_connect(_w.check_button_modules, "toggled", G_CALLBACK(on_modules_check_button_toggled), this);
    
    _w.liststore_system = get_system_list_model();
    _w.combobox_system_type = GTK_COMBO_BOX(get_widget("system_combobox_system_type"));
    gtk_combo_box_set_model(_w.combobox_system_type, GTK_TREE_MODEL(_w.liststore_system));
    bind_list_model_column(_w.combobox_system_type, 0, NULL);
    _w.on_system_type_changed_id = g_signal_connect(_w.combobox_system_type, "changed", G_CALLBACK(on_system_type_changed), this);
    
    _w.combobox_system_variant = GTK_COMBO_BOX(get_widget("system_combobox_system_variant"));
    bind_list_model_column(_w.combobox_system_variant, 0, NULL);
    
    _w.on_system_variant_changed_id = g_signal_connect(_w.combobox_system_variant, "changed", G_CALLBACK(on_system_variant_changed), this);
    
    _w.check_button_system_variant = GTK_CHECK_BUTTON(get_widget("system_check_button_system_variant"));
    g_signal_connect(_w.check_button_system_variant, "toggled", G_CALLBACK(on_system_variant_check_button_toggled), this);

    for (int a = 0;a < NR_OF_ROMS;a++) {
        _w.roms_label[a] = GTK_LABEL(get_widget("roms_label_rom", a + 1));
        _w.roms_comboboxentry[a] = GTK_COMBO_BOX_ENTRY(get_widget("roms_comboboxentry_rom", a + 1));
        _w.roms_open_button[a] = GTK_BUTTON(get_widget("roms_open_button_rom", a + 1));
        _w.roms_check_button[a] = GTK_CHECK_BUTTON(get_widget("roms_check_button_rom", a + 1));
        wire_check_button("rom_dummy",
                      _w.roms_check_button[a],
                      G_CALLBACK(on_roms_settings_check_button_toggled),
                      GTK_WIDGET(_w.roms_comboboxentry[a]),
                      GTK_WIDGET(_w.roms_open_button[a]));

        g_signal_connect(_w.roms_open_button[a], "clicked", G_CALLBACK(on_rom_open_clicked), this);
        _w.on_rom_changed_id[a] = g_signal_connect(_w.roms_comboboxentry[a], "changed", G_CALLBACK(on_rom_changed), this);

        GtkListStore *store = gtk_list_store_new(ROMS_N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT);
        gtk_combo_box_set_model(GTK_COMBO_BOX(_w.roms_comboboxentry[a]), GTK_TREE_MODEL(store));
        gtk_combo_box_entry_set_text_column(_w.roms_comboboxentry[a], 0);
        g_object_unref(store);
    }

    _w.entry_network_ip_address = GTK_ENTRY(get_widget("ip_address_entry"));
    set_preferences_key(G_OBJECT(_w.entry_network_ip_address), "network_ip_address");
    _w.on_network_changed_id[0] = g_signal_connect(_w.entry_network_ip_address, "changed", G_CALLBACK(on_network_changed), this);
    wire_check_button("network_ip_address", GTK_CHECK_BUTTON(get_widget("ip_address_checkbutton")), G_CALLBACK(on_network_settings_check_button_toggled), GTK_WIDGET(_w.entry_network_ip_address));

    _w.entry_network_netmask = GTK_ENTRY(get_widget("netmask_entry"));
    set_preferences_key(G_OBJECT(_w.entry_network_netmask), "network_netmask");
    _w.on_network_changed_id[1] = g_signal_connect(_w.entry_network_netmask, "changed", G_CALLBACK(on_network_changed), this);
    wire_check_button("network_netmask", GTK_CHECK_BUTTON(get_widget("netmask_checkbutton")), G_CALLBACK(on_network_settings_check_button_toggled), GTK_WIDGET(_w.entry_network_netmask));

    _w.entry_network_gateway = GTK_ENTRY(get_widget("gateway_entry"));
    set_preferences_key(G_OBJECT(_w.entry_network_gateway), "network_gateway");
    _w.on_network_changed_id[2] = g_signal_connect(_w.entry_network_gateway, "changed", G_CALLBACK(on_network_changed), this);
    wire_check_button("network_gateway", GTK_CHECK_BUTTON(get_widget("gateway_checkbutton")), G_CALLBACK(on_network_settings_check_button_toggled), GTK_WIDGET(_w.entry_network_gateway));

    _w.entry_network_dns_server = GTK_ENTRY(get_widget("dns_server_entry"));
    set_preferences_key(G_OBJECT(_w.entry_network_dns_server), "network_dns_server");
    _w.on_network_changed_id[3] = g_signal_connect(_w.entry_network_dns_server, "changed", G_CALLBACK(on_network_changed), this);
    wire_check_button("network_dns_server", GTK_CHECK_BUTTON(get_widget("dns_server_checkbutton")), G_CALLBACK(on_network_settings_check_button_toggled), GTK_WIDGET(_w.entry_network_dns_server));

    collapse_tree();
    
    init_dialog(NULL, NULL);
}

void
OptionsWindow::save(void) {
    string config_name = _current_profile->get_config_name();
    
    Preferences::instance()->save();
    
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(_w.treeview));
    gtk_tree_model_foreach(model, tree_model_foreach_func_delete, this);

    /* select parent if we have deleted the currently selected profile */
    if (Preferences::instance()->find_profile(config_name.c_str()) == NULL) {
        GtkTreeIter iter;
        GtkTreeModel *model = get_selected_tree_iter(&iter);
        if (model != NULL) {
            GtkTreeIter parent;
            if (gtk_tree_model_iter_parent(model, &parent, &iter)) {
                GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(_w.treeview));
                gtk_tree_selection_select_iter(selection, &parent);
            }
        }
    }

    // delete in reverse order. otherwise we invalidate the collected path strings
    for (list<string>::reverse_iterator it = _delete_path.rbegin();it != _delete_path.rend();it++) {
        GtkTreeIter iter;
        if (gtk_tree_model_get_iter_from_string(model, &iter, (*it).c_str())) {
            gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);
        }
    }
    _delete_path.clear();
    
    hide();
}

void
OptionsWindow::reject(void) {
    Preferences::instance()->reject();
    apply_profile();
    hide();
}
